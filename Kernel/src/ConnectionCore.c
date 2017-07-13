#include "ConnectionCore.h"

//----------------------------------------------------Main Function---------------------------------------------------------//

void handleSockets(connHandle* master, socketHandler result){
	int p = 0;
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
				puts("ENTRA CPU");
				int unCPU = lAccept(p, CPU_ID);
				printf("CPU LISTO: SOCKET: %i", unCPU);
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
		/*	else if(p == master->inotify)
			{
				inotifyStuff();
			}*/
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
			puts("SE DECONECTA UNA CONSOLA");
			closeHandle(socket, master);
			break;
		case 1:
		{
			int tamanioScript = mensaje->header.tamanio + 1;
			char* script = malloc(tamanioScript);
			memcpy(script, mensaje->data, mensaje->header.tamanio);
			script[mensaje->header.tamanio] = '\0';
			PCB* pcb = createProcess(script, tamanioScript);
			newProcess(pcb, socket, script, tamanioScript);
			if(readyProcess() == -1)
				puts("SE ALCANZO EL LIMITE DE MULTIPROGRAMACION - QUEDA EN NEW");
			free(script);
			break;
		}
		//Abortar procesos
		case 3:
		//lSend(socket, mensaje->data, 3, sizeof(int));
		{
			bool mismaConsola(ProcessControl* pc)
			{
				return pc->consola == socket && pc->state != 9;
			}
			t_list* procesosDeLaConsola = list_filter(process, mismaConsola);
			list_iterate(procesosDeLaConsola, matarDesdeProcessControl);
			lSend(socket, mensaje->data, 3, sizeof(int));
			list_destroy(procesosDeLaConsola);
			break;
		}

		//Para cerrar el hilo receptor de la consola
		case 4:
			lSend(socket, mensaje->data, 4, sizeof(int));
			break;

		case 9:
		{
			int pid = *(int*)mensaje->data;
			matarCuandoCorresponda(pid,-7);
			break;
		}
	}

	destruirMensaje(mensaje);

}

void matarDesdeProcessControl(ProcessControl* pc)
{
	matarCuandoCorresponda(pc->pid, -6);
}

void matarCuandoCorresponda(int pid, int exitCode)
{
	ProcessControl* pc = PIDFind(pid);
	if(pc->state == 2)
		pc->toBeKilled = exitCode;
	else
		killProcess(pc->pid,exitCode);
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
	if(!test)lSend(conexionMemoria, buffer, 1, tamanioScriptSerializado);
	if(!test){Mensaje* respuesta = lRecv(conexionMemoria);
		int to= respuesta->header.tipoOperacion;
		destruirMensaje(respuesta);
		free(buffer);
		return to == 104;
	}
	free(buffer);
	return 1;
}

MemoryRequest deserializeMemReq(void* mr){
	MemoryRequest res;
	memcpy(&res.pid,mr,sizeof(int));
	memcpy(&res.size,mr+sizeof(int),sizeof(int));
	return res;
}

void* serializeMemReq(MemoryRequest mr, int idPage, int offset){
	void* res= malloc(sizeof(int)*4+sizeof(HeapMetadata));
	memcpy(res,&mr.pid,sizeof(int));
	memcpy(res+sizeof(int),&idPage,sizeof(int));
	memcpy(res+sizeof(int)*2,&offset,sizeof(int));
	int size = sizeof(HeapMetadata);
	memcpy(res+sizeof(int)*3, &size, sizeof(int));
	HeapMetadata hm;
	hm.isFree=0;
	hm.size= mr.size;
	memcpy(res+sizeof(int)*4,&hm,sizeof(HeapMetadata));
	return res;
}
void mostrarMetadata(HeapMetadata* hm)
{
	printf("ISFREE: %i SIZE:%i\n", hm->isFree, hm->size);
}
int memoryRequest(MemoryRequest mr, int* offset, PageOwnership* po){
	int operation;
	if((operation = grabarPedido(po,mr,offset)) == -1)
		return operation;
	int memreq;
	if((memreq=sendMemoryRequest(mr,po,*offset))!=1)
	{
		list_iterate(po->occSpaces, mostrarMetadata); // debug
		return memreq;
	}
	return operation;
}


