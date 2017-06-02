#ifndef __CONN_CORE__
#define __CONN_CORE__

#include <commons/collections/list.h>
#include "Configuration.h"
#include <commons/collections/queue.h>
#include "KernelConfiguration.h"
#include "globales.h"
#include "Process.h"
#include "SocketLibrary.h"

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
int enviarScriptAMemoria(PCB*,char*, int);
int sendMemoryRequest(MemoryRequest, int, void*, PageOwnership*);


bool viableRequest(int);
bool fsSock(int, connHandle*);
bool memSock(int, connHandle*);
bool cpuSock(int, connHandle*);
bool consSock(int, connHandle*);
bool isListener(int, connHandle);


void newProcess(PCB*);
void enviarAlgoritmo(int);
void aceptarNuevoCPU(int);
void closeHandle(int, connHandle*);
void recibirDeCPU(int, connHandle*);
void recibirDeConsola(int, connHandle*);
void handleSockets(connHandle*, socketHandler);


void* serializarScript(int, int, int, int*, void*);


t_list* findProcessPages(int);


MemoryRequest deserializeMemReq(void*);


#endif
