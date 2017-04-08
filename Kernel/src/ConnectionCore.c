#include "SocketLibrary.h"

void handleSockets(int listener, socketHandler* list, socketHandler result){
	int p;
	int res=0;
	puts("Entro al for\n");
	for(p=0;p<=(result.nfds);p++){
		if(isReading(p,result)){
			res=1;
			puts("esta leyendo\n");
			if(p==listener){
				puts("Es listener\n");
				addReadSocket(lAccept(p),list);
				puts("new connection\n");
			}
			else{
				int tipoOperacion;
				puts("Habemus Data\n");
				char* data= lRecv(p,&tipoOperacion);
				handleData(data);
				if (data!=NULL)free(data);
			}
		}
	}
	if(!res)puts("No hay sockets interesantes\n");
}