int freeMemory(int pid, int page, int offset){
	// IMPLEMENTAR MATAR PAGINAS CUANDO QUEDAN VACIAS
	PageOwnership* po= findPage(pid,page);
	if(po==NULL)return -1;
	int acc=0,i=0;
	HeapMetadata* hm;
	while(acc<offset){
		hm= list_get(po->occSpaces,i);
		acc+=sizeof(HeapMetadata);
		acc+= hm->size;
		i++;
	}
	//hm= list_get(po->occSpaces,i);
	hm->isFree=1;
	sendKillRequest(pid,page,offset-sizeof(HeapMetadata),hm);
	void* memPage = defragging(pid,page,po->occSpaces);
	if(memPage == NULL)
		return 1;
	int sizeMsg = config->PAG_SIZE + sizeof(int)*4;
	void* message = malloc(sizeMsg);
	memcpy(message, &pid, sizeof(int));
	memcpy(message+sizeof(int), &page, sizeof(int));
	int cero = 0;
	memcpy(message+sizeof(int)*2,&cero , sizeof(int));
	memcpy(message+sizeof(int)*3, &config->PAG_SIZE, sizeof(int));
	memcpy(message+sizeof(int)*4, memPage, config->PAG_SIZE);
	lSend(conexionMemoria, message, 2, sizeMsg);
	free(memPage);
	free(message);
	return 1;
}

void sendKillRequest(int pid, int page, int offset, HeapMetadata* hm){
	int size= sizeof(int)*4+sizeof(HeapMetadata);
	int sizeofhm= sizeof(HeapMetadata);
	void* request= malloc(size);
	memcpy(request,&pid,sizeof(int));
	memcpy(request+sizeof(int),&page,sizeof(int));
	memcpy(request+sizeof(int)*2,&offset,sizeof(int));
	memcpy(request+sizeof(int)*3, &(sizeofhm), sizeof(int));
	memcpy(request+sizeof(int)*4,hm,sizeof(HeapMetadata));

	if(!test)lSend(conexionMemoria,request, 2, size);
	free(request);

}

HeapMetadata* initializeHeapMetadata(int size){
	HeapMetadata* hm= malloc(sizeof(HeapMetadata));
	hm->isFree=0;
	hm->size= size;
	return hm;
}

int grabarPedido(PageOwnership* po, MemoryRequest mr, int* offset){

	if(!viableRequest(mr.size)) return -1;
	HeapMetadata* hm= initializeHeapMetadata(mr.size);
	PageOwnership* paginaExistente = pageToStore(mr);//busco la pagina para guardarlo, si no hay -1
	if (paginaExistente == NULL){//el pedido se grabo en pagina nueva
		po->pid = mr.pid;
		if(!test)if(getNewPage(po)==-2)return -2;//pido nueva pagina
		initializePageOwnership(po);//aca queda el PageOwnership con la estructura que marca el espacio libre
		*offset= occupyPageSize(po,hm);
		list_add(ownedPages,po);

		return 1;
	}
	else{
		//
		puts("ANTES");
		list_iterate(paginaExistente->occSpaces, mostrarMetadata);
		puts("------");
		*offset= occupyPageSize(paginaExistente,hm);//guarda el heapMetadata correspondiente en el PageOwnership
		memcpy(po, paginaExistente, sizeof(PageOwnership));
		return 0;
	}

	/*if (po->idpage!=-1){

		return 0;
	}*/


}

int getNewPage(PageOwnership* po){
	lSend(conexionMemoria,&po->pid,3,sizeof(int));
	Mensaje* msg= lRecv(conexionMemoria);
	if(msg->header.tipoOperacion==-2)
	{
		destruirMensaje(msg);
		return -2;
	}
	memcpy(&po->idpage,msg->data,sizeof(int));
	destruirMensaje(msg);
	return 1;
}

int sendMemoryRequest(MemoryRequest mr, PageOwnership* po, int offset){

	void* serializedMSG= serializeMemReq(mr,po->idpage,offset);
	if(!test) lSend(conexionMemoria,serializedMSG,2,sizeof(int)*4+sizeof(HeapMetadata));//envío el pedido de grabacion
	free(serializedMSG);
	return 1;
}

PageOwnership* pageToStore(MemoryRequest mr){
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
	list_destroy(processPages);
	return po;
}

t_list* findProcessPages(int pid){
	bool _PIDFind(PageOwnership* po){
		return po->pid== pid;
	}
	return list_filter(ownedPages,&(_PIDFind));
}


PageOwnership* findPage(int pid, int page){
	t_list* processPages = findProcessPages(pid);
	bool _pageIdFind(PageOwnership* po){
		return po->idpage== page;
	}
	PageOwnership* elemento = list_find(processPages,&(_pageIdFind));
	list_destroy(processPages);
	return elemento;
}

