#include "SocketLibrary.h"
#include "ConnectionCore.h"
#include "Configuration.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include "KernelConfiguration.h"
#include "globales.h"



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
				aceptarNuevoCPU(unCPU);
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

void aceptarNuevoCPU(int unCPU)
{
	puts("NUEVO CPU");
	queue_push(colaCPUS, (int*)unCPU);
	lSend(unCPU, &config->PAG_SIZE, 0, sizeof(int));
	puts("AGREGADO CPU A COLA");
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
		case 1:
		{
			// TO DO: CHEQUEAR QUE HAY ESPACIO EN MEMORIA
			int tamanioScript = mensaje->header.tamanio;
			PCB* pcb = createProcess(mensaje->data, tamanioScript);
			lSend(socket, &pcb->pid, 2, sizeof(int));
			int tamanioScriptSerializado = 0;
			int paginasTotales = pcb->cantPaginasCodigo + config->STACK_SIZE;
			void* buffer = serializarScript(pcb->pid, tamanioScript, paginasTotales, &tamanioScriptSerializado, mensaje->data);
			lSend(conexionMemoria, buffer, 1, tamanioScriptSerializado);
			queue_push(colaReady, pcb);
			// MANDO A EJECUTAR ACANOMA
			PCBSerializado pcbSerializado = serializarPCB(pcb);
			int CPU = (int)queue_pop(colaCPUS);
			lSend(CPU, pcbSerializado.data, 1, pcbSerializado.size);
			puts("PCB ENVIADO");
			free(pcbSerializado.data);
			free(buffer);
			break;
		}
		case 9:
			/*killProcess(procesos,mensaje->data);
			tamanioScriptSerializado = 0;
			buffer= serializarScript( * ( (int*) mensaje->data ) , 0 , 0 , & tamanioScriptSerializado, "");
			//solo me interesa mandarle el kill, no tengo ningun mensaje que pasarle hasta ahora al menos
			lSend(conexionMemoria, buffer, 9, tamanioScriptSerializado);*/
			break;
	}

	destruirMensaje(mensaje);

}



PCBSerializado serializarPCB(PCB* pcb) // A SER REEMPLAZADO POR LO DE NICO
{
	PCBSerializado pcbSerializado;
	pcbSerializado.size = sizeof(int)*4 + pcb->sizeIndiceCodigo;
	pcbSerializado.data = malloc(pcbSerializado.size);
	memcpy(pcbSerializado.data, &pcb->pid, sizeof(int));
	memcpy(pcbSerializado.data + sizeof(int), &pcb->cantPaginasCodigo, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*2), &pcb->programCounter, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*3), &pcb->sizeIndiceCodigo, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*4), pcb->indiceCodigo, pcb->sizeIndiceCodigo);
	return pcbSerializado;

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
