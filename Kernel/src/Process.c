#include "Configuration.h"
#include <commons/collections/list.h>

PCB* createProcess(t_list* procesos){//devuelve el index en la lista, que coincide con el PID
	PCB* pcb = malloc(sizeof(PCB));
	pcb->pid= list_size(procesos);
	return pcb;
}

void killProcess(t_list* procesos,int* PID){
	bool encontrarPorPID(PCB* PCB){
		return PCB->pid==PID;
	}
	list_remove_by_condition(procesos,&encontrarPorPID);
}