int replacePage(int pid, PageOwnership* po){
	t_list* processPages = findProcessPages(pid);
	if(processPages==NULL)return -1;
	bool _pageIdFind(PageOwnership* po){
		return po->idpage== po->idpage;
	}
	list_remove_and_destroy_by_condition(ownedPages,&(_pageIdFind),&(destroyPageOwnership));
	list_add(processPages, po);
	return 1;
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
	int i,offs;
	for(i=0;i<list_size(po->occSpaces);i++){
		HeapMetadata* hw= list_get(po->occSpaces,i);
		if(hw->isFree==1){
			if(hw->size >= (hm->size+sizeof(HeapMetadata))){
				int aux= hm->size;
				hm->isFree=1;//con el elemento que me pasaron, lo modifico para obtener el que voy a agregar como libre (1)
				hm->size= hw->size-aux-sizeof(HeapMetadata);
				hw->isFree=0;//modifico el elemento de la lista que ya estaba como libre, para que este ocupado con el tamaño requerido
				hw->size= aux;
				list_add_in_index(po->occSpaces,i+1,hm);//agrego la estructura libre (1) al lado
				offs= offset(po->occSpaces,i);
				break;
			}
		}
	}
	return offs;
}

int offset(t_list* heap, int pos){
	int i, offset= 0;
	for(i=0;i<pos;i++){
		offset+= sizeof(HeapMetadata);
		HeapMetadata* hm= list_get(heap,i);
		offset+= hm->size;
	}
	return offset;
}

/*void storeVariable(PageOwnership* po, char* name, int pos){
	HeapControl* hc= malloc(sizeof(HeapControl));
	hc->name=name;
	hc->listPosition= pos;
	list_add(po->control,hc);
}*/

//------------------------------------------------Defragmentación-----------------------------------------------------------//

/*bool fragmented (t_list* page){
	int i,acc=0,cant=0;
	for(i=0; i<list_size(page);i++){
		HeapMetadata* hm= list_get(page,i);
		if(hm->isFree){acc+=hm->size;cant++;}
	}
	return acc>= (config->PAG_SIZE/5) && cant >= 3;
}*/

bool fragmented (t_list* page){
	int i;
	bool hayDos= false;

	for(i=0;i<list_size(page);i++){
		HeapMetadata* hm1= list_get(page,i);
		HeapMetadata* hm2= list_get(page,i+1);
		if(hm1 != NULL && hm2 != NULL && hm1->isFree && hm2->isFree)return true;
	}

	return hayDos;

}

/*void _modifyMemoryPage(int base,int top,int offset,void* memoryPage){
	memcpy(memoryPage+offset,memoryPage+base,top-base);
}*/

void* getMemoryPage(int pid, int idPage){
	int offset=0, size=sizeof(int)*4;
	void* msg= malloc(size);
	memcpy(msg,&pid,sizeof(int));
	memcpy(msg+sizeof(int),&idPage,sizeof(int));
	memcpy(msg+sizeof(int)*2,&offset,sizeof(int));
	memcpy(msg+sizeof(int)*3,&config->PAG_SIZE,sizeof(int));
	if(!test)lSend(conexionMemoria,msg,5,size);
	if(test){//esto esta asi por el test
		return res->data;
	}
	free(msg);
	Mensaje* mes= lRecv(conexionMemoria);
	char* mem = malloc(mes->header.tamanio);
	memcpy(mem, mes->data, mes->header.tamanio);
	destruirMensaje(mes);
	return mem;

}

/*void* defragging(int pid, int idPage, t_list* page){

	if(!fragmented(page)) return 0;

	void* memPage= getMemoryPage(pid,idPage);

	int offset= 0,offsetList=0;
	int cantOcupados= _usedFragments(page);
	int cantMovidos= 0;

	while(cantMovidos < cantOcupados){
		int base=offset,top=offset,baseList,blockSize=0,i,stop=0,inBlock=0,esPrincipio=1;
		for(i=offsetList; i<list_size(page) && stop==0 ;i++){
			HeapMetadata* hm= list_get(page,i);

			if(!hm->isFree){//si no esta libre
				if(!esPrincipio){//si esta ocupada la pagina, y no está al principio, entro a un bloque
					if(!inBlock)baseList=i;//si no estaba en un bloque, este es el i desde donde voy a mover en la lista
					inBlock=1;
					cantMovidos++;//cada vez que sigo adentro de un bloque es porque voy a mover otro elemento
					blockSize++;
				}
				else {//no esta libre y es principio entonces o hay bloques al principio o ya movi y no los tengo que pisar
					cantOcupados--;//como considero que este bloque ya esta al principio, hay un bloque menos para ser movido
					offsetList++;//aca si no esta libre y no estoy en bloque muevo el offset para que al principio si los primeros bloques estan ocupados no los pise
					offset+= (hm->size + sizeof(HeapMetadata));
				}destroyPageOwnership
			}
			else {//si esta libre
				if(inBlock)stop=1;//si esta libre y yo estaba en un bloque, se termino el bloque y tengo que parar
				else if(esPrincipio)esPrincipio=0;
			}

			if(!inBlock)base+= (hm->size + sizeof(HeapMetadata));//si no estoy en un bloque significa que no llegue a donde voy a empezar a mover, tengo que aumentar el puntero base
			if(!stop)top+= (hm->size + sizeof(HeapMetadata));//si no setie el stop es porque todavia no termine de iterar y tengo que seguir aumentando el puntero tope
		}

		_modifyMemoryPage(base,top,offset,memPage);//que me lo mueva al offset, en 0 al principio
		defragPage(page,baseList,blockSize,offsetList);
		offsetList+= blockSize;
		offset+= (top-base);//cambio el offset, lo muevo como tanto espacio grabe para no pisar nada
	}
	calculateFreeSpace(page,_usedFragments(page),memPage);
	return memPage;
}*/

