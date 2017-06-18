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
			int tamanioScript = mensaje->header.tamanio;
			PCB* pcb = createProcess(mensaje->data, tamanioScript);
			if(enviarScriptAMemoria(pcb, mensaje->data, tamanioScript))
			{
				lSend(socket, &pcb->pid, 2, sizeof(int));
				newProcess(pcb, socket);
				if(readyProcess(pcb) == -1)
					puts("SE ALCANZO EL LIMITE DE MULTIPROGRAMACION - QUEDA EN NEW");

			}
			else
			{
				puts("NO HAY ESPACIO");
				lSend(socket, NULL, -2, 0);
			}

			break;
		}
		//Abortar procesos
		case 3:
			lSend(socket, mensaje->data, 3, sizeof(int));
			break;

		//Para cerrar el hilo receptor de la consola
		case 4:
			lSend(socket, mensaje->data, 4, sizeof(int));
			break;

		case 9:
			lSend(socket, mensaje->data, 9, sizeof(int));
			killProcess(mensaje->data);
			int tamanioScriptSerializado = 0;
			lSend(conexionMemoria, NULL, 9, tamanioScriptSerializado);
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
	memcpy(res+sizeof(int),&mr.size,sizeof(int));
	return res;
}

int memoryRequest(MemoryRequest mr, int size, void* contenido){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	HeapMetadata* hm= initializeHeapMetadata(mr.size);
	if(sendMemoryRequest(mr,size,contenido,po)==-1) return -1;
	if(!test) res= lRecv(conexionMemoria);
	if(res->header.tipoOperacion==-1) return -1;
	int operation= grabarPedido(po,mr,hm);
	free(res);
	return operation;
}

HeapMetadata* initializeHeapMetadata(int size){
	HeapMetadata* hm= malloc(sizeof(HeapMetadata));
	hm->isFree=0;
	hm->size= size;
	return hm;
}

int grabarPedido(PageOwnership* po, MemoryRequest mr, HeapMetadata* hm){

	if (po->idpage==-1){//el pedido se grabo en pagina nueva
		memcpy(po,res->data,sizeof(int));
		initializePageOwnership(po);//aca queda el PageOwnership con la estructura que marca el espacio libre
	}
	int pos= occupyPageSize(po,hm);//guarda el heapMetadata correspondiente en el PageOwnership
	storeVariable(po,mr.variable,pos);//guarda la estructura de control de variables

	if (po->idpage!=-1){
		list_replace(ownedPages, &PIDFindPO, po);
		return 0;
	}

	list_add(ownedPages,po);
	return 1;

}

int sendMemoryRequest(MemoryRequest mr, int size, void* msg, PageOwnership* po){
	if(!viableRequest(mr.size)) return -1;
	po->pid= mr.pid;
	po->idpage= pageToStore(mr);//busco la pagina para guardarlo, si no hay -1
	void* serializedMSG= serializeMemReq(mr);
	memcpy(serializedMSG,&po->idpage,sizeof(int));
	memcpy(serializedMSG,msg,size);
	if(!test) lSend(conexionMemoria,serializedMSG,3,sizeof(mr)+size);//si el idPag es 0 no tiene pagina donde se puede grabar y necesita una nueva
	free(serializedMSG);
	return 1;
}

int pageToStore(MemoryRequest mr){
	t_list* processPages= findProcessPages(mr.pid);

	bool pageHasEnoughSpace(PageOwnership* po){
		int i,acc=0;
		for(i=0;i<list_size(po->occSpaces);i++){
			HeapMetadata* hw= list_get(po->occSpaces,i);
			if(hw->isFree==1)acc+=hw->size;
		}
		return acc >= mr.size+sizeof(HeapMetadata);
	}
	PageOwnership* po = list_find(processPages,&(pageHasEnoughSpace));
	if(po!=NULL) return po->idpage;
	else return -1;
}

