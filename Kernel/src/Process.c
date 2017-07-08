#include "Process.h"




//--------------------------------------------Creacion y Destruccion--------------------------------------------------------//




PCB* createProcess(char* script, int tamanioScript){
	PCB* pcb = malloc(sizeof(PCB));
	pcb->pid = (maxPID++);
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
	crearEstructurasFSProceso(pcb->pid);
	metadata_destruir(metadata);
	return pcb;
}


void _modifyExitCode(int PID,int exitCode){
	bool _PIDFind(PCB* pcb){
		return pcb->pid== PID;
	}

	PCB* pcb= list_find(colaFinished->elements,&(_PIDFind));
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
	lSend(conexionMemoria, &PID, 9, sizeof(int));
	lSend(pc->consola, &PID, 9, sizeof(int));
	//Verifica si esta en alguna de cola de algun semaoforo
	quitarDeColaDelSemaforoPorKill(PID);
	eliminarEntradasTabla(PID);
	_modifyExitCode(PID,exitCode);
	if(checkMultiprog() && queue_size(colaNew) >0)
		fromNewToReady();

}






//-----------------------------------------------------Manejo de Planificacion-----------------------------------------------//




void newProcess(PCB* pcb, int consola)
{
	queue_push(colaNew, pcb);
	ProcessControl* pc= malloc(sizeof(ProcessControl));
	pc->pid= pcb->pid;
	pc->state= 0;
	pc->consola = consola;
	pc->toBeKilled = 0;
	list_add(process,pc);
}


int readyProcess(){//-1 ==> no se pudo poner en ready
	if(checkMultiprog()){
		fromNewToReady();
	} else {
		return -1;
		}

	executeProcess();
	return 1;
}


int executeProcess(){
	if(queue_size(colaCPUS)<=0 || queue_size(colaReady) <=0){
		return -1;
	} else{
		PCB* pcb= fromReadyToExecute();
		serializado pcbSerializado;
		int CPU = (int)queue_pop(colaCPUS);
		if(!test){
			pcbSerializado = serializarPCB(pcb);
			lSend(CPU, pcbSerializado.data, 1, pcbSerializado.size);
			puts("PCB ENVIADO");
			free(pcbSerializado.data);
		}
		return 1;
		}
}


void cpuReturnsProcessTo(PCB* newPCB, int state){
	switch(state){
		case 1:
			fromExecuteToReady(newPCB->pid);
			replacePCBinQueue(newPCB,colaReady);
			break;
		case 3:
			fromExecuteToBlocked(newPCB->pid);
			replacePCBInList(newPCB,blockedList);
			break;
	}

}


//----------------------------------------------------Manejo de Colas-------------------------------------------------------//




PCB* fromNewToReady(){
	return _fromQueueToQueue(colaNew,colaReady,1);
}


PCB* fromBlockedToReady(int pid){
	return _fromListToQueue(blockedList,colaReady,pid,1);
}


PCB* fromExecuteToReady(int pid){
	return _fromListToQueue(executeList,colaReady,pid,1);
}

PCB* fromExecuteToBlocked(int pid){
	return _fromListToList(executeList,blockedList,pid,3);
}


PCB* fromReadyToExecute(){
	return _fromQueueToList(colaReady,executeList,2);
}


PCB* fromExecuteToFinished(int pid){
	return _fromListToQueue(executeList,colaFinished,pid,9);
}


PCB* fromReadyToFinished(int pid){
	return _fromListToQueue(colaReady->elements,colaFinished,pid,9);
}


PCB* fromBlockedToFinished(int pid){
	return _fromListToQueue(blockedList,colaFinished,pid,9);
}


PCB* _fromQueueToQueue(t_queue* fromQueue, t_queue* toQueue, int newState){
	PCB* pcb = queue_pop(fromQueue);
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
	_processChangeStateToQueue(toQueue,pcb,newState);
	return pcb;
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
		return 1;
	}
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
	return list_remove_by_condition(process,&(_PIDFind));
}

ProcessControl* PIDFind(int PID){
	bool _PIDFind(ProcessControl* pc){
		return pc->pid== PID;
	}
	return list_find(process,&(_PIDFind));
}


void modifyProcessState(int PID, int newState){
	ProcessControl* pc = PIDFindAndRemove(PID);
	pc->state= newState;
	list_add(process, pc);
}


void _processChangeStateToQueue(t_queue* toQueue, PCB* pcb, int newState){
	_processChangeStateToList(toQueue->elements,pcb,newState);
}


void _processChangeStateToList(t_list* toList, PCB* pcb, int newState){
	list_add(toList, pcb);
	modifyProcessState(pcb->pid,newState);
}

