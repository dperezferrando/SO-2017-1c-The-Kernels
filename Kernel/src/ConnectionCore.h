#ifndef __CONN_CORE__
#define __CONN_CORE__

#include <commons/collections/list.h>
#include "Configuration.h"
#include <commons/collections/queue.h>
#include "KernelConfiguration.h"
#include "globales.h"
#include "Process.h"
#include "SocketLibrary.h"
#include <stdlib.h>

typedef struct connectionHandler{
	socketHandler cpu;
	socketHandler consola;
	int listenCPU;
	int listenConsola;
}connHandle;


PCB* recibirPCB(Mensaje*);

int PIDFindPO(int);
int readyProcess();
int executeProcess();
int checkMultiprog();
int offset(t_list*, int);
int enviarScriptAMemoria(PCB*,char*, int);
int grabarPedido(PageOwnership*, MemoryRequest, HeapMetadata*,int*);
int sendMemoryRequest(MemoryRequest, int, void*, PageOwnership*, int);

bool viableRequest(int);
bool fsSock(int, connHandle*);
bool memSock(int, connHandle*);
bool cpuSock(int, connHandle*);
bool consSock(int, connHandle*);
bool isListener(int, connHandle);


void newProcess(PCB*, int);
void enviarInformacion(int);
void aceptarNuevoCPU(int);
void closeHandle(int, connHandle*);
void recibirDeCPU(int, connHandle*);
void recibirDeConsola(int, connHandle*);
void initializePageOwnership(PageOwnership*);
void storeVariable(PageOwnership*, char*, int);
void handleSockets(connHandle*, socketHandler);
void matarCuandoCorresponda(ProcessControl* pc);

void* serializarScript(int, int, int, int*, void*);


t_list* findProcessPages(int);


MemoryRequest deserializeMemReq(void*);
HeapMetadata* initializeHeapMetadata(int);

//Para semaforos
int sonIguales(char* s1, char* s2);
int obtenerPosicionSemaforo(char* c);
int obtenerValorSemaforo(char* c);
void enviarAColaDelSemaforo(char* c, int* pid);
int* quitarDeColaDelSemaforo(char* c);
void operarSemaforo(char* c, int num);
void waitSemaforo(char* c);
void signalSemaforo(char* c);
void crearListaDeColasSemaforos();
int laColaDelSemaforoEstaVacia(int posicionSemaforo);
void quitarDeColaDelSemaforoPorKill(int);


#endif
