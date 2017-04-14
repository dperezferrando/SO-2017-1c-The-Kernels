#include "Listen.h"
#include "ConnectionCore.h"



void handler(configFile* config){
	socketHandler sHandlerMaster= initializeSocketHandler();
	//socketHandler sHandlerControl;
	socketHandler sHandlerResult;//=initializeSocketHandler();
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
	puts("Sockets Listos, entramos al while\n");
	char* info = NULL;
	char c = '0';
	while(c != 'c'){
		sHandlerResult= lSelect(sHandlerMaster,15);
		handleSockets(conexionConsola,conexionCPU, &info, &sHandlerMaster,sHandlerResult);
		FD_ZERO(sHandlerResult.readSockets);
		free(sHandlerResult.readSockets);
		FD_ZERO(sHandlerResult.writeSockets);
		free(sHandlerResult.writeSockets);
		//sHandlerResult=initializeSocketHandler();
		c = getchar();
	}
	free(info);
	FD_ZERO(sHandlerMaster.readSockets);
	free(sHandlerMaster.readSockets);
	FD_ZERO(sHandlerMaster.writeSockets);
	free(sHandlerMaster.writeSockets);
	FD_ZERO(sHandlerResult.readSockets);
	free(sHandlerResult.readSockets);
	FD_ZERO(sHandlerResult.writeSockets);
	free(sHandlerResult.writeSockets);

}
//aa
