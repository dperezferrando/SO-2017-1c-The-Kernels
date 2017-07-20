#ifndef __CONSOLA__
#define __CONSOLA__
#include <commons/collections/list.h>
#include "globales.h"
#include "Process.h"


void recibir_comandos();
void mostrarTablaDeArchivosProceso(int pid);
void mostrarEntradaTablaArchivoProceso(entradaTablaFSProceso* entrada);
void mostrarTablaDeArchivosGlobal();
void mostrarEntradaTablaGlobalFS(entradaTablaGlobalFS* entrada);
void mostrarProcesosEnEstado(int estado);
void mostrarPorConsola(ProcessControl* pc);
char* leerCaracteresEntrantes();
void mostrarPID(PCB* pcb);
#endif
