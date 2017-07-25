#include <stdio.h>
#include <stdlib.h>

#include "Listen.h"
#include "globales.h"
#include "KernelConfiguration.h"
#include "ConsolaKernel.h"
#include "KernelTest.h"
#include <signal.h>
const char* keys[17] = {"PUERTO_PROG", "PUERTO_CPU", "IP_MEMORIA", "PUERTO_MEMORIA", "IP_FS", "PUERTO_FS", "QUANTUM", "QUANTUM_SLEEP", "ALGORITMO", "GRADO_MULTIPROG", "SEM_IDS", "SEM_INIT", "SHARED_VARS", "STACK_SIZE","IP_PROPIA", "LOG","NULL"};


void levantarLog();
void morirDecentemente();
void destruirVariableGlobal(GlobalVariable* gb);
void destruirVariablesGlobales();
void destruirCapaMemoria();
void destruirColasPlanificacion();
void destruirTablasFS();
void destruirListaProcesos();
void destruirSemaforo(Semaforo* sem);
void destruirListaDeColasSemaforos();
void initializeGlobalVariables();
