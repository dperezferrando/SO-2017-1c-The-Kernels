#include "Process.h"

PCB* createProcess(char* script, int tamanioScript){//devuelve el index en la lista, que coincide con el PID
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

int PIDFind(int PID){
	bool _PIDFind(ProcessControl* pc){
		return pc->pid== PID;
	}
	return *((int*)list_find(process,&(_PIDFind)));
}

void modifyProcessState(int PID, int newState){
	ProcessControl* pc = list_get(process,PIDFind(PID));
	pc->state= newState;
	list_replace(process, PIDFind(PID), pc);
}

PCB* _fromTo(t_queue* fromQueue, t_queue* toQueue, int newState){
	PCB* pcb = queue_pop(fromQueue);
	queue_push(toQueue, pcb);
	modifyProcessState(pcb->pid,newState);
	return pcb;
}

PCB* fromNewToReady(){
	return _fromTo(colaNew,colaReady,1);
}

PCB* fromReadyToExecute(){
	return _fromTo(colaReady,colaExecute,2);
}
