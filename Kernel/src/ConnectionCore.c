#include "SocketLibrary.h"


void handleSockets(int listener, int listener2, char** info, socketHandler* master, socketHandler result){
	int p;
//	int res=0;
//	puts("Entro al for\n");
	for(p=0;p<=(result.nfds);p++)
	{
		if(isReading(p,result))
		{
//			res=1;
//			puts("esta leyendo\n");
			if(p==listener){
//				puts("Es listener\n");
				int unaConsola = lAccept(p, CONSOLA_ID);
				addReadSocket(unaConsola,master);
//				puts("new connection\n");
			}
			else if(p==listener2)
			{
//				puts("Es listener\n");
				int unCPU = lAccept(p, CPU_ID);
				addWriteSocket(unCPU,master);
//				puts("new connection CPU\n");
			}
			else{

//				puts("Habemus Data\n");
				char* data= lRecv(p);
				if(data == NULL)
				{
					free(*info);
					*info = NULL;
//					puts("Se cierra la conexion");
					closeConnection(p,master);
				}
				else *info = data;
			}

		}
		else if(isWriting(p, result) && *info != NULL)
			lSend(p, *info, strlen(*info)+1);
	}

//	if(!res)puts("No hay sockets interesantes\n");
}

