#include "Listen.h"


int max(int a, int b)
{
	return a > b ? a : b;
}

void handler(configFile* config){
	t_list* procesos= list_create();
	connHandle handleMaster= initializeConnectionHandler();
	socketHandler handleResult;
	initialize(config, procesos, &handleMaster);
	while(1){
		handleResult= updateSockets(handleMaster);
		handleResult= lSelect(handleResult,0);
		handleSockets(&handleMaster, handleResult);
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
	int maxSocket= max(max(master.consola.nfds,master.cpu.nfds),max(max(master.memoria+1, master.fs+1), max(master.listenCPU+1,master.listenConsola+1)));
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

void initialize(configFile* config,t_list* procesos, connHandle* handleMaster){
	int conexionConsola= getBindedSocket(LOCALHOST,config->PUERTO_PROG);
	int conexionCPU = getBindedSocket(LOCALHOST, config->PUERTO_CPU);
	int conexionMemoria = getConnectedSocket(config->IP_MEMORIA, config->PUERTO_MEMORIA, KERNEL_ID);
	int conexionFS = getConnectedSocket(config->IP_FS, config->PUERTO_FS, KERNEL_ID);
	lListen(conexionConsola,BACKLOG);
	lListen(conexionCPU, BACKLOG);
	handleMaster->listenConsola= conexionConsola;
	handleMaster->listenCPU= conexionCPU;
	handleMaster->fs = conexionFS;
	handleMaster->memoria = conexionMemoria;
}

void destroyConnHandler(connHandle* handleMaster){


}
