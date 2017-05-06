#include "SocketLibrary.h"
#include "ConnectionCore.h"
#include <commons/collections/list.h>



void handleSockets(connHandle* master, socketHandler result){
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
				addReadSocket(unCPU,&(master->cpu));
			}
			else if(consSock(p, master))
			{
				recibirDeConsola(p, master);
			}
			else if(cpuSock(p, master))
			{
				recibirDeCPU(p, master);
			}
			// habria que poner lo mismo para fs y memoria

		}
		if(isWriting(p, result))
		{
			/*if(memSock(p, master))
				puts("La memoria espera algo"); // xddd fuck logic
			if(fsSock(p, master))
				puts("El fs espera algo"); // xddd fuck logic*/
		}
		// habria que poner lo mismo para consola y cpu
	}
}

void closeHandle(int s, connHandle* master)
{
	closeConnection(s, &(master->consola));
	closeConnection(s, &(master->cpu));
}

void recibirDeConsola(int socket, connHandle* master)
{
	puts("CONSOLA");
	Mensaje* mensaje = lRecv(socket);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			closeHandle(socket, master);
			break;
		case 1: // TESTING
			;
			t_list* procesos = list_create();
			int bufferSize = mensaje->header.tamanio + sizeof(int);
			void* buffer = malloc(bufferSize);
			int pid = createProcess(procesos);
			memcpy(buffer, &pid, sizeof(int));
			memcpy(buffer + sizeof(int), mensaje->data, mensaje->header.tamanio);
			printf("%s\n", mensaje->data);
			lSend(master->memoria, buffer, 1, bufferSize);
			free(buffer);
			break;
	}

	destruirMensaje(mensaje);
}

void recibirDeCPU(int socket, connHandle* master)
{
	puts("CPU");
	Mensaje* mensaje = lRecv(socket);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			closeHandle(socket, master);
			break;
		case 1: // TESTING
			printf("MENSAJE CPU: %s\n", mensaje->data);
			break;
	}
	destruirMensaje(mensaje);
}



bool cpuSock(int p, connHandle* master){
	return FD_ISSET(p,&master->cpu.readSockets);
}


bool consSock(int p, connHandle* master){
	return FD_ISSET(p,&master->consola.readSockets);
}
bool memSock(int p, connHandle* master){
	return p == master->memoria;
}

bool fsSock(int p, connHandle* master)
{
	return p == master->fs;
}

bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola );
}