bool isEmpty(t_list* page){
	int i;
	for(i=0;i<list_size(page);i++){
		HeapMetadata* hm = list_get(page,i);
		if(!hm->isFree){
			return 0;
		}
	}
	return 1;
}


void* defragging(int pid, int idPage, t_list* page){

    if(!fragmented(page)) return NULL;

    if(isEmpty(page)){
    	int findIndex(int pid, int idPage){
    		int i;
    		for(i=0;i<list_size(ownedPages);i++){
    			PageOwnership* po= list_get(ownedPages,i);
    			if(po->pid==pid && po->idpage==idPage)break;
    		}
    		return i;
    	}
    	int index= findIndex(pid,idPage);
    	PageOwnership* po= findPage(pid,idPage);
    	freePage(po, index);
    	return NULL;
    }

    HeapMetadata* hm;

    void* memPage= getMemoryPage(pid,idPage);

    int offset= 0,offsetList=0,cant=0,i=0,size;

    size= list_size(page);

    for(i=0;i<(size-1);i++){

        offset= _unusedFragmentsOffset(page,offset,&offsetList,&cant);

        if(offset!= -1){

            hm = malloc(sizeof(HeapMetadata));

            hm->isFree=1;
            hm->size=0;

            for(i=offsetList;i<cant+offsetList;i++){
                HeapMetadata* free= list_get(page,i);
                hm->size+= free->size;
            }

            hm->size+= sizeof(HeapMetadata) * (cant-1);

            list_add_in_index(page,offsetList,hm);

            for(i=offsetList+1;i<cant+offsetList+1;i++){

                list_remove_and_destroy_element(page,i,&free);
                i--;
                cant--;

            }

            /*for(i=0;list_size(page)>i;i++){
            		HeapMetadata* hm43= list_get(page,i);
            		printf("%d HM: is free %d, size %d     -----    ",i,hm43->isFree,hm43->size);
            	}*/

            memcpy(memPage+offset,hm,sizeof(HeapMetadata));

            size= list_size(page);

            i=0; offsetList=0; offset=0;cant=0;

        }

    }


    return memPage;

}

int _unusedFragmentsOffset(t_list* page,int offset,int* offsetList,int* cant){
	int i,block=0;

	for(i=0; i<(list_size(page)-1) && !block ; i++){
		HeapMetadata* hm1= list_get(page,i);
		HeapMetadata* hm2= list_get(page,i+1);
		if(hm1->isFree && hm2->isFree){
			block=1;
			break;
		}
		offset+= (hm1->size+sizeof(HeapMetadata));
		(*offsetList)= (*offsetList)+1;
	}
	if(!block)return -1;
	HeapMetadata* hm1= list_get(page,i);
	while(hm1 != NULL && hm1->isFree && i<list_size(page)){
		*cant= *cant + 1;
		i++;
		hm1= list_get(page,i);
	}

	return offset;
}

/*
void defragPage(t_list* page, int baseList, int blockSize, int offsetList){
	int i;
	for(i= baseList; i < (baseList + blockSize) ; i++){//desde donde esta el primer bloque, hasta el ultimo
		HeapMetadata* hm= list_get(page,i);//agarro el bloque
		list_remove(page,i);//lo saco de donde esta ahora
		list_add_in_index(page,offsetList + (i-baseList),hm);//lo pongo en el offset mas la cantidad que ya haya asignado para no pisar nada
	}
}

void calculateFreeSpace(t_list* page,int cantOcupados, void* memPage){
	int i,acc=0,j=0;
	int c= list_size(page);

	for(i=0;i<c;i++){
		HeapMetadata* hm= list_get(page,j);
		if(!hm->isFree){
			acc+=(hm->size + sizeof(HeapMetadata));
			j++;
		}
		else list_remove_and_destroy_element(page,j,&(free));
	}

	HeapMetadata* hm = malloc(sizeof(HeapMetadata));
	acc+=sizeof(HeapMetadata);
	hm->isFree=1;
	hm->size= config->PAG_SIZE - acc;
	list_add(page,hm);
	memcpy(memPage+acc,hm,sizeof(HeapMetadata));

}

bool _usedFragment(HeapMetadata* hm){
	return hm->isFree==0;
}

int _usedFragments(t_list* page){
	t_list* aux= list_filter(page,&(_usedFragment));
	return list_size(aux);
}
*/



