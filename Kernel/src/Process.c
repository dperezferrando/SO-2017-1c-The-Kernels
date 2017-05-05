#include "Configuration.h"
#include <commons/collections/list.h>

int createProcess(t_list* procesos){//devuelve el index en la lista, que coincide con el PID
	PCB pcb;
	pcb.identificadorProceso= list_size(procesos);
	list_add(procesos,&pcb);
	return pcb.identificadorProceso;
}
