#include "ConsolaKernel.h"

void recibir_comandos()
{
	while(morir == 0)
	{
		//0-> new, 1->ready, 2->execute, 3-> blocked, 4-> suspended, 9-> killed,
		char* entrada = leerCaracteresEntrantes();
		if (*entrada!=NULL){
			char** comando = string_split(entrada, " ");
			printf("COMANDO: %s ARG: %s\n", comando[0], comando[1]);
			if(!strcmp(comando[0], "cola"))
			{
				if(comando[1] == NULL)
				{
					puts("-------TODOS LOS PROCESOS (INCLUYE EXIT)------");
					pthread_mutex_lock(&mProcess);
					list_iterate(process, mostrarPorConsola);
					pthread_mutex_unlock(&mProcess);
					//MUTEX
					puts("-------TODOS LOS PROCESOS (INCLUYE EXIT)------");
				}
				else if(!strcmp(comando[1], "new"))
				{
					puts("-------COLA NEW------");
					mostrarProcesosEnEstado(0);
					puts("-------COLA NEW------");
				}
				else if(!strcmp(comando[1], "ready"))
				{
					puts("-------COLA READY------");
					pthread_mutex_lock(&mColaReady);
					list_iterate(colaReady->elements, mostrarPID);
					pthread_mutex_unlock(&mColaReady);
					puts("-------COLA READY------");
				}
				else if(!strcmp(comando[1], "execute"))
				{
					puts("-------COLA EXECUTE------");
					pthread_mutex_lock(&mListaExec);
					list_iterate(executeList, mostrarPID);
					pthread_mutex_unlock(&mListaExec);

					puts("-------COLA EXECUTE------");
				}
				else if(!strcmp(comando[1], "blocked"))
				{
					puts("-------COLA BLOCKED------");
					pthread_mutex_lock(&mListaBlocked);
					list_iterate(blockedList, mostrarPID);
					pthread_mutex_unlock(&mListaBlocked);

					puts("-------COLA BLOCKED------");
				}
				else if(!strcmp(comando[1], "exit"))
				{
					puts("-------COLA EXIT------");
					pthread_mutex_lock(&mColaFinished);
					list_iterate(colaFinished->elements, mostrarPID);
					pthread_mutex_unlock(&mColaFinished);
					puts("-------COLA EXIT------");
				}
			}
			else if(!strcmp(comando[0], "rafagasEj"))
			{
				if(comando[1] != NULL)
				{
					int pid = atoi(comando[1]);
					ProcessControl* pc = PIDFind(pid); // ADENTRO TIENE MUTEX
					printf("PROCESO PID: %i LLEVA %i RAFAGAS EJECUTADAS\n", pc->pid, pc->rafagasEj);
				}
			}
			// FALTAN VARIOS CMDS QUE TODAVIA NO ESTA IMPLEMENTADO DONDE SE GUARDA ESA INFO
			else if(!strcmp(comando[0], "tablaArchivos"))
			{
				if(comando[1]!=NULL){
					if(!strcmp(comando[1], "global"))
					{
						puts("-------TABLA DE ARCHIVOS GLOBAL-------");
						mostrarTablaDeArchivosGlobal();
						puts("-------TABLA DE ARCHIVOS GLOBAL-------");
					}
					else
					{
						int pid = atoi(comando[1]);
						pthread_mutex_lock(&mMaxPID);
						if(pid > maxPID | pid < 0)
							puts("PID INVALIDO");
						else
						{
							printf("-------TABLA DE ARCHIVOS PID: %i-------\n", pid);
							mostrarTablaDeArchivosProceso(pid);
							printf("-------TABLA DE ARCHIVOS PID: %i-------\n", pid);
						}
						pthread_mutex_unlock(&mMaxPID);
					}
				}
				else{
					puts("Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "multiprog"))
			{
				if(comando[1]!=NULL)
				{
					int multiprog = atoi(comando[1]);
					if(multiprog <= 0)
						puts("NO ES NU NUMERO VALIDO");
					else
					{
						pthread_mutex_lock(&mMultiprog);
						config->GRADO_MULTIPROG = multiprog;
						pthread_mutex_unlock(&mMultiprog);
						printf("NUEVO GRADO DE MULTIPROGRAMACION: %i\n", config->GRADO_MULTIPROG);
						enviarTodosLosNewAReady();
					}
				}
				else{
					puts("Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "syscalls"))
			{
				if(comando[1]!=NULL){
					int pid = atoi(comando[1]);
					ProcessControl* pc = PIDFind(pid);
					printf("SYSCALLS PID: %i: %i\n", pc->pid, pc->syscalls);
				}
			}
			else if(!strcmp(comando[0], "heapInfo"))
			{
				if(comando[1]!=NULL){
					int pid = atoi(comando[1]);
					ProcessControl* pc = PIDFind(pid);
					puts("---------HEAP INFO----------");
					printf("CANTIDAD ALLOCS: %i | BYTES ALOCADOS: %i | PAGINAS ALOCADAS (CONTANDO LIBERADAS): %i\n", pc->cantAlocar, pc->heapBytes, pc->heapPages);
					printf("CANTIDAD FREES: %i | BYTES LIBERADOS: %i\n", pc->cantFree, pc->freedBytes);
					puts("---------HEAP INFO----------");

				}
			}
			else if(!strcmp(comando[0], "mostrarPCB"))
			{
				if(comando[1]!=NULL)
				{
					int pid = atoi(comando[1]);
					//0-> new, 1->ready, 2->execute, 3-> blocked, 4-> suspended, 9-> killed,
						bool mismoPID(PCB* pcb)
						{
							return pcb->pid == pid;
						}
						pthread_mutex_lock(&mColaFinished);
						PCB* pcb = list_find(colaFinished->elements, mismoPID);
						pthread_mutex_unlock(&mColaFinished);
						// SOLO LOS EXIT PARA SALIR DEL PASO
						if(pcb == NULL)
							puts("NO ESTA EN FINISHED);
						else
							mostrarPCB(pcb);

				}
			}
			else if(!strcmp(comando[0], "kill"))
			{
				if(comando[1]!=NULL){
					int pid = atoi(comando[1]);
					matarCuandoCorresponda(pid,-7);
					printf("SE ENVIO A AJUSTICIAR EL PROCESO PID: %i. SE AJUSTICIARA CUANDO CORRESPONDA.\n", pid);
				}
				else{
					puts("Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "togglePlanif"))
			{
				pthread_mutex_lock(&mTogglePlanif);
				togglePlanif=1;
				pthread_mutex_unlock(&mTogglePlanif);
			}
			else if(!strcmp(comando[0], "untogglePlanif"))
			{
				pthread_mutex_lock(&mTogglePlanif);
				togglePlanif=0;
				pthread_mutex_unlock(&mTogglePlanif);
				enviarTodosLosNewAReady();
				enviarTodosLosReadyAExecute();
			}
			else if(!strcmp(comando[0], "exit"))
			{
				morir = 1;
			}
			else
				puts("COMANDO INVALIDO");
			free(comando[0]);
			free(comando[1]);
			free(comando);
			free(entrada);
		}
	}

}

void mostrarPCB(PCB* pcb)
{
	puts("----------------PCB----------------");
	printf("PID: %i | EXIT CODE: %i\n", pcb->pid, pcb->exitCode);
	mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
	puts("----------------PCB----------------");
}

void enviarTodosLosNewAReady()
{
	pthread_mutex_lock(&mColaNew);
	int new = queue_size(colaNew);
	pthread_mutex_unlock(&mColaNew);
	int i;
	for(i = 0;i<new;i++)
		readyProcess();
}

void enviarTodosLosReadyAExecute()
{
	pthread_mutex_lock(&mColaReady);
	int ready = queue_size(colaReady);
	pthread_mutex_unlock(&mColaReady);
	int i;
	for(i = 0;i<ready;i++)
		executeProcess();
}

void mostrarPID(PCB* pcb)
{
	printf("PID: %i\n", pcb->pid);
}

char* leerCaracteresEntrantes() {
	int i, caracterLeido;
	char* cadena = malloc(30);
	for(i = 0; (caracterLeido= getchar()) != '\n'; i++)
		cadena[i] = caracterLeido;
	cadena[i] = '\0';
	return cadena;
}

void mostrarTablaDeArchivosProceso(int pid)
{
	tablaDeProceso* tabla = encontrarTablaDelProceso(pid);
	if(tabla == NULL)
		puts("NO TIENE ARCHIVOS");
	else
		list_iterate(tabla->entradasTablaProceo, &mostrarEntradaTablaArchivoProceso);
}

void mostrarEntradaTablaArchivoProceso(entradaTablaFSProceso* entrada)
{
	puts("|\tCURSOR\t|\tFLAGS\t|APUNTA A (RUTA)\t|");
	printf("|\t %i\t|\t%s\t|\t%s\t|\n", entrada->cursor, entrada->flags, entrada->entradaGlobal->ruta);
}

void mostrarTablaDeArchivosGlobal()
{
	//MUTEX
	list_iterate(tablaGlobalFS,  mostrarEntradaTablaGlobalFS);
	//MUTEX
}

void mostrarEntradaTablaGlobalFS(entradaTablaGlobalFS* entrada)
{
	puts("|\tRUTA\t|\tINSTANCIAS\t|");
	printf("|\t%s\t|\t%i\t|\n",entrada->ruta, entrada->instancias);
}

void mostrarProcesosEnEstado(int estado)
{
	bool mismoEstado(ProcessControl* pc)
	{
		return estado == pc->state;
	}
	pthread_mutex_lock(&mProcess);
	t_list* filtrados = list_filter(process, &mismoEstado);
	pthread_mutex_unlock(&mProcess);
	list_iterate(filtrados, &mostrarPorConsola);

}

void mostrarPorConsola(ProcessControl* pc)
{
	printf("PID %i | ESTADO: %i\n", pc->pid, pc->state);
}