//------------------------------------------------Funciones de CPU----------------------------------------------------------//


void recibirDeCPU(int socket, connHandle* master)
{
	puts("CPU");
	PCB* pcb;
	Mensaje* mensaje = lRecv(socket);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			printf("SE DESCONECTA CPU SOCKET: %i\n", socket);
			closeHandle(socket, master);
			removerCPUDeCola(socket);
			break;
		case 1:
			pcb = recibirPCB(mensaje);
			puts("SE TERMINO LA EJECUCION DE UN PROCESO. SE DEBERIA ENVIAR A COLA FINALIZADO EL PCB");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			cpuReturnsProcessTo(pcb, 9);
			killProcess(pcb->pid,0);
			pushearAColaCPUS(socket);
			executeProcess();
			break;
		case 2:
			pcb = recibirPCB(mensaje);
			puts("VUELVE PCB POR FIN DE Q");
			cpuReturnsProcessTo(pcb,1);
			matarSiCorresponde(pcb->pid);
			pushearAColaCPUS(socket);
			executeProcess();
			break;
		case 3:
			pcb = recibirPCB(mensaje);
			puts("VUELVE PCB POR BLOQUEO");
			cpuReturnsProcessTo(pcb,3);
			matarSiCorresponde(pcb->pid);
			pushearAColaCPUS(socket);
			executeProcess();
			break;
		case 4:
			pcb = recibirPCB(mensaje);
			puts("PROGRAMA ABORTADO POR STACK OVERFLOW");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			cpuReturnsProcessTo(pcb, 9);
			killProcess(pcb->pid, -5);
			pushearAColaCPUS(socket);
			executeProcess();
			break;
		case 5:
			pcb = recibirPCB(mensaje);
			puts("PROGRAMA ABORTADO POR PROBLEMAS EN SYSCALL");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			cpuReturnsProcessTo(pcb, 9);
			matarSiCorresponde(pcb->pid);
			pushearAColaCPUS(socket);
			executeProcess();
			break;
		case 6: // EL CPU DEVUELVE PCB POR FIN DE Q PERO A SU VEZ SE DESCONECTA EL CPU (SIGUSR1)
			pcb = recibirPCB(mensaje);
			puts("VUELVE PCB POR FIN DE Q [SIGUSR1]");
			cpuReturnsProcessTo(pcb,1);
			matarSiCorresponde(pcb->pid);
			executeProcess();
			break;
		case 200:
		{
			//obtener valor variable compartida
			puts("CPU PIDE VALOR DE VARIABLE");
			char* nombre= malloc(mensaje->header.tamanio);
			memcpy(nombre,mensaje->data,mensaje->header.tamanio);
			GlobalVariable * gb= findGlobalVariable(nombre);
			lSend(socket,&gb->value,104,sizeof(int));
			free(nombre);
			break;
		}
		case 201:
		{
			//asignar valor variable compartida
			puts("CPU QUIERE ASIGNAR VARIABLE");
			int len=0;
			memcpy(&len,mensaje->data,sizeof(int));
			char* nombre= malloc(len);
			memcpy(nombre,mensaje->data+sizeof(int),len);
			int newValue= 0;
			memcpy(&newValue,mensaje->data+sizeof(int) + len,sizeof(int));
			GlobalVariable * gb= findGlobalVariable(nombre);
			gb->value= newValue;
			free(nombre);
			break;
		}
		case 204:
		{
			// RESERVAR HEAP
			puts("CPU PIDE HEAP");
			MemoryRequest mr = deserializeMemReq(mensaje->data);
			PageOwnership* po= malloc(sizeof(PageOwnership));
			int* offset = malloc(sizeof(int));
			int res= memoryRequest(mr,offset,po);
			if( res== -1){
				puts("PEDIDO MAYOR QUE EL TAMAÑO DE UNA PAGINA");
				matarCuandoCorresponda(mr.pid,-8);
				lSend(socket, NULL, -2, 0);
				break;
			}
			else if (res == -2){
				puts("NO HAY ESPACIO DEBE FINALIZAR PROCESO");
				matarCuandoCorresponda(mr.pid,-9);
				lSend(socket, NULL, -2, 0);
				break;
			}
			*offset = (*offset)+sizeof(HeapMetadata);
			void* msg = malloc(sizeof(int)*2);
			memcpy(msg,&po->idpage,sizeof(int));
			memcpy(msg+sizeof(int),offset,sizeof(int));

			if(!test)lSend(socket,msg,104,sizeof(int)*2);

			free(msg);
			free(offset);

			break;
		}
		case 205:
		{
			// LIBERAR HEAP
			puts("CPU QUIERE LIBERAR HEAP");
			int pid, page, offset;
			memcpy(&pid,mensaje->data,sizeof(int));
			memcpy(&page,mensaje->data+sizeof(int),sizeof(int));
			memcpy(&offset,mensaje->data+sizeof(int)*2,sizeof(int));
			freeMemory(pid,page,offset);
			//memoryRequest(mr,mensaje->header.tamanio-sizeof(mr),mensaje->data+sizeof(mr)); funcion para eliminar
			break;
		}
		case 202:
		{
			puts("PROCESO UTILIZA WAIT");
			char* sem = malloc(mensaje->header.tamanio+1);
			memcpy(sem, mensaje->data, mensaje->header.tamanio);
			sem[mensaje->header.tamanio] = '\0';
			puts("OBTENER VALOR");
			if(obtenerValorSemaforo(sem) <= 0) {
				//Aviso a CPU que hay bloqueo
				lSend(socket, mensaje->data, 3, sizeof(int));
				//CPU me envia el pid a bloquearse
				Mensaje* m = lRecv(socket);
				int* pidblock = malloc(sizeof(int));
				*pidblock = *(int*)m->data;
				//Envio el pid a la cola del semaforo
				enviarAColaDelSemaforo(sem, pidblock);
				free(sem);
				destruirMensaje(m);
			} else {
				//Aviso a CPU que no hay bloqueo
				lSend(socket, mensaje->data, 0, sizeof(int));
				waitSemaforo(sem);
			}
			break;
		}

		case 203:
		{
			puts("PROCESO UTILIZA SIGNAL");
			char* sem = malloc(mensaje->header.tamanio+1);
			memcpy(sem, mensaje->data, mensaje->header.tamanio);
			sem[mensaje->header.tamanio] = '\0';
			int pos = obtenerPosicionSemaforo(sem);
			if(laColaDelSemaforoEstaVacia(pos))
				signalSemaforo(sem);
			else {
				int pid = quitarDeColaDelSemaforo(sem);
				fromBlockedToReady(pid);
			}
			free(sem);
			break;
		}
		case 206: // ABRIR ARCHIVO
		{

			rutayPermisos rp = deserializarRutaPermisos(mensaje->data);
			int fd = abrirArchivo(rp.pid,rp.ruta, rp.permisos);
			if(fd != -1)
				lSend(socket, &fd, 104, sizeof(int));
			else
			{
				puts("EL PROGRAMA QUISO ABRIR UN ARCHIVO QUE NO EXISTE SIN FLAG 'C'");
				lSend(socket, NULL, -3, 0);
				matarCuandoCorresponda(rp.pid, -2);
			}
			break;
		}

		case 207: // BORRAR ARCHIVO
		{
			int fd;
			int pid;
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&fd, mensaje->data+sizeof(int), sizeof(int));
			int estado = borrarArchivo(pid,fd);
			if(estado == -1)
			{
				puts("NO SE PUEDE BORRAR, HAY OTROS PROCESOS USANDOLO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				matarCuandoCorresponda(pid, -20);
				lSend(socket, NULL, -3, 0);
			}
			else if(estado == 0)
			{
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3, 0);
				matarCuandoCorresponda(pid, -2);
			}
			else
				lSend(socket,NULL, 104, 0);
			break;
		}

		case 208: // CERRAR ARCHIVO
		{
			int fd;
			int pid;
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&fd, mensaje->data+sizeof(int), sizeof(int));
			if(!cerrarArchivo(pid, fd))
			{
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3, 0);
				matarCuandoCorresponda(pid, -2);
			}
			else
				lSend(socket, NULL, 104, 0);
			break;
		}

		case 209: // MOVER CURSOR
		{
			fileInfo info;
			memcpy(&info, mensaje->data, sizeof(fileInfo));
			if(!moverCursorArchivo(info))
			{
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3, 0);
				matarCuandoCorresponda(info.pid, -2);
			}
			else
				lSend(socket, NULL, 104, 0);
			break;
		}

		case 210: // ESCRIBIR ARCHIVO
		{

			fileInfo info;
			char* data = deserializarPedidoEscritura(mensaje->data,&info);
			printf("[ESCRITURA]: PID: %i | FD: %i | TAMANIO: %i\n", info.pid, info.fd, info.tamanio);
			int estado = escribirArchivo(info, data);
			if(estado == -1)
			{
				puts("CPU NO TIENE PERMISO DE ESCRITURA DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3 ,0);
				matarCuandoCorresponda(info.pid, -4);

			}
			else if(estado == 0)
			{
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3 ,0);
				matarCuandoCorresponda(info.pid, -2);
			}
			else
				lSend(socket, NULL, 104, 0);
			free(data);
			break;
		}

		case 211: // LEER ARCHIVO
		{
			puts("CPU QUIERE LEER");
			fileInfo info;
			memcpy(&info, mensaje->data, sizeof(fileInfo));
			char* data = leerArchivo(info);
			if(data == NULL)
			{
				puts("CPU NO PUEDE LEER, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3 ,0);
				matarCuandoCorresponda(info.pid, -3);
			}
			else if(data == -1)
			{
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
				lSend(socket, NULL, -3 ,0);
				matarCuandoCorresponda(info.pid, -2);
			}
			else
			{
				lSend(socket, data, 104, info.tamanio);
				free(data);
			}
			break;
		}
	}
	destruirMensaje(mensaje);

}