t_list* findProcessPages(int pid){
	bool _PIDFind(PageOwnership* po){
		return po->pid== pid;
	}
	return list_filter(ownedPages,&(_PIDFind));
}

void initializePageOwnership(PageOwnership* po){
	po->occSpaces= list_create();
	po->control= list_create();
	HeapMetadata* hm= malloc(sizeof(HeapMetadata));
	hm->isFree=1;
	hm->size= config->PAG_SIZE-sizeof(HeapMetadata);
	list_add(po->occSpaces,hm);
}

int occupyPageSize(PageOwnership* po,HeapMetadata* hm){
	int i,pos;
	for(i=0;i<list_size(po->occSpaces);i++){
		HeapMetadata* hw= list_get(po->occSpaces,i);
		if(hw->isFree==1){
			if(hw->size >= (hm->size+sizeof(HeapMetadata))){
				hw->size-=(hm->size+sizeof(HeapMetadata));
				list_replace(po->occSpaces,i,hm);//reemplazo el elemento libre por uno ocupado
				list_add_in_index(po->occSpaces,i+1,hw);//agrego la estructura libre al lado
				pos= i;
				break;
			}
		}
	}
	return i;
}

void storeVariable(PageOwnership* po, char* name, int pos){
	HeapControl* hc= malloc(sizeof(HeapControl));
	hc->name=name;
	hc->listPosition= pos;
	list_add(po->control,hc);
}

//------------------------------------------------Funciones de CPU----------------------------------------------------------//


void recibirDeCPU(int socket, connHandle* master)
{
	puts("CPU");
	PCB* pcb= malloc(sizeof(PCB));
	Mensaje* mensaje = lRecv(socket);
	pcb = recibirPCB(mensaje);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			closeHandle(socket, master);
			break;
		case 1:
			puts("SE TERMINO LA EJECUCION DE UN PROCESO. SE DEBERIA ENVIAR A COLA FINALIZADO EL PCB");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			pcb->exitCode = 0;
			// HAY QUE ENVIAR EL PCB RECIBIDO A LA COLA FINISHED, EN ESTE MOMENTO NO SE HACE, SE PASA EL PCB VIEJO QUE ESTA EN LA COLA EXECUTED.
			killProcess(pcb->pid);
			queue_push(colaCPUS, socket);
			executeProcess();
			break;
		case 2:
			puts("TERMINA EJECUCION DE PROCESO PERO ESTE NO ESTA FINALIZADO");
			cpuReturnsProcessTo(pcb,1);
			executeProcess();
			break;
		case 3:
			puts("TERMINA EJECUCION DE PROCESO Y ESTE VA A BLOCKED");
			cpuReturnsProcessTo(pcb,3);
			executeProcess();
			break;
		case 4:
			puts("PROGRAMA ABORTADO");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			killProcess(pcb->pid);
			executeProcess();
			queue_push(colaCPUS, socket);
			break;
		case 5:
			puts("PROCESO PIDE MEMORIA");
			MemoryRequest mr= deserializeMemReq(mensaje->data);
			memoryRequest(mr,mensaje->header.tamanio-sizeof(mr),mensaje->data+sizeof(mr));
			break;
	}
	destruirMensaje(mensaje);
	free(pcb);
}


void aceptarNuevoCPU(int unCPU)
{
	puts("NUEVO CPU");
	queue_push(colaCPUS, (int*)unCPU);
	enviarInformacion(unCPU);
	puts("AGREGADO CPU A COLA");
	executeProcess();

}


void enviarInformacion(int CPU){
	int algoritmo=0;
	if(strcmp(config->ALGORITMO, "RR")){algoritmo= config->QUANTUM;}
	char* data = malloc(sizeof(int)*3);
	memcpy(data, &config->STACK_SIZE, sizeof(int));
	memcpy(data + sizeof(int), &config->PAG_SIZE, sizeof(int));
	memcpy(data + sizeof(int)*2, &algoritmo, sizeof(int));
	lSend(CPU, data, 10, sizeof(int)*3);
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


