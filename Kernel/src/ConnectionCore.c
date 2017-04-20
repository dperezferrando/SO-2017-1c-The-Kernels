#include "SocketLibrary.h"
#include "ConnectionCore.h"


void handleSockets(char** info, connHandle* master, socketHandler result){
	int p;
	for(p=0;p<=(result.nfds);p++)
	{
		if(isReading(p,result))
		{
			if(p==master->listenConsola){
				int unaConsola = lAccept(p, CONSOLA_ID);
				addReadSocket(unaConsola,&(master->consola));
			}
			else if(p==master->listenCPU)
			{
				int unCPU = lAccept(p, CPU_ID);
				addWriteSocket(unCPU,&(master->cpu));
			}
			else{
				char* data= lRecv(p);
				if(data == NULL)
				{
					free(*info);
					*info = NULL;
					closeConnection(p,master);
				}
				else {
					*info = data;
					handleConnection(p,master,data);
				}
			}

		}
		else if(isWriting(p, result) && *info != NULL)
			lSend(p, *info, strlen(*info)+1);
	}
}

void handleConnection(int p, connHandle* master, void* msg){

	if(memSock(p,master))memMsg(msg);
		else if (cpuSock(p,master))cpuMsg(msg);
			else if (fsSock(p,master))fsMsg(msg);
				else if (consSock(p,master)) consMsg(msg);

}



bool memSock(int p, connHandle* master){
	return FD_ISSET(p,master->memoria.readSockets);
}

bool cpuSock(int p, connHandle* master){
	return FD_ISSET(p,master->cpu.readSockets);
}

bool fsSock(int p, connHandle* master){
	return FD_ISSET(p,master->fs.readSockets);
}

bool consSock(int p, connHandle* master){
	return FD_ISSET(p,master->consola.readSockets);
}

bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola );
}
