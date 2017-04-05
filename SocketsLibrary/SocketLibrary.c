#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#include "Header.h"
#include "InternalSocketFunctions.h"
#include "SocketLibrary.h"

typedef struct addrinfo addrInfo;
typedef struct sockadrr sockAddr;

/*int getSocket(char* ip, char* port){
	return internalSocket(ip,port,*empty);
}
//Está para ser habilitado solo en caso de que por alguna razón uno necesite un socket que no bindea o conecta
void empty(int a, sockAddr b, int c){}*/

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
	sendHeader(sender,1,len);//tipo de proceso hardcodeado, hay que ver de donde pija se saca
	internalSend(sender,msg,len);
}
