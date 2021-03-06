#include "ConsolaKernel.h"

void recibir_comandos()
{
	while(morir == 0)
	{
		//0-> new, 1->ready, 2->execute, 3-> blocked, 4-> suspended, 9-> killed,
		char* entrada = leerCaracteresEntrantes();
		if (*entrada!=NULL){
			char** comando = string_split(entrada, " ");
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
					printf("[KERNEL]: PROCESO PID: %i LLEVA %i RAFAGAS EJECUTADAS\n", pc->pid, pc->rafagasEj);
				}
			}
			else if(!strcmp(comando[0], "tablaArchivos"))
			{
				if(comando[1]!=NULL){
					if(!strcmp(comando[1], "global"))
					{
						puts("-------TABLA DE ARCHIVOS GLOBAL-------");
						puts("|\tRUTA\t|\tINSTANCIAS\t|");
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
							puts("|\tFD\t|\tCURSOR\t|\tFLAGS\t|APUNTA A (RUTA)\t|");
							mostrarTablaDeArchivosProceso(pid);
							printf("-------TABLA DE ARCHIVOS PID: %i-------\n", pid);
						}
						pthread_mutex_unlock(&mMaxPID);
					}
				}
				else{
					puts("[KERNEL]: Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "multiprog"))
			{
				if(comando[1]!=NULL)
				{
					int multiprog = atoi(comando[1]);
					if(multiprog <= 0)
						puts("[CONFIG]: NO ES NU NUMERO VALIDO");
					else
					{
						pthread_mutex_lock(&mMultiprog);
						config->GRADO_MULTIPROG = multiprog;
						pthread_mutex_unlock(&mMultiprog);
						printf("[CONFIG]: NUEVO GRADO DE MULTIPROGRAMACION: %i\n", config->GRADO_MULTIPROG);
						log_info(logFile, "[CONFIG]: NUEVO GRADO DE MULTIPROGRAMACION: %i\n", config->GRADO_MULTIPROG);
						enviarTodosLosNewAReady();
					}
				}
				else{
					puts("[KERNEL]: Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "syscalls"))
			{
				if(comando[1]!=NULL){
					int pid = atoi(comando[1]);
					ProcessControl* pc = PIDFind(pid);
					printf("[KERNEL]: SYSCALLS PID: %i: %i\n", pc->pid, pc->syscalls);
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
						if(pcb == NULL)
							puts("NO ESTA EN FINISHED");
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
					puts("[KERNEL]: Argumento no puede ser null\n");
				}
			}
			else if(!strcmp(comando[0], "togglePlanif"))
			{

				pthread_mutex_lock(&mTogglePlanif);
				if(togglePlanif == 0)
				{
					log_error(logFile, "[PLANIFICACION]: DETIENDO PLANIFICACION");
					togglePlanif = 1;
					pthread_mutex_unlock(&mTogglePlanif);
				}
				else
				{
					log_error(logFile, "[PLANIFICACION]: REANUDANDO PLANIFICACION");
					togglePlanif = 0;
					pthread_mutex_unlock(&mTogglePlanif);
					matarTodosLosQueCorresponda();
					enviarTodosLosNewAReady();
					enviarTodosLosReadyAExecute();

				}

			}
			else if(!strcmp(comando[0], "exit"))
			{
				morir = 1;
				getConnectedSocket(config->IP_PROPIA, config->PUERTO_PROG, CONSOLA_ID);
			}
			else if(!strcmp(comando[0], "help"))
			{

				puts("---------------COMANDOS---------------");
				puts("cola [exit] [ready] [blocked] [new] [execute] [vacio]");
				puts("rafagasEj [PID] | tablaArchivos [global] [PID]");
				puts("multiprog [numero] | syscalls [PID] | heapInfo [PID]");
				puts("mostrarPCB [PIDEnExit] | kill [PID] | togglePlanif");
				puts("exit");
				puts("---------------COMANDOS---------------");
			}
			else
				puts("[KERNEL]: COMANDO INVALIDO");
			free(comando[0]);
			free(comando[1]);
			free(comando);
			free(entrada);
		}
	}

}

void matarTodosLosQueCorresponda()
{
	bool paraMatar(ProcessControl* pc)
	{
		return pc->toBeKilled != 0;
	}
	t_list* ajusticiados = list_filter(process, paraMatar);
	void matar(ProcessControl* pc)
	{
		matarSiCorresponde(pc->pid);
	}
	list_iterate(ajusticiados, matar);
	list_destroy(ajusticiados);
}

void mostrarPCB(PCB* pcb)
{
	puts("----------------PCB----------------");
	mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
	printf("PID: %i | EXIT CODE: %i\n", pcb->pid, pcb->exitCode);
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


char* leerCaracteresEntrantes() {
	int i, caracterLeido;
	char* cadena = malloc(100);
	for(i = 0; (caracterLeido= getchar()) != '\n'; i++)
		cadena[i] = caracterLeido;
	cadena[i] = '\0';
	return cadena;
}
//a
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
	printf("|\t%i\t|\t %i\t|\t%s\t|\t%s\t|\n", entrada->fd, entrada->cursor, entrada->flags, entrada->entradaGlobal->ruta);
}

void mostrarTablaDeArchivosGlobal()
{
	//MUTEX
	list_iterate(tablaGlobalFS,  mostrarEntradaTablaGlobalFS);
	//MUTEX
}

void mostrarEntradaTablaGlobalFS(entradaTablaGlobalFS* entrada)
{
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
