#include "SocketLibrary.h"
#include "ConnectionCore.h"
#include "Configuration.h"
#include <commons/collections/list.h>
#include <parser/metadata_program.h>
#include "KernelConfiguration.h"
#include "globales.h"
#include <math.h>

void* serializarScript(int pid, int tamanio, int paginasTotales, int* tamanioSerializado, void* script);

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

		}
		if(isWriting(p, result))
		{
			// enviar a consola y cpu
		}

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
	PCB* proceso;
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			closeHandle(socket, master);
			break;
		case 1:
			proceso = createProcess(procesos);
			lSend(socket, &proceso->pid, 2, sizeof(int));
			int tamanioScript = mensaje->header.tamanio;
			int tamanioScriptSerializado = 0;
			proceso->cantPaginasCodigo = ceil((double)tamanioScript/(double)config->PAG_SIZE);
			int paginasTotales = proceso->cantPaginasCodigo + config->STACK_SIZE;
			void* buffer = serializarScript(proceso->pid, tamanioScript, paginasTotales, &tamanioScriptSerializado, mensaje->data);
			lSend(conexionMemoria, buffer, 1, tamanioScriptSerializado);
			/*t_metadata_program* metadata= metadata_desde_literal(mensaje->data);
			proceso->programCounter= metadata->instruccion_inicio;
			proceso->indiceCodigo= malloc(metadata->instrucciones_size * 2 * sizeof(int));*/

			list_add(procesos, proceso);
			free(buffer);
			break;
		case 9:
			killProcess(procesos,mensaje->data);
			tamanioScriptSerializado = 0;
			buffer= serializarScript( * ( (int*) mensaje->data ) , 0 , 0 , & tamanioScriptSerializado, "");
			//solo me interesa mandarle el kill, no tengo ningun mensaje que pasarle hasta ahora al menos
			lSend(conexionMemoria, buffer, 9, tamanioScriptSerializado);
			break;
	}

	destruirMensaje(mensaje);

}

void* serializarScript(int pid, int tamanio, int paginasTotales, int* tamanioSerializado, void* script)
{
	*tamanioSerializado = tamanio + (sizeof(int)*2);
	void* buffer = malloc(*tamanioSerializado);
	memcpy(buffer, &pid, sizeof(int));
	memcpy(buffer + sizeof(int), &paginasTotales, sizeof(int));
	memcpy(buffer + sizeof(int)*2, script, tamanio);
	return buffer;
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


bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola );
}
