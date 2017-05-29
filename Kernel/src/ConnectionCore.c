#include "ConnectionCore.h"

//----------------------------------------------------Main Function---------------------------------------------------------//

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




//---------------------------------------------------Funciones de Consola---------------------------------------------------//


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
			if(enviarScriptAMemoria(pcb, mensaje->data, tamanioScript))
			{
				lSend(socket, &pcb->pid, 2, sizeof(int));
				newProcess(pcb);
				readyProcess(pcb);
			}
			else
			{
				puts("NO HAY ESPACIO");
				lSend(socket, NULL, -2, 0);
			}

			break;
		}
		case 9:
			killProcess(mensaje->data);
			int tamanioScriptSerializado = 0;
			void* buffer= serializarScript( * ( (int*) mensaje->data ) , 0 , 0 , & tamanioScriptSerializado, "");
			//solo me interesa mandarle el kill, no tengo ningun mensaje que pasarle hasta ahora al menos
			lSend(conexionMemoria, buffer, 9, tamanioScriptSerializado);
			break;
	}

	destruirMensaje(mensaje);

}

bool consSock(int p, connHandle* master){
	return FD_ISSET(p,&master->consola.readSockets);
}




//------------------------------------------------Funciones de Memoria-------------------------------------------------------//


int enviarScriptAMemoria(PCB* pcb, char* script, int tamanioScript)
{
	int tamanioScriptSerializado = 0;
	int paginasTotales = pcb->cantPaginasCodigo + config->STACK_SIZE;
	void* buffer = serializarScript(pcb->pid, tamanioScript, paginasTotales, &tamanioScriptSerializado, script);
	lSend(conexionMemoria, buffer, 1, tamanioScriptSerializado);
	Mensaje* respuesta = lRecv(conexionMemoria);
	free(buffer);
	int to= respuesta->header.tipoOperacion;
	free(respuesta);
	return to == 104;
}

MemoryRequest deserializeMemReq(void* mr){
	MemoryRequest res;
	memcpy(&res.pid,mr,sizeof(int));
	memcpy(&res.size,mr+sizeof(int),sizeof(int));
	return res;
}

void* serializeMemReq(MemoryRequest mr){
	void* res= malloc(sizeof(mr));
	memcpy(res,&mr.pid,sizeof(int));
	memcpy(res,&mr.size,sizeof(int));
	return res;
}

int memoryRequest(MemoryRequest mr, int size, void* msg){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	if(sendMemoryRequest(mr,size,msg,po)==-1) return -1;
	if(!test) res= lRecv(conexionMemoria);
	if(res->header.tipoOperacion==-1) return -1;
	if (po->idpage!=0){//el pedido se grabo en la pagina ya asignada
		HeapMetadata* hm= malloc(sizeof(HeapMetadata));
		memcpy(hm,res->data+(sizeof(int)*2),sizeof(HeapMetadata));
		list_add(po->occSpaces,hm);
		list_replace(ownedPages, PIDFindPO(mr.pid), po);
		return 0;
	}
		else{//se le otorgo una pagina y se grabo alli
			memcpy(&po->pid,res->data,sizeof(int));
			memcpy(&po->idpage,res->data+sizeof(int),sizeof(int));
			po->occSpaces=list_create();
			list_add(ownedPages,po);
			free(res);
			return 1;
		}
}

int sendMemoryRequest(MemoryRequest mr, int size, void* msg, PageOwnership* po){
	if(!viableRequest(mr.size)) return -1;
	po->pid= mr.pid;
	po->idpage= pageToStore(mr);
	void* serializedMSG= serializeMemReq(mr);
	memcpy(serializedMSG,&po->idpage,sizeof(int));
	memcpy(serializedMSG,msg,size);
	if(!test) lSend(conexionMemoria,serializedMSG,3,sizeof(mr)+size);
	return 1;
}

