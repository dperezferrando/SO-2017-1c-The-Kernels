/*
 ============================================================================
 Name        : Kernel.c
 Author      : Santiago M. Lorenzo
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Kernel.h"

int main(int argc, char** argsv) {

	puts("!!!Hello Kernel!!!\n"); /* prints !!!Hello World!!! */
	config = configurate(RUTA, readConfigFile, keys);
	levantarLog();
	initializeGlobalVariables();
	if(argc > 1 && strcmp(argsv[1],"-test")==0){
		test=1;
		kernelTest(1);
		return EXIT_SUCCESS;
	}
	test=0;
	morir =0;

	process= list_create();
	colaCPUS = queue_create();

	colaNew = queue_create();
	colaReady = queue_create();
	blockedList = list_create();
	executeList = list_create();
	colaFinished=queue_create();
	tablaGlobalFS = list_create();
	tablasDeProcesosFS = list_create();
	ownedPages = list_create();
	crearListaDeColasSemaforos();

	pthread_create(&consolaKernel, NULL, (void*) recibir_comandos, NULL);
	handler();
	pthread_join(consolaKernel, NULL);
	morirDecentemente();
	return 0;
}


void levantarLog()
{
	if(fopen(config->log, "r") != NULL)
		remove(config->log);
	logFile = log_create(config->log, "KERNEL", 1, 1);
}

void morirDecentemente()
{
	destruirConfig(config);
	destruirColasPlanificacion();
	destruirTablasFS();
	destruirListaProcesos();
	destruirListaDeColasSemaforos();
	destruirCapaMemoria();
	destruirVariablesGlobales();
	log_destroy(logFile);
}

void destruirVariableGlobal(GlobalVariable* gb)
{
	free(gb->name);
	free(gb);
}
void destruirVariablesGlobales()
{
	list_destroy_and_destroy_elements(globalVariables,&destruirVariableGlobal);
}

void destruirCapaMemoria()
{
	list_destroy_and_destroy_elements(ownedPages, &destroyPageOwnership);
}

void destruirColasPlanificacion()
{
	queue_destroy_and_destroy_elements(colaFinished, destruirPCB);
	queue_destroy_and_destroy_elements(colaNew, destruirPCB);
	queue_destroy_and_destroy_elements(colaReady, destruirPCB);
	queue_destroy(colaCPUS);
	list_destroy_and_destroy_elements(blockedList, destruirPCB);
	list_destroy_and_destroy_elements(executeList, destruirPCB);
}

void destruirTablasFS()
{
	list_destroy_and_destroy_elements(tablasDeProcesosFS, destruirTablaProceso);
	list_destroy_and_destroy_elements(tablaGlobalFS, destruirEntradaGlobal);

}

void destruirListaProcesos()
{
	list_destroy_and_destroy_elements(process, destruirProcessControl);
}

void destruirSemaforo(Semaforo* sem)
{
	free(sem->nombre);
	queue_destroy_and_destroy_elements(sem->cola, free);
	free(sem);
}

void destruirListaDeColasSemaforos()
{
	list_destroy_and_destroy_elements(listaDeColasSemaforos, destruirSemaforo);
}

void initializeGlobalVariables(){
	int i;
	globalVariables= list_create();
	GlobalVariable* gb;
	for(i=0;config->SHARED_VARS[i]!= NULL;i++){
		gb = malloc(sizeof(GlobalVariable));
		gb->name = malloc(strlen(config->SHARED_VARS[i]));
		memcpy(gb->name,config->SHARED_VARS[i]+1,strlen(config->SHARED_VARS[i]));
		gb->value=0;
		list_add(globalVariables,gb);
	}
}
