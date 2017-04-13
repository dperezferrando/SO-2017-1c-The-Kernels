#include "SocketLibrary.h"


void handleData(char*);
/*int getSocket(char* ip, char* port){
	return internalSocket(ip,port,*empty);
}
//Está para ser habilitado solo en caso de que por alguna razón uno necesite un socket que no bindea o conecta
void empty(int a, sockAddr b, int c){}*/


//-----------------------------------------MAIN FUNCTIONS-------------------------------------------------------------------//

int enviarHandShake(int socket, int id)
{
	int* idProceso = malloc(sizeof(int));
	*idProceso = id;
	lSend(socket, 0, idProceso, sizeof(id));
	free(idProceso);
	int i;
	return (int)lRecv(socket, &i);


}

int recibirHandShake(int socket)
{
	int op;

	int* idProceso = (int*)lRecv(socket, &op);
	if(idProceso == NULL)
		return -1;
	int id = (*idProceso);
	free(idProceso);
	if(op == 0)
		return id;
	else
		return -1;
}



int getBindedSocket(char* ip, char* port){
	int(*action)(int,const struct sockaddr*,socklen_t)=&bind;
	return internalSocket(ip,port,action);
}

int getConnectedSocket(char* ip, char* port){
	int(*action)(int,const struct sockaddr*,socklen_t)=&connect;
	return internalSocket(ip,port,action);
}

void lListen(int socket,int backlog){//hay que cambiarle el nombre
	errorIfEqual(listen(socket,backlog),-1,"Listen");//cantidad de conexiones que acepta, osea de sockets que voy a manejar
}

int lAccept(int sockListener){
	int newSocket,size=sizeof(struct sockaddr);
	struct sockaddr_storage* addr;
	errorIfEqual(newSocket= accept(sockListener,&addr,&size),-1,"Accept");
	if(newSocket<0)errorIfEqual(0,0,"Accept");
	return newSocket;
}

void* lRecv(int reciever, int* tipoOperacion){
	Header* header=malloc(sizeof(Header));
	header->tamanio=0;
	header->tipoOperacion=0;
	recieveHeader(reciever,header);
	void* buf=malloc(header->tamanio);
	int status= internalRecv(reciever,buf,header->tamanio);
	*tipoOperacion= header->tipoOperacion;
	free(header);
	if(status!=0)return buf;else return NULL;
}

void closeConnection(int s,socketHandler* master){
	close(s);
	FD_CLR(s,master->readSockets);
}

void recieveHeader(int socket, Header* header){
	internalRecv(socket,header,sizeof(Header));
	puts("Header recibido\n");
	//Header header= *((Header*)buf);
	//printf("tamanio header: %d\n",header->tamanio);
}

void lSend(int sender, int tipoOperacion, const void* msg, int len){
	_sendHeader(sender,tipoOperacion, len);//tipo de proceso hardcodeado, hay que ver de donde pija se saca
	internalSend(sender,msg,len);
}

void handleData(char* data){
	printf("La data es: %s\n",data);
	//getchar();
}

socketHandler lSelect(socketHandler handler, int duration){
	timeVal time= _setTimeVal(duration,0);
	socketHandler result= copySocketHandler(handler);
	int status= select(result.nfds,result.readSockets,result.writeSockets,NULL,&time);
	errorIfEqual(status,-1,"select");
	return result;
}

//---------------------------------------------------------------------------------------------//

void addReadSocket(int reader,socketHandler* handler){
	if((handler->nfds-1) < reader) (handler->nfds) = reader+1;
	FD_SET(reader,handler->readSockets);
}

void addWriteSocket(int writer, socketHandler* handler){
	if(handler->nfds-1<writer)handler->nfds=writer+1;
	FD_SET(writer,handler->writeSockets);
}

void rmvReadSocket(int reader, socketHandler* handler){
	FD_CLR(reader,handler->readSockets);
}

void rmvWriteSocket(int writer, socketHandler* handler){
	FD_CLR(writer,handler->writeSockets);
}

void clrReaders(socketHandler* handler){
	FD_ZERO(handler->readSockets);
}

void clrWriters(socketHandler* handler){
	FD_ZERO(handler->writeSockets);
}

void clrHandler(socketHandler* handler){
	clrReaders(handler);
	clrWriters(handler);
	handler->nfds=0;
}

int isReading(int reader, socketHandler handler){
	return FD_ISSET(reader,handler.readSockets);
}

int isWriting(int writer, socketHandler* handler){
	return FD_ISSET(writer,handler->writeSockets);
}

socketHandler initializeSocketHandler(){
	socketHandler b;
	b.nfds=0;
	b.readSockets=malloc(250*sizeof(fd_set));
	b.writeSockets=malloc(250*sizeof(fd_set));
	return b;
}


socketHandler copySocketHandler(socketHandler handler){
	socketHandler result=initializeSocketHandler();
	*(result.readSockets)= *(handler.readSockets);
	*(result.writeSockets)= *(handler.writeSockets);
	result.nfds=handler.nfds;
	return result;
}