int pageToStore(MemoryRequest mr){
	t_list* processPages= list_create();
	findProcessPages(mr.pid,processPages);
	bool pageHasEnoughSpace(PageOwnership* po){
		int i,acc=0;
		for(i=0;i<list_size(po->occSpaces);i++){
			HeapMetadata* hw= list_get(po->occSpaces,i);
			acc+=hw->size;
		}
		return acc<= mr.size+sizeof(HeapMetadata);
	}
	PageOwnership* po = list_find(processPages,&pageHasEnoughSpace);
	if(po!=NULL) {return po->idpage;} else {return 0;}
}

void findProcessPages(int pid, t_list* processPages){
	bool _PIDFind(PageOwnership* po){
		return po->pid== pid;
	}
	processPages= list_filter(ownedPages,&(_PIDFind));
}

//------------------------------------------------Funciones de CPU----------------------------------------------------------//


void recibirDeCPU(int socket, connHandle* master)
{
	puts("CPU");
	PCB* pcb= malloc(sizeof(PCB));
	Mensaje* mensaje = lRecv(socket);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			closeHandle(socket, master);
			break;
		case 1:
			puts("SE TERMINO LA EJECUCION DE UN PROCESO. SE DEBERIA ENVIAR A COLA FINALIZADO EL PCB");
			pcb = recibirPCB(mensaje);
			break;
		case 2:
			puts("TERMINA EJECUCION DE PROCESO PERO ESTE NO ESTA FINALIZADO");
			pcb = recibirPCB(mensaje);
			cpuReturnsProcessTo(pcb,1);
			break;
		case 3:
			puts("PROCESO PIDE MEMORIA");
			MemoryRequest mr= deserializeMemReq(mensaje->data);
			memoryRequest(mr,mensaje->header.tamanio-sizeof(mr),mensaje->data+sizeof(mr));
			break;
		case 4:
			puts("TERMINA EJECUCION DE PROCESO Y ESTE VA A BLOCKED");
			pcb= recibirPCB(mensaje);
			cpuReturnsProcessTo(pcb,3);
			break;
	}
	destruirMensaje(mensaje);
	free(pcb);
}


void aceptarNuevoCPU(int unCPU)
{
	puts("NUEVO CPU");
	queue_push(colaCPUS, (int*)unCPU);
	lSend(unCPU, &config->PAG_SIZE, 0, sizeof(int));
	enviarAlgoritmo(unCPU);
	puts("AGREGADO CPU A COLA");
}


void enviarAlgoritmo(int CPU){
	int algoritmo=0;
	if(strcmp(config->ALGORITMO, "RR")){algoritmo= config->QUANTUM;}
	lSend(CPU, &algoritmo, 10, sizeof(int));
}


bool cpuSock(int p, connHandle* master){
	return FD_ISSET(p,&master->cpu.readSockets);
}




//---------------------------------------------------Funciones Auxiliares----------------------------------------------------//


void* serializarScript(int pid, int tamanio, int paginasTotales, int* tamanioSerializado, void* script)
{
	*tamanioSerializado = tamanio + (sizeof(int)*2);
	void* buffer = malloc(*tamanioSerializado);
	memcpy(buffer, &pid, sizeof(int));
	memcpy(buffer + sizeof(int), &paginasTotales, sizeof(int));
	memcpy(buffer + sizeof(int)*2, script, tamanio);
	return buffer;
}

PCB* recibirPCB(Mensaje* mensaje){
	PCB* pcb = deserializarPCB(mensaje->data);
	printf("RECIBIDO PCB: PID: %i\n", pcb->pid);
	return pcb;
}


void closeHandle(int s, connHandle* master)
{
	closeConnection(s, &(master->consola));
	closeConnection(s, &(master->cpu));
}


int checkMultiprog(){
	int currentMultiprog= queue_size(colaReady) + list_size(blockedList) + list_size(executeList);
	return config->GRADO_MULTIPROG > currentMultiprog;
}


bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola );
}

bool viableRequest(int requestSize){
	return ((config->PAG_SIZE)-10) >= requestSize;
}

int PIDFindPO(int PID){
	bool _PIDFind(PageOwnership* po){
		return po->pid== PID;
	}
	return *((int*)list_find(ownedPages,&(_PIDFind)));
}


