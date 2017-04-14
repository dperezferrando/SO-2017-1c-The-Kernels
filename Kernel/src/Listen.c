#include "Listen.h"
#include "ConnectionCore.h"



void handler(configFile* config){
	socketHandler sHandlerMaster= initializeSocketHandler();
	//socketHandler sHandlerControl;
	socketHandler sHandlerResult=initializeSocketHandler();
	int listener= getBindedSocket(LOCALHOST,config->PUERTO_PROG);
	int conexionMemoria = getConnectedSocket(config->IP_MEMORIA, config->PUERTO_MEMORIA, KERNEL_ID);
	int conexionFS = getConnectedSocket(config->IP_FS, config->PUERTO_FS, KERNEL_ID);
	lListen(listener,BACKLOG);
	addReadSocket(listener,&sHandlerMaster);
	puts("Sockets Listos, entramos al while\n");
	while(1){
		sHandlerResult= lSelect(sHandlerMaster,15);
		handleSockets(listener,conexionMemoria, conexionFS, &sHandlerMaster,sHandlerResult);
		free(sHandlerResult.readSockets);
		free(sHandlerResult.writeSockets);
		sHandlerResult=initializeSocketHandler();
	}
		free(sHandlerMaster.readSockets);
		free(sHandlerMaster.writeSockets);
		free(sHandlerResult.readSockets);
		free(sHandlerResult.writeSockets);
}
//aa
