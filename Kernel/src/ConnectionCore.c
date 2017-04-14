#include "SocketLibrary.h"

void handleSockets(int listener, int conexionMemoria, int conexionFS, socketHandler* master, socketHandler result){
	int p;
	int res=0;
	puts("Entro al for\n");
	for(p=0;p<=(result.nfds);p++){
		if(isReading(p,result)){
			res=1;
			puts("esta leyendo\n");
			if(p==listener){
				puts("Es listener\n");
				addReadSocket(lAccept(p),master);
				puts("new connection\n");
			}
			else{
				int tipoOperacion;
				puts("Habemus Data\n");
				int id = recibirHandShake(p);
				if(id != CONSOLA_ID)
				{
					closeConnection(p, master);
					break;
				}
				int* confirmacion = malloc(sizeof(int));
				*confirmacion = 1;
				lSend(p, 0, confirmacion, sizeof(confirmacion));
				free(confirmacion);
				char data[25];
				strcpy(data,lRecv(p,&tipoOperacion));
				if(data==NULL){
					puts("Se cierra la conexion");
					closeConnection(p,master);
					free(data);
					continue;
					}
				//handleData(data);
				enviarALosDemas(data, conexionMemoria, conexionFS);
				//free(data);

			}
		}
	}
	if(!res)puts("No hay sockets interesantes\n");
}

void enviarALosDemas(char* data, int conexionMemoria, int conexionFS)
{
	lSend(conexionMemoria, 0, data, strlen(data));
	lSend(conexionFS, 0, data, strlen(data));
}
