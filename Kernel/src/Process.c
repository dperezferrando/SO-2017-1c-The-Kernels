#include "Process.h"




//--------------------------------------------Creacion y Destruccion--------------------------------------------------------//




PCB* createProcess(char* script, int tamanioScript){
	PCB* pcb = malloc(sizeof(PCB));
	pthread_mutex_lock(&mMaxPID);
	pcb->pid = (maxPID++);
	pthread_mutex_unlock(&mMaxPID);
	t_metadata_program* metadata = metadata_desde_literal(script);
	pcb->cantPaginasCodigo = ceil((double)tamanioScript/(double)config->PAG_SIZE);
	pcb->sizeIndiceCodigo = metadata->instrucciones_size*sizeof(indCod);
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	pcb->sizeIndiceEtiquetas = metadata->etiquetas_size;
	pcb->indiceEtiqueta = malloc(pcb->sizeIndiceEtiquetas);
	memcpy(pcb->indiceEtiqueta, metadata->etiquetas, pcb->sizeIndiceEtiquetas);
	memcpy(pcb->indiceCodigo, metadata->instrucciones_serializado, metadata->instrucciones_size*sizeof(indCod));
	pcb->nivelDelStack = 0;
	pcb->indiceStack = crearIndiceDeStack();
	pcb->exitCode = 0;
	pcb->programCounter = 0;
	pcb->rafagasTotales = 0;
	crearEstructurasFSProceso(pcb->pid);
	metadata_destruir(metadata);
	return pcb;
}


void _modifyExitCode(int PID,int exitCode){
	bool _PIDFind(PCB* pcb){
		return pcb->pid== PID;
	}
	pthread_mutex_lock(&mColaFinished);
	PCB* pcb= list_find(colaFinished->elements,&(_PIDFind));
	pthread_mutex_unlock(&mColaFinished);
	pcb->exitCode= exitCode;
}


void killProcess(int PID,int exitCode){
	bool encontrarPorPID(PCB* PCB){
		return (PCB->pid)==PID;
	}
	ProcessControl* pc= PIDFind(PID);
	t_queue* queue= NULL;
	t_list* list= NULL;
	switch (pc->state){
	case 0:
		queue= colaNew;
		fromNewToFinished(PID);
		break;
	case 1:
		fromReadyToFinished(PID);
		break;
	case 2:
		fromExecuteToFinished(PID);
		break;
	case 3:
		fromBlockedToFinished(PID);
		break;
	}
	if(exitCode == -1) // Si no hay espacio le digo a la consola que murio por no haber espacio
		lSend(pc->consola, &PID, -2, sizeof(int));
	else // Si murio por otra razon le aviso tambien a memoria
	{
		if(exitCode != -6 && pc->state != 0)
			lSend(pc->consola, &PID, 9, sizeof(int));
		if(pc->state != 0)
			lSend(conexionMemoria, &PID, 9, sizeof(int));
	}
	//Verifica si esta en alguna de cola de algun semaoforo
	quitarDeSemaforosPorKill(PID);
	eliminarEntradasDelProceso(PID);
	_modifyExitCode(PID,exitCode);
	informarMemoryLeaks(PID);
	freeProcessPages(PID);
	if(checkMultiprog() && queue_size(colaNew) >0){
		readyProcess();
		//executeProcess(); no corresponde
	}
	log_warning(logFile, "[KILL]: EL PROCESO PID %i FUE AJUSTICIADO", PID);

}

void informarMemoryLeaks(int pid)
{
	bool PIDFind(PageOwnership* po){
		return po->pid== pid;
	}

	t_list* paginas= list_filter(ownedPages, &PIDFind);
	int size = list_size(paginas);
	log_info(logFile,"[HEAP]: PAGINAS HEAP DEL PROCESO PID %i ANTES DE MORIR: %i\n", pid, size);
	if(size == 0)
		log_info(logFile,"EL PROCESO PID: %i LIBERO TODO SU HEAP O NUNCA TUVO\n", pid);
	else
	{
		log_warning(logFile,"---------------------------------------------------------");
		log_warning(logFile,"EL PROCESO PID: %i NO LIBERO LA SIGUIENTE MEMORIA:\n", pid);
		list_iterate(paginas, &mostrarPaginaHeap);
		log_warning(logFile,"LIBERANDO MEMORIA...");
		log_warning(logFile,"---------------------------------------------------------");

	}
	list_destroy(paginas);

}


void mostrarPaginaHeap(PageOwnership* po)
{
	log_warning(logFile,"PAGINA: %i\n", po->idpage);
	list_iterate(po->occSpaces, &mostrarMetadata);
}


