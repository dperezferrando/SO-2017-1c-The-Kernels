#include "Listen.h"


#define max(a,b) a > b ? a : b

void handler(configFile* config){
	t_list* procesos= list_create();
	connHandle handleMaster= initializeConnectionHandler();
	socketHandler handleResult;
	initialize(config, procesos, &handleMaster);
	char* info = NULL;
	while(1){
		handleResult= updateSockets(handleMaster);
		handleResult= lSelect(handleResult,0);
		handleSockets(&info, &handleMaster, handleResult);
		destroySocketHandler(handleResult);
	}
	free(info);
	destroyConnHandler(&handleMaster);
}

connHandle initializeConnectionHandler(){
	connHandle aux;
	aux.consola= initializeSocketHandler();
	aux.memoria= initializeSocketHandler();
	aux.cpu= initializeSocketHandler();
	aux.fs= initializeSocketHandler();
	return aux;
}

socketHandler updateSockets(connHandle master){
	fd_set* aux= malloc(sizeof(master.consola.readSockets)*4);
	int maxSocket= max(max(max(master.consola.nfds,master.cpu.nfds),max(master.fs.nfds,master.memoria.nfds)),max(master.listenCPU,master.listenConsola));
	int p;
	for(p=0;p<maxSocket;p++){
		if(memSock(p,&master) || cpuSock(p,&master) || fsSock(p,&master) || consSock(p,&master) || isListener(p,master)){
			FD_SET(p,aux);
		}
	}

	socketHandler response;
	* (response.readSockets)  = * (aux);
	* (response.writeSockets) = * (aux);
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
	addReadSocket(conexionMemoria, &handleMaster->memoria);
	addReadSocket(conexionFS, &handleMaster->fs);
}

void destroyConnHandler(connHandle* handleMaster){

}
