#include "Listen.h"
#include "ConnectionCore.h"


void handler(configFile* config){
	t_list* procesos= list_create();
	connHandle handleMaster= initializeConnectionHandler();
	connHandle handleResult;
	initialize(config, procesos, &handleMaster);
	char* info = NULL;
	while(1){
		handleResult= updateSockets(handleMaster);
		handleSockets(&info, &handleMaster);
		destroySocketHandler(handlerResult);
	}
	free(info);
	destroySocketHandler(sHandlerMaster);
}

connHandle initializeConnectionHandler(){
	connHandle aux;
	aux.consola= initializeSocketHandler();
	aux.memoria= initializeSocketHandler();
	aux.cpu= initializeSocketHandler();
	aux.fs= initializeSocketHandler();
	return aux;
}

connHandle updateSockets(connHandle master){

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
	addWriteSocket(conexionMemoria, handleMaster->memoria->readSockets);
	addWriteSocket(conexionFS, handleMaster->fs->readSockets);
}
