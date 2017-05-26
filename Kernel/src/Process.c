#include "Process.h"




//--------------------------------------------Creacion y Destruccion--------------------------------------------------------//




PCB* createProcess(char* script, int tamanioScript){
	PCB* pcb = malloc(sizeof(PCB));
	pcb->pid= (maxPID++);
	t_metadata_program* metadata = metadata_desde_literal(script);
	pcb->cantPaginasCodigo = ceil((double)tamanioScript/(double)config->PAG_SIZE);
	pcb->sizeIndiceCodigo = metadata->instrucciones_size*sizeof(indCod);
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	pcb->exitCode = 0;
	memcpy(pcb->indiceCodigo, metadata->instrucciones_serializado, metadata->instrucciones_size*sizeof(indCod));
	pcb->programCounter = 0;
	return pcb;
}


void killProcess(int* PID){
	bool encontrarPorPID(PCB* PCB){
		return (PCB->pid)==*PID;
	}
	//list_remove_by_condition(colaNew,&encontrarPorPID);
}




//-----------------------------------------------------Manejo de Planificacion-----------------------------------------------//




void newProcess(PCB* pcb)
{
	puts("6");
	queue_push(colaNew, pcb);
	puts("7");
	ProcessControl* pc= malloc(sizeof(ProcessControl));
	pc->pid= pcb->pid;
	pc->state= 0;
	puts("8");
	list_add(process,pc);
	puts("9");
}


int readyProcess(){//-1 ==> no se pudo poner en ready
	if(checkMultiprog()){
		puts("no viole segmento");
		fromNewToReady();
	} else {
		return -1;
		}

	executeProcess();
	return 1;
}


int executeProcess(){
	if(queue_size(colaCPUS)<=0){
		return -1;
	} else{
		PCB* pcb= fromReadyToExecute();
		PCBSerializado pcbSerializado = serializarPCB(pcb);
		int CPU = (int)queue_pop(colaCPUS);
		lSend(CPU, pcbSerializado.data, 1, pcbSerializado.size);
		puts("PCB ENVIADO");
		free(pcbSerializado.data);
		return 1;
		}
}




//----------------------------------------------------Manejo de Colas-------------------------------------------------------//




PCB* fromNewToReady(){
	puts("from new to ready");
	return _fromQueueToQueue(colaNew,colaReady,1);
	puts("paso a ready");
}


PCB* fromBlockedToReady(int pid){
	return _fromListToQueue(blockedList,colaReady,pid,1);
}


PCB* fromExecuteToReady(int pid){
	return _fromListToQueue(executeList,colaReady,pid,2);
}


PCB* fromReadyToExecute(){
	return _fromQueueToList(colaReady,executeList,2);
}


PCB* fromExecuteToFinished(int pid){
	return _fromListToQueue(executeList,colaFinished,pid,9);
}


PCB* fromReadyToFinished(){
	return _fromQueueToQueue(colaReady,colaFinished,9);
}


PCB* fromBlockedToFinished(int pid){
	return _fromListToQueue(blockedList,colaFinished,pid,9);
}


PCB* _fromQueueToQueue(t_queue* fromQueue, t_queue* toQueue, int newState){
	PCB* pcb = queue_pop(fromQueue);
	puts("pcb");
	_processChangeStateToQueue(toQueue,pcb,newState);
	puts("state");
	return pcb;
}

PCB* _fromQueueToList(t_queue* fromQueue, t_list* toList, int newState){
	PCB* pcb = queue_pop(fromQueue);
	_processChangeStateToList(toList,pcb, newState);
	return pcb;
}


PCB* _fromListToQueue(t_list* fromList, t_queue* toQueue, int PID, int newState){
	bool encontrarPorPID(PCB* PCB){
		return (PCB->pid)==PID;
	}
	PCB* pcb= list_remove_by_condition(fromList,&encontrarPorPID);
	_processChangeStateToQueue(toQueue,pcb,newState);
	return pcb;
}




//---------------------------------------------------------Auxiliares------------------------------------------------------//




ProcessControl* PIDFind(int PID){
	bool _PIDFind(ProcessControl* pc){
		return pc->pid== PID;
	}
	return list_remove_by_condition(process,&(_PIDFind));
}


void modifyProcessState(int PID, int newState){
	puts("modify process state");
	ProcessControl* pc = PIDFind(PID);
	puts("lo saco de la lista");
	printf("process control is null: %d",pc==NULL);
	pc->state= newState;
	list_add(process, pc);
}


void _processChangeStateToQueue(t_queue* toQueue, PCB* pcb, int newState){
	queue_push(toQueue, pcb);
	puts("push ook");
	puts("pcb se rompe?");
	printf("%d",pcb->pid);
	modifyProcessState(pcb->pid,newState);
}


void _processChangeStateToList(t_list* toList, PCB* pcb, int newState){
	list_add(toList, pcb);
	modifyProcessState(pcb->pid,newState);
}