void removerCPUDeCola(int cpu)
{
	bool mismoSocket(int unSocket)
	{
		return unSocket == cpu;
	}
	list_remove_by_condition(colaCPUS->elements, mismoSocket);
}

void pushearAColaCPUS(int cpu)
{
	pthread_mutex_lock(&mColaCPUS);
	queue_push(colaCPUS, cpu);
	pthread_mutex_unlock(&mColaCPUS);

}


char* deserializarPedidoEscritura(char* serializado, fileInfo* info)
{

	memcpy(info, serializado, sizeof(fileInfo));
	char* data = malloc(info->tamanio);
	memcpy(data, serializado+sizeof(fileInfo), info->tamanio);
	return data;
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
	if(!strcmp(config->ALGORITMO, "RR")){algoritmo= config->QUANTUM;}
	char* data = malloc(sizeof(int)*3);
	memcpy(data, &config->STACK_SIZE, sizeof(int));
	memcpy(data + sizeof(int), &config->PAG_SIZE, sizeof(int));
	memcpy(data + sizeof(int)*2, &algoritmo, sizeof(int));
	lSend(CPU, data, 10, sizeof(int)*3);
	free(data);
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
	pthread_mutex_lock(&mMultiprog);
	int res = config->GRADO_MULTIPROG > currentMultiprog;
	pthread_mutex_unlock(&mMultiprog);
	return res;
}


