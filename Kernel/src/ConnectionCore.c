#include "SocketLibrary.h"
#include "ConnectionCore.h"


void handleSockets(Mensaje** info, connHandle* master, socketHandler result){
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
				Mensaje* data= lRecv(p);
				if(data->header.tipoOperacion == -1)
				{
					destruirMensaje(data);
					closeConnection(p,master);
				}
				else {
					*info = data;
					handleConnection(p,master,data);
				}
			}

		}
		else if(isWriting(p, result) && *info != NULL)
			lSend(p, (*info)->data, (*info)->header.tipoOperacion,(*info)->header.tamanio);
	}
}

void handleConnection(int p, connHandle* master, Mensaje* msg){

	if(memSock(p,master))memMsg(p, msg);
		else if (cpuSock(p,master))cpuMsg(p, msg);
			else if (fsSock(p,master))fsMsg(p, msg);
				else if (consSock(p,master)) consMsg(p, msg);

}
// funciones implementadas humo para testear
void memMsg(int socket, Mensaje* msg)
{
	puts("SOY MEMORIA");
	lSend(socket, msg->data, msg->header.tipoOperacion, msg->header.tamanio);
}

void cpuMsg(int socket, Mensaje* msg)
{
	puts("SOY CPU");
	lSend(socket, msg->data, msg->header.tipoOperacion, msg->header.tamanio);
}

void fsMsg(int socket, Mensaje* msg)
{
	puts("SOY FS");
	lSend(socket, msg->data, msg->header.tipoOperacion, msg->header.tamanio);
}

void consMsg(int socket, Mensaje* msg)
{
	puts("SOY CONSOLA");

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
