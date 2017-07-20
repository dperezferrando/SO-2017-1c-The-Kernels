#include "Listen.h"
#include "globales.h"


int max(int a, int b)
{
	return a > b ? a : b;
}

void handler(){
	connHandle handleMaster= initializeConnectionHandler();
	socketHandler result;
	initialize(config, &handleMaster);
	while(morir == 0){
		result= updateSockets(handleMaster);
		result= lSelect(result,0);
		handleSockets(&handleMaster, result);
		//destroySocketHandler(handleResult);
	}
	destroyConnHandler(&handleMaster);
}

connHandle initializeConnectionHandler(){
	connHandle aux;
	aux.consola= initializeSocketHandler();
	aux.cpu= initializeSocketHandler();
	return aux;
}

socketHandler updateSockets(connHandle master){
	fd_set aux;
	FD_ZERO(&aux);
	int maxSocket= max(max(max(master.consola.nfds,master.cpu.nfds),max(master.listenCPU+1,master.listenConsola+1)),master.inotify+1);
	int p;
	//printf("max: %i\nSockets:", maxSocket);
	for(p=0;p<maxSocket;p++){
		if(cpuSock(p,&master)|| consSock(p,&master) || isListener(p,master)){
			FD_SET(p,&aux);
		//	printf("%i -", p);
		}
	}
	//puts("");

	socketHandler response = initializeSocketHandler();
	response.readSockets  = aux;
	response.writeSockets = aux;
	response.nfds= maxSocket;

	return response;
}

void initialize(configFile* config,connHandle* handleMaster){
	conexionConsola= getBindedSocket(config->IP_PROPIA,config->PUERTO_PROG);
	conexionCPU = getBindedSocket(config->IP_PROPIA, config->PUERTO_CPU);
	conexionMemoria = getConnectedSocket(config->IP_MEMORIA, config->PUERTO_MEMORIA, KERNEL_ID);
	Mensaje* pagSize = lRecv(conexionMemoria);
	memcpy(&config->PAG_SIZE,pagSize->data, pagSize->header.tamanio);
	destruirMensaje(pagSize);
	conexionFS = getConnectedSocket(config->IP_FS, config->PUERTO_FS, KERNEL_ID);
	maxPID=0;
	lListen(conexionConsola,BACKLOG);
	lListen(conexionCPU, BACKLOG);
	handleMaster->listenConsola= conexionConsola;
	handleMaster->listenCPU= conexionCPU;
	int inotify = inotify_init();
	int watch_inotify = inotify_add_watch(inotify, "/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Kernel/Debug/", IN_MODIFY);
	handleMaster->inotify = inotify;

}

void destroyConnHandler(connHandle* handleMaster){


}
