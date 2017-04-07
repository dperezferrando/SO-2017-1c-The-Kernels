#include "SocketLibrary.h"


void handleData(char*);
/*int getSocket(char* ip, char* port){
	return internalSocket(ip,port,*empty);
}
//Está para ser habilitado solo en caso de que por alguna razón uno necesite un socket que no bindea o conecta
void empty(int a, sockAddr b, int c){}*/


//-----------------------------------------MAIN FUNCTIONS-------------------------------------------------------------------//

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

void lRecv(int reciever, void* buf){
	Header* header=malloc(sizeof(Header));
	header->tamanio=0;
	header->tipoProceso=0;
	recieveHeader(reciever,header);
	buf=malloc(sizeof(header->tamanio));
	internalRecv(reciever,buf,header->tamanio);
	free(header);
}

void recieveHeader(int socket, Header* header){
	puts("RecieveHeader");
	internalRecv(socket,header,sizeof(Header));
	puts("Header recibido");
	//Header header= *((Header*)buf);
	//printf("tamanio header: %d\n",header->tamanio);
}

void lSend(int sender, const void* msg, int len){
	_sendHeader(sender,1,len);//tipo de proceso hardcodeado, hay que ver de donde pija se saca
	internalSend(sender,msg,len);
}

void handleResults(int listener, socketHandler* list, socketHandler* result){//agregarle una lista al Handler que tenga los listeners
	int p;
	for(p=0;p<list->nfds;p++){
		puts("Entro al for\n");
		if(isReading(p,result)){
			puts("esta leyendo\n");
			if(p==listener){
				puts("Es listener\n");
				addReadSocket(lAccept(p),list);
				puts("new connection");
			}
			else{
				puts("No es Listener\n");
				char* data;
				lRecv(p,data);
				handleData(data);
				if (data!=NULL)free(data);
			}
		}
	}
}

void handleData(char* data){
	printf("La data es: %s",data);
	getchar();
}

socketHandler lSelect(socketHandler* handler, int duration){
	timeVal time= _setTimeVal(duration,0);
	int status= select(handler->nfds,handler->readSockets,handler->writeSockets,NULL,&time);
	errorIfEqual(status,-1,"select");
	return *handler;
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

int isReading(int reader, socketHandler* handler){
	return FD_ISSET(reader,handler->readSockets);
}

int isWriting(int writer, socketHandler* handler){
	return FD_ISSET(writer,handler->writeSockets);
}


