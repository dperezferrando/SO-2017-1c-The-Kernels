#ifndef __CONN_CORE__
#define __CONN_CORE__

#include <commons/collections/list.h>
#include "Configuration.h"
#include <commons/collections/list.h>
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

int readyProcess();
int executeProcess();
int checkMultiprog();
int enviarScriptAMemoria(PCB*,char*, int);

bool fsSock(int, connHandle*);
bool memSock(int, connHandle*);
bool cpuSock(int, connHandle*);
bool consSock(int, connHandle*);
bool isListener(int, connHandle);
void aceptarNuevoCPU(int);
void recibirDeConsola(int, connHandle*);
void handleSockets(connHandle*, socketHandler);
void recibirDeCPU(int, connHandle*);
void newProcess(PCB*);

#endif
