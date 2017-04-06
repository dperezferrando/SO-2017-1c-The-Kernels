#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "Header.h"
#include "InternalSocketFunctions.h"
#include "SocketLibrary.h"

typedef struct socketHandler{
	fd_set* readSockets;
	fd_set* writeSockets;
}socketHandler;


int getHigherFileDescriptor(socketHandler handler){//esto no estoy seguro
	int higher=0;
	int* p;
	for(p= handler.readSockets ; p != NULL ;p++){
		if(higher<p)higher=p;
	}
	return higher;
}

/*int getSocket(char* ip, char* port){
	return internalSocket(ip,port,*empty);
}
//Está para ser habilitado solo en caso de que por alguna razón uno necesite un socket que no bindea o conecta
void empty(int a, sockAddr b, int c){}*/


//-----------------------------------------MAIN FUNCTIONS-------------------------------------------------------------------//

int getBindedSocket(char* ip, char* port){
	return internalSocket(ip,port,&bind);
}

int getConnectedSocket(char* ip, char* port){
	return internalSocket(ip,port,&connect);
}

void lListen(int socket,int backlog){//hay que cambiarle el nombre
	errorIfEqual(listen(socket,backlog),-1,"Listen");//cantidad de conexiones que acepta, osea de sockets que voy a manejar
}

int lAccept(int sockListener, sockAddr* addr){
	int newSocket;
	errorIfEqual(newSocket= accept(sockListener,addr,sizeof(&(addr))),-1,"accept");
	return newSocket;
}

void lRecv(int reciever, void* buf){
	Header* header;
	internalRecv(reciever,header,sizeof(Header));
	buf=malloc(header->tamanio);
	internalRecv(reciever,buf,header->tamanio);
}

void lSend(int sender, const void* msg, int len){
	_sendHeader(sender,1,len);//tipo de proceso hardcodeado, hay que ver de donde pija se saca
	internalSend(sender,msg,len);
}

socketHandler lSelect(socketHandler handler, int duration){
	timeVal time= _setTimeVal(duration,0);
	int higherFD= getHigherFileDescriptor(handler);
	int status= select(higherFD,handler.readSockets,handler.writeSockets,NULL,&time);
	errorIfEqual(status,-1,"select");
	return handler;
}


//---------------------------------------------------------------------------------------------//

void addReadSocket(int reader,socketHandler handler){
	FD_SET(reader,handler.readSockets);
}

void addWriteSocket(int writer, socketHandler handler){
	FD_SET(writer,handler.writeSockets);
}

void rmvReadSocket(int reader, socketHandler handler){
	FD_CLR(reader,handler.readSockets);
}

void rmvWriteSocket(int writer, socketHandler handler){
	FD_CLR(writer,handler.writeSockets);
}

void clrReaders(socketHandler handler){
	FD_ZERO(handler.readSockets);
}

void clrWriters(socketHandler handler){
	FD_ZERO(handler.writeSockets);
}

void clrHandler(socketHandler handler){
	clrReaders(handler);
	clrWriters(handler);
}

int isReading(int reader, socketHandler handler){
	return FD_ISSET(reader,handler.readSockets);
}

int isWriting(int writer, socketHandler handler){
	return FD_ISSET(writer,handler.writeSockets);
}



