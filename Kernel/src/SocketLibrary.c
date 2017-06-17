#include "SocketLibrary.h"


void handleData(char*);
/*int getSocket(char* ip, char* port){
	return internalSocket(ip,port,*empty);
}
//Está para ser habilitado solo en caso de que por alguna razón uno necesite un socket que no bindea o conecta
void empty(int a, sockAddr b, int c){}*/



//-----------------------------------------MAIN FUNCTIONS-------------------------------------------------------------------//

int enviarHandShake(int socket, int idPropia)
{
	int* idProceso = malloc(sizeof(int));
	*idProceso = idPropia;
	lSend(socket, idProceso, HANDSHAKE,sizeof(int));
	free(idProceso);
	Mensaje* confirmacion = lRecv(socket);
	//int conf = confirmacion != NULL && (*confirmacion) != 0;
	int conf = confirmacion->header.tipoOperacion != -1 && *((int*)confirmacion->data) != 0;
	destruirMensaje(confirmacion);
	return  conf;

}

int recibirHandShake(int socket, int idEsperada) // bool
{
	Mensaje* handshake = lRecv(socket);
	int* confirmacion = malloc(sizeof(int));
	if(handshake->header.tipoOperacion != 0)
		return 0;
	int id = (*(int*)handshake->data);
	destruirMensaje(handshake);
	*confirmacion = id == idEsperada;
	lSend(socket, confirmacion, HANDSHAKE,sizeof(int));
	int conf = *confirmacion;
	free(confirmacion);
	return conf;

}



int getBindedSocket(char* ip, char* port){
	int(*action)(int,const struct sockaddr*,socklen_t)=&bind;
	return internalSocket(ip,port,action);

}

int getConnectedSocket(char* ip, char* port, int idPropia){
	int(*action)(int,const struct sockaddr*,socklen_t)=&connect;
	int socket = internalSocket(ip,port,action);
	if(!enviarHandShake(socket, idPropia))
		errorIfEqual(0,0,"El servidor no admite conexiones para este proceso");
	return socket;

}

void lListen(int socket,int backlog){//hay que cambiarle el nombre
	errorIfEqual(listen(socket,backlog),-1,"Listen");//cantidad de conexiones que acepta, osea de sockets que voy a manejar
}

int lAccept(int sockListener, int idEsperada){
	int newSocket,size=sizeof(struct sockaddr);
	struct sockaddr_storage* addr;
	errorIfEqual(newSocket= accept(sockListener,&addr,&size),-1,"Accept");
	if(!recibirHandShake(newSocket, idEsperada))
		newSocket = -1;
	if(newSocket<0)errorIfEqual(0,0,"Accept - Proceso equivocado u otro error");
	return newSocket;
}

Mensaje* lRecv(int receiver)
{
	Mensaje* mensaje = malloc(sizeof(Mensaje));
	int status = internalRecv(receiver, &mensaje->header, sizeof(Header));
	if(status == 0)
	{
		mensaje->header = _createHeader(-1, -1);
		mensaje->data = NULL;
		return mensaje;
		// ARREGLAR HEADER SIN PUNTERO
	}
	int tamanioData = mensaje->header.tamanio;
	if(tamanioData != 0)
	{
		mensaje->data = malloc(tamanioData);
		internalRecv(receiver, mensaje->data, tamanioData);
	}
	return mensaje;

}



void closeConnection(int s,socketHandler* master){
	close(s);
	FD_CLR(s,&(master->readSockets));
	FD_CLR(s,&(master->writeSockets));
	int i = 0, max = 0;
	for(i = 0;i<master->nfds;i++)
	{
		if(FD_ISSET(i, &(master->readSockets)) || FD_ISSET(i, &(master->writeSockets)))
			max =i;
	}
	master->nfds = max+1;

}

int recieveHeader(int socket, Header* header){
	int status= internalRecv(socket,header,sizeof(Header));
//	puts("Header recibido\n");
	return status;
	//Header header= *((Header*)buf);
	//printf("tamanio header: %d\n",header->tamanio);
}

void lSend(int sender, void* msg, int tipoOperacion, int size){
	//_sendHeader(sender,len);
	int tamanioTotal = sizeof(Header) + size;
	Header header = _createHeader(size, tipoOperacion);
	void* buffer = malloc(tamanioTotal);
	memcpy(buffer, &header, sizeof(Header));
	if(msg != NULL)
		memcpy(buffer + sizeof(Header), msg, size);
	internalSend(sender,buffer,tamanioTotal);
	free(buffer);
}

void destruirMensaje(Mensaje* mensaje)
{
	free(mensaje->data);
	free(mensaje);
}

void handleData(char* data){
	printf("La data es: %s\n",data);
	//getchar();
}