//-----------------------------------------------------Manejo de Planificacion-----------------------------------------------//




void newProcess(PCB* pcb, int consola, char* script, int tamanioScript)
{
	pthread_mutex_lock(&mColaNew);
	queue_push(colaNew, pcb);
	pthread_mutex_unlock(&mColaNew);
	ProcessControl* pc= malloc(sizeof(ProcessControl));
	pc->pid= pcb->pid;
	pc->state= 0;
	pc->consola = consola;
	pc->CPU= -1;
	pc->toBeKilled = 0;
	pc->script = malloc(tamanioScript);
	pc->rafagasEj = 0;
	pc->heapBytes = 0;
	pc->syscalls = 0;
	pc->heapBytes = 0;
	pc->cantAlocar = 0;
	pc->cantFree = 0;
	pc->freedBytes = 0;
	pc->heapPages = 0;
	memcpy(pc->script, script, tamanioScript);
	pc->tamanioScript = tamanioScript;
	pc->toBeSignaled = 0;
	list_add(process,pc);
}

void matarSiCorresponde(int pid)
{
	ProcessControl* pc = PIDFind(pid);
	if(pc->toBeKilled != 0)
		killProcess(pid,pc->toBeKilled);
}

int readyProcess(){//-1 ==> no se pudo poner en ready
	pthread_mutex_lock(&mTogglePlanif);//si justo lo cambian, lo lamento, los semaforos no van a prevenir que en la funcion no entre cambiado
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);
	if(toggle)return -2;
	if(checkMultiprog()){
		fromNewToReady();
	} else {
		return -1;
		}

	executeProcess();
	return 1;
}


int executeProcess(){
	pthread_mutex_lock(&mTogglePlanif);//si justo lo cambian, lo lamento, los semaforos no van a prevenir que en la funcion no entre cambiado
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);
	if(toggle)return -2;
	pthread_mutex_lock(&mColaCPUS);
	int cpus = queue_size(colaCPUS);
	pthread_mutex_unlock(&mColaCPUS);
	pthread_mutex_lock(&mColaReady);
	int ready = queue_size(colaReady);
	pthread_mutex_unlock(&mColaReady);
	if(cpus <=0 ||  ready <=0){
		return -1;
	} else{
		PCB* pcb= fromReadyToExecute();
		serializado pcbSerializado;
		pthread_mutex_lock(&mColaCPUS);
		int CPU = (int)queue_pop(colaCPUS);
		pthread_mutex_unlock(&mColaCPUS);
		if(!test){
			pcbSerializado = serializarPCB(pcb);
			int quantumSleep = config->QUANTUM_SLEEP;
			char* buff = malloc(sizeof(int)+pcbSerializado.size);
			memcpy(buff, &quantumSleep, sizeof(int));
			memcpy(buff+sizeof(int), pcbSerializado.data, pcbSerializado.size);
			lSend(CPU, buff, 1, pcbSerializado.size + sizeof(int));
			ProcessControl* pc = PIDFind(pcb->pid);
			pc->CPU= CPU;
			free(pcbSerializado.data);
			free(buff);
		}
		return 1;
		}
}


void cpuReturnsProcessTo(PCB* newPCB, int state){
	ProcessControl* pc = PIDFind(newPCB->pid);
	pthread_mutex_lock(&mProcess);
	pc->CPU=-1;
	pthread_mutex_unlock(&mProcess);
	switch(state){
		case 1:
			fromExecuteToReady(newPCB->pid);
			replacePCBinQueue(newPCB,colaReady);
			break;
		case 3:
			fromExecuteToBlocked(newPCB->pid);
			replacePCBInList(newPCB,blockedList);
			break;
		case 9:
			fromExecuteToFinished(newPCB->pid);
			replacePCBinQueue(newPCB, colaFinished);
			break;
	}

}


//----------------------------------------------------Manejo de Colas-------------------------------------------------------//



PCB* fromNewToReady(){

	pthread_mutex_lock(&mTogglePlanif);
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);

	if(toggle)return NULL;

	pthread_mutex_lock(&mColaNew);
	pthread_mutex_lock(&mColaReady);
	PCB* pcb = _fromQueueToQueue(colaNew,colaReady,1);
	pthread_mutex_unlock(&mColaReady);
	pthread_mutex_unlock(&mColaNew);

	if(pcb == NULL)
		return NULL;
	ProcessControl* pc = PIDFind(pcb->pid);
	log_warning(logFile, "[PLANIFICACION]: PID %i DE NEW A READY", pcb->pid);
	if(!enviarScriptAMemoria(pcb, pc->script, pc->tamanioScript))
	{
		log_error(logFile,"[PLANIFICACION]: NO HAY ESPACIO PARA EL PROCESO PID %i", pcb->pid);
		if(!test)lSend(pc->consola, &pcb->pid, -2, sizeof(int));
		killProcess(pcb->pid, -1);
	}
	else
		if(!test)lSend(pc->consola, &pcb->pid, 2, sizeof(int));

	return pcb;
}


