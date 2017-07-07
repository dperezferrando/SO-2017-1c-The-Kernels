#ifndef __GLOBAL__
#define __GLOBAL__

#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include "KernelConfiguration.h"
#include "SocketLibrary.h"


struct ProcessControl{
	int pid;
	int state; //0-> new, 1->ready, 2->execute, 3-> blocked, 4-> suspended, 9-> killed, //NULL-> no fue aceptado todavia//
	int consola;
	bool toBeKilled;
} typedef ProcessControl;

struct MemoryRequest{
	int pid;
	int size;
} typedef MemoryRequest;

struct HeapMetadata{
	uint32_t size;
	bool isFree;
} typedef HeapMetadata;

struct HeapControl{
	char* name;
	int listPosition;
} typedef HeapControl;

struct PageOwnership{
	int pid;
	int idpage;
	t_list* occSpaces;
	t_list* control;
} typedef PageOwnership;

struct entradaTablaGlobalFS
{
	char* ruta;
	int instancias;
} typedef entradaTablaGlobalFS;

struct entradaTablaFSProceso
{
	char* flags;
	entradaTablaGlobalFS* entradaGlobal;
	int cursor; // offset

} typedef entradaTablaFSProceso;

struct tablaDeProceso // Lista de tablas de proceso (que a su vez son una lista)
{
	int pid;
	t_list* entradasTablaProceo;

} typedef tablaDeProceso;

bool test;
bool morir;

int maxPID;
int conexionFS;
int conexionMemoria;

Mensaje* res;

t_queue* colaNew;
t_queue* colaCPUS;
t_queue* colaReady;
t_queue* colaFinished;
//t_queue* colaSuspended;

t_list* process;
t_list* ownedPages;
t_list* executeList;
t_list* blockedList;
t_list* listaDeColasSemaforos;
t_list* tablaGlobalFS;
t_list* tablasDeProcesosFS;

configFile* config;

pthread_t consolaKernel;

/*
 * en caso de ver que sea necesario *
	pthread_mutex_t mx_colaReady;
	pthread_mutex_t mx_colaBlocked;
	pthread_mutex_t mx_colaSuspended;
	pthread_mutex_t mx_colaFinished;
	pthread_mutex_t mx_colaCPUS;
*/


#endif
