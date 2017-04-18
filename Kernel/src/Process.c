#include "Process.h"

int createProcess(t_list* procesos){//devuelve el index en la lista, que coincide con el PID
	PCB pcb;
	pcb.PID= list_size(procesos);
	list_add(procesos,pcb);
	return pcb.PID;
}
