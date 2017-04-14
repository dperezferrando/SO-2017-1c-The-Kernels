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
	lSend(socket, idProceso, sizeof(idPropia));
	free(idProceso);
	int* confirmacion = lRecv(socket);
	int conf = confirmacion != NULL && (*confirmacion) != 0;
	free(confirmacion);
	return  conf;

}

int recibirHandShake(int socket, int idEsperada) // bool
{
	int* idProceso = (int*)lRecv(socket);
	int* confirmacion = malloc(sizeof(int));
	if(idProceso == NULL)
		return 0;
	int id = (*idProceso);
	free(idProceso);
	*confirmacion = id == idEsperada;
	lSend(socket, confirmacion, sizeof(confirmacion));
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

void* lRecv(int reciever){
	Header* header=malloc(sizeof(Header));
	header->tamanio=0;
	int status= recieveHeader(reciever,header);
	if(status != 0)
	{
		void* buf=malloc(header->tamanio);
		status= internalRecv(reciever,buf,header->tamanio);
		free(header);
		return buf;
	}
	else
	{
		free(header);
		return NULL;
	}
}

void closeConnection(int s,socketHandler* master){
	close(s);
	FD_CLR(s,master->readSockets);
}

int recieveHeader(int socket, Header* header){
	int status= internalRecv(socket,header,sizeof(Header));
	puts("Header recibido\n");
	return status;
	//Header header= *((Header*)buf);
	//printf("tamanio header: %d\n",header->tamanio);
}

void lSend(int sender, const void* msg, int len){
	_sendHeader(sender,len);//tipo de proceso hardcodeado, hay que ver de donde pija se saca
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

int isWriting(int writer, socketHandler handler){
	return FD_ISSET(writer,handler.writeSockets);
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


