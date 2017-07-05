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
			killProcess(*(int*)mensaje->data);
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

void* serializeMemReq(MemoryRequest mr, int idPage, int offset, int msgSize, void* msg){
	void* res= malloc(sizeof(int)*5+msgSize);
	memcpy(res,&mr.pid,sizeof(int));
	memcpy(res+sizeof(int),&mr.size,sizeof(int));
	memcpy(res+sizeof(int)*2,&idPage,sizeof(int));
	memcpy(res+sizeof(int)*3,&offset,sizeof(int));
	memcpy(res+sizeof(int)*4,msg,msgSize);
	return res;
}

int memoryRequest(MemoryRequest mr, int size, void* contenido){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	HeapMetadata* hm= initializeHeapMetadata(mr.size);
	int* offset= malloc(sizeof(int));
	if(!test) res= lRecv(conexionMemoria);
	if(res->header.tipoOperacion==-1) return -1;
	int operation= grabarPedido(po,mr,hm,offset);
	//if(sendMemoryRequest(mr,size,contenido,po)==-1) return -1;
	free(res);
	return operation;
}

HeapMetadata* initializeHeapMetadata(int size){
	HeapMetadata* hm= malloc(sizeof(HeapMetadata));
	hm->isFree=0;
	hm->size= size;
	return hm;
}

int grabarPedido(PageOwnership* po, MemoryRequest mr, HeapMetadata* hm, int* offset){

	if (po->idpage==-1){//el pedido se grabo en pagina nueva
		memcpy(po,res->data,sizeof(int));
		initializePageOwnership(po);//aca queda el PageOwnership con la estructura que marca el espacio libre
	}
	*offset= occupyPageSize(po,hm);//guarda el heapMetadata correspondiente en el PageOwnership
	//storeVariable(po,mr.variable,pos);//guarda la estructura de control de variables

	if (po->idpage!=-1){
		list_replace(ownedPages, &PIDFindPO, po);
		return 0;
	}

	list_add(ownedPages,po);
	return 1;

}

