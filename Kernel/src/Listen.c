#include "Listen.h"
#include "ConnectionCore.h"

#define max(a,b) a > b ? a : b

void handler(configFile* config){
	t_list* procesos= list_create();
	connHandle handleMaster= initializeConnectionHandler();
	socketHandler handleResult;
	initialize(config, procesos, &handleMaster);
	char* info = NULL;
	while(1){
		handleResult= updateSockets(handleMaster);
		handleSockets(&info, &handleMaster, handleResult);
		destroySocketHandler(handleResult);
	}
	free(info);
	destroySocketHandler(handleMaster);
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
		if(memSock(p,master) || cpuSock(p,master) || fsSock(p,master) || consSock(p,master) || isListener(p,master)){
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
	addWriteSocket(conexionMemoria, handleMaster->memoria->readSockets);
	addWriteSocket(conexionFS, handleMaster->fs->readSockets);
}

bool memSock(int p, connHandle master){
	return FD_ISSET(p,master.memoria.readSockets);
}

bool cpuSock(int p, connHandle master){
	return FD_ISSET(p,master.cpu.readSockets);
}

bool fsSock(int p, connHandle master){
	return FD_ISSET(p,master.fs.readSockets);
}

bool consSock(int p, connHandle master){
	return FD_ISSET(p,master.consola.readSockets);
}

bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola );
}
