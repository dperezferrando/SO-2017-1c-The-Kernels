#include "Listen.h"
#include "ConnectionCore.h"



void handler(configFile* config){
	socketHandler sHandlerMaster= initializeSocketHandler();
	socketHandler sHandlerResult;
	int conexionConsola= getBindedSocket(LOCALHOST,config->PUERTO_PROG);
	int conexionCPU = getBindedSocket(LOCALHOST, config->PUERTO_CPU);
	int conexionMemoria = getConnectedSocket(config->IP_MEMORIA, config->PUERTO_MEMORIA, KERNEL_ID);
	int conexionFS = getConnectedSocket(config->IP_FS, config->PUERTO_FS, KERNEL_ID);
	lListen(conexionConsola,BACKLOG);
	lListen(conexionCPU, BACKLOG);
	addReadSocket(conexionConsola,&sHandlerMaster);
	addReadSocket(conexionCPU, &sHandlerMaster);
	addWriteSocket(conexionMemoria, &sHandlerMaster);
	addWriteSocket(conexionFS, &sHandlerMaster);
	char* info = NULL;
	while(1){
		sHandlerResult= lSelect(sHandlerMaster,15);
		handleSockets(conexionConsola,conexionCPU, &info, &sHandlerMaster,sHandlerResult);
		destroySocketHandler(sHandlerResult);
	}
	free(info);
	destroySocketHandler(sHandlerMaster);


}
//aa