int sendMemoryRequest(MemoryRequest mr, int size, void* msg, PageOwnership* po, int offset){
	if(!viableRequest(mr.size)) return -1;
	po->pid= mr.pid;
	po->idpage= pageToStore(mr);//busco la pagina para guardarlo, si no hay -1
	void* serializedMSG= serializeMemReq(mr,po->idpage,offset,size,msg);
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
	int i,offs;
	for(i=0;i<list_size(po->occSpaces);i++){
		HeapMetadata* hw= list_get(po->occSpaces,i);
		if(hw->isFree==1){
			if(hw->size >= (hm->size+sizeof(HeapMetadata))){
				hw->size-=(hm->size+sizeof(HeapMetadata));
				list_replace(po->occSpaces,i,hm);//reemplazo el elemento libre por uno ocupado
				list_add_in_index(po->occSpaces,i+1,hw);//agrego la estructura libre al lado
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
			pcb = recibirPCB(mensaje);
			puts("SE TERMINO LA EJECUCION DE UN PROCESO. SE DEBERIA ENVIAR A COLA FINALIZADO EL PCB");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			pcb->exitCode = 0;
			// HAY QUE ENVIAR EL PCB RECIBIDO A LA COLA FINISHED, EN ESTE MOMENTO NO SE HACE, SE PASA EL PCB VIEJO QUE ESTA EN LA COLA EXECUTED.
			killProcess(pcb->pid);
			queue_push(colaCPUS, socket);
			executeProcess();
			free(pcb);
			break;
		case 2:
			pcb = recibirPCB(mensaje);
			puts("TERMINA EJECUCION DE PROCESO PERO ESTE NO ESTA FINALIZADO");
			cpuReturnsProcessTo(pcb,1);
			executeProcess();
			break;
		case 3:
			pcb = recibirPCB(mensaje);
			puts("TERMINA EJECUCION DE PROCESO Y ESTE VA A BLOCKED");
			cpuReturnsProcessTo(pcb,3);
			queue_push(colaCPUS, socket);
			executeProcess();
			break;
		case 4:
			pcb = recibirPCB(mensaje);
			puts("PROGRAMA ABORTADO");
			mostrarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
			killProcess(pcb->pid);
			executeProcess();
			queue_push(colaCPUS, socket);
			free(pcb);
			break;
		case 204:
			pcb = recibirPCB(mensaje);
			puts("PROCESO PIDE MEMORIA");
			MemoryRequest mr= deserializeMemReq(mensaje->data);
			memoryRequest(mr,mensaje->header.tamanio-sizeof(mr),mensaje->data+sizeof(mr));
			break;
		case 202:
		{
			puts("PROCESO UTILIZA WAIT");
			char* sem = malloc(mensaje->header.tamanio+1);
			memcpy(sem, mensaje->data, mensaje->header.tamanio);
			sem[mensaje->header.tamanio] = '\0';
			if(obtenerValorSemaforo(sem) <= 0) {
				//Aviso a CPU que hay bloqueo
				lSend(socket, mensaje->data, 3, sizeof(int));
				//CPU me envia el pid a bloquearse
				Mensaje* m = lRecv(socket);
				int* pidblock = malloc(sizeof(int));
				*pidblock = *(int*)m->data;
				//Envio el pid a la cola del semaforo
				enviarAColaDelSemaforo(sem, pidblock);
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
				int pid = *quitarDeColaDelSemaforo(sem);
				fromBlockedToReady(pid);
			}
			break;
		}

		case 206: // ABRIR ARCHIVO
		{
			// FALTA CREAR ARCHIVO SI NO EXISTE
			int pid;
			char* ruta;
			char* permisos;
			deserializarInfoArchivo(mensaje->data, &pid, ruta, permisos);
			int fd = abrirArchivo(pid,ruta, permisos);
			if(fd != -1)
				lSend(socket, &fd, 104, sizeof(int));
			else
				lSend(socket, NULL, -3,0);
			break;
		}

		case 207: // BORRAR ARCHIVO
		{
			int fd;
			int pid;
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&fd, mensaje->data+sizeof(int), sizeof(int));
			if(!borrarArchivo(pid,fd))
				puts("NO SE PUEDE BORRAR, HAY OTROS PROCESOS USANDOLO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
			break;
		}

		case 208: // CERRAR ARCHIVO
		{
			int fd;
			int pid;
			memcpy(&pid, mensaje->data, sizeof(int));
			memcpy(&fd, mensaje->data+sizeof(int), sizeof(int));
			if(!cerrarArchivo(pid, fd))
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
			break;
		}

		case 209: // MOVER CURSOR
		{
			fileInfo info;
			memcpy(&info, mensaje->data, sizeof(fileInfo));
			if(!moverCursorArchivo(info))
				puts("CPU ENVIO FD SIN SENTIDO, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
			break;
		}

		case 210: // ESCRIBIR ARCHIVO
		{
			char* data;
			fileInfo info;
			deserializarPedidoEscritura(mensaje->data, data,&info);
			if(!escribirArchivo(info, data))
				puts("CPU NO PUEDE ESCRIBIR DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
			free(data);
			break;
		}

		case 211: // LEER ARCHIVO
		{
			fileInfo info;
			memcpy(&info, mensaje->data, sizeof(fileInfo));
			char* data = leerArchivo(info);
			if(data == NULL)
				puts("CPU NO PUEDE LEER, DEBE MORIR EL CPU, ENVIAR AVISO A CPU");
			else
				lSend(socket, data, 104, info.tamanio);
			break;
		}
	}
	destruirMensaje(mensaje);
}

void deserializarPedidoEscritura(char* serializado, char** data, fileInfo* info)
{
	memcpy(info, serializado, sizeof(fileInfo));
	*data = malloc(info->tamanio);
	memcpy(*data, serializado+sizeof(fileInfo), info->tamanio);
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
	queue_push((t_queue*)list_get(listaDeColasSemaforos, pos), pid);
	printf("EL SEMAFORO %s BLOQUEO EL PROCESO %i\n",config->SEM_IDS[pos], *pid);
}

int* quitarDeColaDelSemaforo(char* c) {
	int pos = obtenerPosicionSemaforo(c);
	int* pid = queue_peek(list_get(listaDeColasSemaforos, pos));
	queue_pop(list_get(listaDeColasSemaforos, pos));
	printf("EL SEMAFORO %s DESBLOQUEO EL PROCESO %i\n",config->SEM_IDS[pos], *pid);
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
	return strlen((char*)config->SEM_INIT)/ sizeof(char*);
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

