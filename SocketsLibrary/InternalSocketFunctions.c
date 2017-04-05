#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include "Header.h"
#include "InternalSocketFunctions.h"

addrInfo getaddrinfocall(char* ip, char* port) {

	addrInfo hints;
	addrInfo *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family= AF_UNSPEC;
	hints.ai_socktype= SOCK_STREAM;
	hints.ai_flags=AI_PASSIVE;

	errorIfNotEqual(getaddrinfo(ip,port,&hints,&servinfo),0,"getaddrinfo");
	return *servinfo;
}

int internalSocket(char* ip, char* port,int (*action)(int,struct sockaddr *,socklen_t)){
	addrInfo addr= getaddrinfocall(ip,port);
	int s= socket(addr.ai_family,addr.ai_socktype,addr.ai_protocol);
	errorIfEqual(action(s,addr.ai_addr,addr.ai_addrlen),-1,"action over socket");
	return s;
}

void internalRecv(int reciever, void* buf, int size){
	int status;
	errorIfEqual(status= recv(reciever,buf,size,0),-1,"Recieve");
	errorIfEqual(status,0,"Connection Closed");
}

void internalSend(int s, const void* msg, int len){
	errorIfEqual(send(s,msg,len,0),-1,"Send");//flags harcodeado en 0 pero se puede agregar de ser necesario
}

void errorIf(int (*criteria)(int,int), int value, int test, char* toPrint){
	if(criteria(value,test)) {
			fprintf(stderr,"%s error: %s\n", toPrint, gai_strerror(value));
			exit(1);
		}
}

void errorIfEqual(int value, int test, char* toPrint){
	int (*equals)(int,int)= &isEqual;
	errorIf(equals,value,test,toPrint);
}

void errorIfNotEqual(int value, int test, char* toPrint){
	int (*notEquals)(int,int)= &isNotEqual;
	errorIf(notEquals,value,test,toPrint);
}

int isEqual(int a, int b){
	return a==b;
}

int isNotEqual(int a, int b){
	return a!=b;
}

void sendHeader(int sender,int tipoProceso,int len){
	Header header= createHeader(tipoProceso, len);
	internalSend(sender,&header,sizeof(Header));
}

Header createHeader(int tp,int size){
	Header header;
	header.tipoProceso= tp;
	header.tamanio= size;
	return header;
}
