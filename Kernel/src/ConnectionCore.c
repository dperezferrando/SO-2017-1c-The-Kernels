#include "SocketLibrary.h"

void handleSockets(int listener, socketHandler* master, socketHandler result){
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
				if(id != 4)
					closeConnection(p, master);
				int* confirmacion = malloc(sizeof(int));
				*confirmacion = 1;
				lSend(p, 0, confirmacion, sizeof(confirmacion));
				free(confirmacion);
				char* data= lRecv(p,&tipoOperacion);
				if(data==NULL){
					puts("Se cierra la conexion");
					closeConnection(p,master);
					free(data);
					continue;
					}
				handleData(data);
				free(data);

			}
		}
	}
	if(!res)puts("No hay sockets interesantes\n");
}
