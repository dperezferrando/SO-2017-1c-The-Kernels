#include "SocketLibrary.h"
#include "KernelConfiguration.h"
#include <commons/collections/list.h>

typedef struct connectionHandler{
	socketHandler memoria;
	socketHandler fs;
	socketHandler cpu;
	socketHandler consola;
	int listenCPU;
	int listenConsola;
}connHandle;

#define PUERTO "7171"
#define PUERTO2 "7172"
#define LOCALHOST "127.0.0.1"
#define BACKLOG 10


void handler(configFile* config);