PCB* fromBlockedToReady(int pid){
//	pthread_mutex_lock(&mListaBlocked);
//	pthread_mutex_lock(&mColaReady);
	pthread_mutex_lock(&mTogglePlanif);
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);
	if(toggle)return NULL;
	log_warning(logFile, "[PLANIFICACION]: PID %i DE BLOCKED A READY",pid);
	return _fromListToQueue(blockedList,colaReady,pid,1);
//	pthread_mutex_unlock(&mColaReady);
//	pthread_mutex_unlock(&mListaBlocked);
}


PCB* fromExecuteToReady(int pid){
//	pthread_mutex_lock(&mListaExec);
//	pthread_mutex_lock(&mColaReady);
	log_warning(logFile, "[PLANIFICACION]: PID %i DE EJECUCION A READY", pid);
	return _fromListToQueue(executeList,colaReady,pid,1);
//	pthread_mutex_unlock(&mColaReady);
//	pthread_mutex_unlock(&mListaExec);
}

PCB* fromExecuteToBlocked(int pid){
//	pthread_mutex_lock(&mListaExec);
//	pthread_mutex_lock(&mListaBlocked);
	log_warning(logFile, "[PLANIFICACION]: PID %i DE EJECUCION A BLOCKED", pid);
	return _fromListToList(executeList,blockedList,pid,3);
//	pthread_mutex_unlock(&mListaBlocked);
//	pthread_mutex_unlock(&mListaExec);
}


PCB* fromReadyToExecute(){

	pthread_mutex_lock(&mTogglePlanif);
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);

	if(toggle)return NULL;
	log_warning(logFile, "[PLANIFICACION]: SE ENVIA UN PROCESO NEW A EJECUTAR");
//	pthread_mutex_lock(&mColaReady);
//	pthread_mutex_lock(&mListaExec);
	return _fromQueueToList(colaReady,executeList,2);
//	pthread_mutex_unlock(&mListaExec);
//	pthread_mutex_unlock(&mColaReady);
}


PCB* fromExecuteToFinished(int pid){
//	pthread_mutex_lock(&mColaFinished);
//	pthread_mutex_lock(&mListaExec);
	log_warning(logFile, "[PLANIFICACION]: PID %i DE EJECUCION A FINISHED", pid);
	return _fromListToQueue(executeList,colaFinished,pid,9);
//	pthread_mutex_unlock(&mListaExec);
//	pthread_mutex_unlock(&mColaFinished);
}


PCB* fromReadyToFinished(int pid){

	pthread_mutex_lock(&mTogglePlanif);
	bool toggle= togglePlanif;
	pthread_mutex_unlock(&mTogglePlanif);

	if(toggle)return NULL;
	log_warning(logFile, "[PLANIFICACION]: PID %i DE READY A FINISHED", pid);
//	pthread_mutex_lock(&mColaFinished);
//	pthread_mutex_lock(&mColaReady);
	return _fromListToQueue(colaReady->elements,colaFinished,pid,9);
//	pthread_mutex_unlock(&mColaReady);
//	pthread_mutex_unlock(&mColaFinished);
}


PCB* fromBlockedToFinished(int pid){
//	pthread_mutex_lock(&mColaFinished);
//	pthread_mutex_lock(&mListaBlocked);
	log_warning(logFile, "[PLANIFICACION]: PID %i DE BLOCKED A FINISHED", pid);
	return _fromListToQueue(blockedList,colaFinished,pid,9);
//	pthread_mutex_unlock(&mListaBlocked);
//	pthread_mutex_unlock(&mColaFinished);
}


PCB* fromNewToFinished(int pid){
	log_warning(logFile, "[PLANIFICACION]: PID %i DE NEW A FINISHED", pid);
	return _fromListToQueue(colaNew->elements, colaFinished, pid, 9);
}


PCB* _fromQueueToQueue(t_queue* fromQueue, t_queue* toQueue, int newState){
	PCB* pcb = queue_pop(fromQueue);
	if(pcb == NULL)
		return NULL;
	_processChangeStateToQueue(toQueue,pcb,newState);
	return pcb;
}

