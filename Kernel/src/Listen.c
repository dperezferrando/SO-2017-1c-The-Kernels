#include "Listen.h"
#include "ConnectionCore.h"

socketHandler a();

socketHandler a(){
	socketHandler b;
	b.nfds=0;
	b.readSockets=malloc(250*sizeof(fd_set));
	b.writeSockets=malloc(250*sizeof(fd_set));
	return b;
}



void handler(){
	socketHandler sHandlerMaster= a();
	//socketHandler sHandlerControl;
	socketHandler sHandlerResult=a();
	int listener= getBindedSocket(LOCALHOST,PUERTO);
	lListen(listener,BACKLOG);
	addReadSocket(listener,&sHandlerMaster);
	puts("Sockets Listos, entramos al while\n");
	while(1){
		sHandlerResult= lSelect(sHandlerMaster,15);
		handleSockets(listener,&sHandlerMaster,sHandlerResult);
		free(sHandlerResult.readSockets);
		free(sHandlerResult.writeSockets);
		sHandlerResult=a();
	}
		free(sHandlerMaster.readSockets);
		free(sHandlerMaster.writeSockets);
		free(sHandlerResult.readSockets);
		free(sHandlerResult.writeSockets);
}

