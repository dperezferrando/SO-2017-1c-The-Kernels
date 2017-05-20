#include "Configuration.h"
#include "globales.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>

PCB* createProcess(char* script, int tamanioScript){//devuelve el index en la lista, que coincide con el PID
	PCB* pcb = malloc(sizeof(PCB));
	pcb->pid= queue_size(colaReady);
	t_metadata_program* metadata = metadata_desde_literal(script);
	pcb->cantPaginasCodigo = ceil((double)tamanioScript/(double)config->PAG_SIZE);
	pcb->sizeIndiceCodigo = metadata->instrucciones_size*sizeof(indCod);
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	pcb->exitCode = 0;
	memcpy(pcb->indiceCodigo, metadata->instrucciones_serializado, metadata->instrucciones_size*sizeof(indCod));
	pcb->programCounter = 0;
	return pcb;
}

void killProcess(t_list* procesos,int* PID){
	bool encontrarPorPID(PCB* PCB){
		return PCB->pid==PID;
	}
	list_remove_by_condition(procesos,&encontrarPorPID);
}
