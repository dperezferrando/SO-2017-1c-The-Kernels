#include "SocketLibrary.h"

void handleSockets(int listener, int listener2, char** info, socketHandler* master, socketHandler result){
	int p;
	int res=0;
	puts("Entro al for\n");
	for(p=0;p<=(result.nfds);p++){

		if(isReading(p,result))
		{
			res=1;
			puts("esta leyendo\n");
			if(p==listener){
				puts("Es listener\n");
				int unaConsola = lAccept(p, CONSOLA_ID);
				addReadSocket(unaConsola,master);
				free(*info);
				//*info = NULL;

				puts("new connection\n");
			}
			else if(p==listener2)
			{
				puts("Es listener\n");
				int unCPU = lAccept(p, CPU_ID);
				addWriteSocket(unCPU,master);
				puts("new connection CPU\n");
			}
			else{

				puts("Habemus Data\n");

				*info = lRecv(p);

				if(*info==NULL){
					puts("Se cierra la conexion");
					closeConnection(p,master);
					continue;
					}

				//handleData(data);
				//enviarALosDemas(data, conexionMemoria, conexionFS);
				//free(data);

			}

		}
		else if(isWriting(p, result) && *info != NULL)
		{
			char data[25];
			strcpy(data,*info);
			lSend(p, data, strlen(data)+1);
			free(*info);
			//closeConnection(p, master);
		}
	}
	if(!res)puts("No hay sockets interesantes\n");
}