socketHandler lSelect(socketHandler handler, int duration){
	timeVal time= _setTimeVal(duration,0);
	socketHandler result= copySocketHandler(handler);
	int status= select(result.nfds,&result.readSockets,&result.writeSockets,NULL,&time);
	//printf("STATUS: %i\n", status);
	errorIfEqual(status,-1,"select");
	return result;
}

//---------------------------------------------------------------------------------------------//

void addReadSocket(int reader,socketHandler* handler){
	if((handler->nfds-1) < reader)
		handler->nfds = reader+1;
	FD_SET(reader,&(handler->readSockets));
}

void addWriteSocket(int writer, socketHandler* handler){
	if(handler->nfds-1<writer)handler->nfds=writer+1;
	FD_SET(writer,&(handler->writeSockets));
}

void rmvReadSocket(int reader, socketHandler* handler){
	FD_CLR(reader,&(handler->readSockets));
}

void rmvWriteSocket(int writer, socketHandler* handler){
	FD_CLR(writer,&(handler->writeSockets));
}

void clrReaders(socketHandler* handler){
	FD_ZERO(&(handler->readSockets));
}

void clrWriters(socketHandler* handler){
	FD_ZERO(&(handler->writeSockets));
}

void clrHandler(socketHandler* handler){
	clrReaders(handler);
	clrWriters(handler);
	handler->nfds=0;
}

int isReading(int reader, socketHandler handler){
	return FD_ISSET(reader,&(handler.readSockets));
}

int isWriting(int writer, socketHandler handler){
	return FD_ISSET(writer,&(handler.writeSockets));
}

socketHandler initializeSocketHandler(){
	socketHandler b;
	b.nfds=0;
	FD_ZERO(&b.readSockets);
	FD_ZERO(&b.writeSockets);
	return b;
}

/*void destroySocketHandler(socketHandler handler)
{
	free(handler.readSockets);
	free(handler.writeSockets);
}*/


socketHandler copySocketHandler(socketHandler handler){
	socketHandler result=initializeSocketHandler();
	result.readSockets= handler.readSockets;
	result.writeSockets= handler.writeSockets;
	result.nfds=handler.nfds;
	return result;
}

// PRIVADAS

int optval= 1;

addrInfo* getaddrinfocall(char* ip, char* port) {

	addrInfo hints;
	addrInfo *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family= AF_UNSPEC;
	hints.ai_socktype= SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;
	errorIfNotEqual(getaddrinfo(ip,port,&hints,&servinfo),0,"getaddrinfo");
	return servinfo;
}

int internalSocket(char* ip, char* port,int (*action)(int,const struct sockaddr *,socklen_t)){
	addrInfo* addr= getaddrinfocall(ip,port);
	int s= _getFirstSocket(addr,action);
	errorIfEqual(s,NULL,"socket");
	freeaddrinfo(addr);
	return s;
}

int _getFirstSocket(addrInfo* addr, int (*action)(int,const struct sockaddr *,socklen_t)){
	int s=NULL;
	addrInfo* p;

	for	(p=addr; (p != NULL) ; p= p->ai_next){

		if((s=socket(p->ai_family,p->ai_socktype,p->ai_protocol))<0)continue;

		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int));

		if(action(s,p->ai_addr,p->ai_addrlen)==-1){close(s);continue;};

		break;
	}

	if(p==NULL){
		fprintf(stderr, "selectserver: failed to bind or connect");
		close(s);
		exit(2);
	}

	return s;
}

int internalRecv(int reciever, void* buf, int size){
	int status;
	errorIfEqual(status=recv(reciever,buf,size,MSG_WAITALL),-1,"recv");
	return status;
}

void internalSend(int s, void* msg, int len){
	errorIfEqual(send(s,msg,len,0),-1,"Send");//flags harcodeado en 0 pero se puede agregar de ser necesario
}

void _errorIf(int (*criteria)(int,int), int value, int test, char* toPrint){
	if(criteria(value,test)) {
			fprintf(stderr,"%s error: %s\n", toPrint, gai_strerror(value));
			exit(1);
		}
}

void errorIfEqual(int value, int test, char* toPrint){
	int (*equals)(int,int)= &_isEqual;
	_errorIf(equals,value,test,toPrint);
}

void errorIfNotEqual(int value, int test, char* toPrint){
	int (*notEquals)(int,int)= &_isNotEqual;
	_errorIf(notEquals,value,test,toPrint);
}

int _isEqual(int a, int b){
	return a==b;
}

int _isNotEqual(int a, int b){
	return a!=b;
}

/*void _sendHeader(int sender,int len){
	Header header= _createHeader(len);
	internalSend(sender,&header,sizeof(Header));
}*/

Header _createHeader(int size, int tipoOperacion){
	Header header;
	header.tamanio= size;
	header.tipoOperacion=tipoOperacion;
	return header;
}


timeVal _setTimeVal(int seconds, int microseconds){
	timeVal time;
	time.tv_sec= seconds;
	time.tv_usec= microseconds;
	return time;
}