PCB* _fromQueueToList(t_queue* fromQueue, t_list* toList, int newState){
	PCB* pcb = queue_pop(fromQueue);
	_processChangeStateToList(toList,pcb, newState);
	return pcb;
}


PCB* _fromListToQueue(t_list* fromList, t_queue* toQueue, int PID, int newState){
	PCB* pcb= removePcbFromList(PID,fromList);
	if(pcb == NULL)
		return NULL;
	else
	{
		_processChangeStateToQueue(toQueue,pcb,newState);
		return pcb;
	}
}

PCB* _fromListToList(t_list* fromList, t_list* toList, int PID, int newState){
	PCB* pcb= removePcbFromList(PID,fromList);
	_processChangeStateToList(toList,pcb,newState);
	return pcb;
}

PCB* removePcbFromList(int PID,t_list* fromList){
	bool encontrarPorPID(PCB* PCB){
		return (PCB->pid)==PID;
	}
	return list_remove_by_condition(fromList,&encontrarPorPID);
}

int replacePCBInList(PCB* pcb,t_list* list){
	PCB* replacedPCB= removePcbFromList(pcb->pid,list);
	if(pcb->pid == replacedPCB->pid){
		list_add(list,pcb);
		destruirPCB(replacedPCB);
		return 1;
	}
	destruirPCB(replacedPCB);
	return -1;
}

int replacePCBinQueue(PCB* pcb,t_queue* queue){
	return replacePCBInList(pcb,queue->elements);
}


//---------------------------------------------------------Auxiliares------------------------------------------------------//




ProcessControl* PIDFindAndRemove(int PID){
	bool _PIDFind(ProcessControl* pc){
		return pc->pid== PID;
	}
	pthread_mutex_lock(&mProcess);
	ProcessControl* pc = list_remove_by_condition(process,&(_PIDFind));
	pthread_mutex_unlock(&mProcess);
	return pc;
}

ProcessControl* PIDFind(int PID){
	bool _PIDFind(ProcessControl* pc){
		return pc->pid== PID;
	}
	pthread_mutex_lock(&mProcess);
	ProcessControl* pc =  list_find(process,&(_PIDFind));
	pthread_mutex_unlock(&mProcess);
	return pc;
}


void modifyProcessState(int PID, int newState){
	ProcessControl* pc = PIDFindAndRemove(PID);
	pthread_mutex_lock(&mProcess);
	pc->state= newState;
	list_add(process, pc);
	pthread_mutex_unlock(&mProcess);
}


void _processChangeStateToQueue(t_queue* toQueue, PCB* pcb, int newState){
	_processChangeStateToList(toQueue->elements,pcb,newState);
}


void _processChangeStateToList(t_list* toList, PCB* pcb, int newState){
	list_add(toList, pcb);
	modifyProcessState(pcb->pid,newState);
}

void destruirProcessControl(ProcessControl* pc)
{
	free(pc->script);
	free(pc);
}


//---------------------------------------------------------Destruccion de paginas de heap del proceso que muere----------------------------------------------------//



void freeProcessPages(int pid){
	int i;
	bool PIDFind(PageOwnership* po){
		return po->pid== pid;
	}
	t_list* paginas= list_filter(ownedPages, &PIDFind);
	int size= list_size(paginas);
	for(i=0;i<size;i++){
		PageOwnership* po= list_get(paginas,0);
		list_remove(paginas,0);
    	int index= findIndex(po->pid,po->idpage);
		freePage(po,index, 0);
	}
	list_destroy(paginas);
}

void freePage(PageOwnership* po, int index, int avisoAMemoria){
	void* msg= malloc(sizeof(int)*2);
	memcpy(msg,&po->pid,sizeof(int));
	memcpy(msg+sizeof(int),&po->idpage,sizeof(int));
	if(!test && avisoAMemoria)lSend(conexionMemoria,msg,4,sizeof(int)*2);
	log_info(logFile,"[HEAP]: SE LIBERO PID %i, PAGINA %i\n",po->pid,po->idpage);
	list_remove_and_destroy_element(ownedPages,index,&destroyPageOwnership);
	free(msg);
}


void destroyPageOwnership(PageOwnership* po){
	int i;
	list_destroy(po->control);
	int size= list_size(po->occSpaces);
	for (i=0;i<size;i++){
		list_remove_and_destroy_element(po->occSpaces,0,&free);
	}
	list_destroy(po->occSpaces);
	free(po);
}