bool isListener(int p, connHandle master){
	return ( p==master.listenCPU || p== master.listenConsola);// || p == master.inotify);
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

GlobalVariable* findGlobalVariable(char* nombre){
	bool isNamed(GlobalVariable* gb){
		return !strcmp(gb->name,nombre);
	}
	return list_find(globalVariables,&isNamed);
}

//---------------------------------------------------Para semaforos----------------------------------------------------//

//Obtiene la posicion en SEM_IDS a partir del nombre del semaforo
int obtenerPosicionSemaforo(char* c) {
	int i;
	for(i=0; !sonIguales(c, config->SEM_IDS[i]); i++);
	return i;
}

int obtenerValorSemaforo(char* c) {
	int pos = obtenerPosicionSemaforo(c);
	char* valor = config->SEM_INIT[pos];
	//Convierto el string a int
	return atoi(valor);
}

void enviarAColaDelSemaforo(char* c, int* pid) {
	int pos = obtenerPosicionSemaforo(c);
	t_queue* colaDelSemaforo = (t_queue*)list_get(listaDeColasSemaforos, pos);
	queue_push(colaDelSemaforo, pid);
	printf("EL SEMAFORO %s BLOQUEO EL PROCESO %i\n",config->SEM_IDS[pos], *pid);
}

int quitarDeColaDelSemaforo(char* c) {
	int pos = obtenerPosicionSemaforo(c);
	//int* pid = queue_peek(list_get(listaDeColasSemaforos, pos));
	int* punteroPid = queue_pop(list_get(listaDeColasSemaforos, pos));
	int pid = *punteroPid;
	printf("EL SEMAFORO %s DESBLOQUEO EL PROCESO %i\n",config->SEM_IDS[pos], pid);
	free(punteroPid);
	return pid;
}

//Para aumentar o disminuir el valor del semaforo
void operarSemaforo(char* c, int num) {
	int pos = obtenerPosicionSemaforo(c);
	char* valor = config->SEM_INIT[pos];
	char* semaforo = config->SEM_IDS[pos];
	//Como en SEM_INIT los valores son string lo convierto int y le sumo o resto 1
	int nuevoValor = atoi(valor)+num ;
	//Lo guardo en su posicion pero antes lo vuelvo a convertir a string que trucazo no
	config->SEM_INIT[pos] = string_itoa(nuevoValor);
	printf("SEMAFORO %s CAMBIO SU VALOR A: %s\n",semaforo, string_itoa(nuevoValor));
}

void waitSemaforo(char* c) {
	operarSemaforo(c, -1);
}

void signalSemaforo(char* c) {
	operarSemaforo(c, 1);
}

//Me dice cuantos elementos hay en SEM_INIT
int cantidadSemaforos() {
	int i = 0;
	while(config->SEM_INIT[i] != NULL)
		i++;
	return i;
}

//Inicio la lista y las colas
void crearListaDeColasSemaforos() {
	listaDeColasSemaforos = list_create();
	int i;
	for (i = 0; i < cantidadSemaforos(); i++) {
		//Cada semaforo tiene su cola de bloqueados
		t_queue* cola = queue_create();
		list_add(listaDeColasSemaforos, cola);
	}
}

int laColaDelSemaforoEstaVacia(int posicionSemaforo) {
	return queue_is_empty(list_get(listaDeColasSemaforos, posicionSemaforo));
}

//Funciones locas basicamente son para buscar un pid en listaDeColasSemaforos y removerlo, en caso que se mate un proceso bloqueado

//Busca en la cola del semaforo el pid
int* buscarEnCola(int pos, int pid) {
	bool buscarPorPID(void* pidProceso){
		return (*(int*)pidProceso)==pid;
	}
	int* p = list_find(   ((t_queue*)list_get(listaDeColasSemaforos, pos))->elements  , buscarPorPID);
	return p;
}

//Busca en la lista de semaforos la cola del semaforo donde se encuentra el pid
t_queue* buscarColaContenedora(int pid) {
	int i;
	for(i=0; i<list_size(listaDeColasSemaforos) && buscarEnCola(i, pid) == NULL; i++);
	if(i==list_size(listaDeColasSemaforos)) {
		return NULL; //No encontro la cola
	}
	else {
		//Encontro la cola
		return list_get(listaDeColasSemaforos,i);
	}
}

void quitarDeColaDelSemaforoPorKill(int pid) {
	bool buscarPorPID(void* pidProceso){
		return (*(int*)pidProceso)==pid;
	}
	t_queue* cola  = buscarColaContenedora(pid);
	if(cola != NULL) {
		printf("QUITANDO EL PROCESO% i DE LA COLA DEL SEMAFORO POR KILL\n", *(int*)(list_remove_by_condition(cola->elements, buscarPorPID)));
	}
}

//La expresividad no se mancha

int sonIguales(char* s1, char* s2) {
	if (strcmp(s1, s2) == 0)
		return 1;
	else
		return 0;
}

void inotifyStuff(int p)
{
	char buffer[BUF_LEN];
		puts("SE MODIFICO EL ARCHIVO DE CONFIGURACION");
		int length = read(p, buffer, BUF_LEN);
		int offset = 0;
		// Luego del read buffer es un array de n posiciones donde cada posición contiene
		// un eventos ( inotify_event ) junto con el nombre de este.
		while (offset < length) {
			// El buffer es de tipo array de char, o array de bytes. Esto es porque como los
			// nombres pueden tener nombres mas cortos que 24 caracteres el tamaño va a ser menor
			// a sizeof( struct inotify_event ) + 24.
			struct inotify_event *event = (struct inotify_event *) &buffer[offset];
				// El campo "len" nos indica la longitud del tamaño del nombre
			if (event->len) {
				// Dentro de "mask" tenemos el evento que ocurrio y sobre donde ocurrio
				// sea un archivo o un directorio
				printf("AYY %s\n", event->name);
				if (event->mask & IN_CLOSE_WRITE)
				{
					if (event->mask & IN_ISDIR)
					{
						printf("The directory %s was modified.\n", event->name);
					}
					else
					{
						if(strcmp(event->name, "config.conf"))
							break;
						printf("The file %s was modified.\n", event->name);
						t_config* conf = config_create(RUTA);
						puts("A");
						config->QUANTUM_SLEEP = config_get_int_value(conf, "QUANTUM_SLEEP");
						puts("B");
						printf("NUEVO QUANTUM SLEEP: %i\n", config->QUANTUM_SLEEP);
						config_destroy(conf);
					}
				}
			}
			offset += sizeof (struct inotify_event) + event->len;
		}
/*		inotify_rm_watch(file_descriptor, watch_descriptor);
		close(file_descriptor);*/
}
