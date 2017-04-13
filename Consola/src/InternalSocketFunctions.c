#include "Header.h"
#include "InternalSocketFunctions.h"

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
	errorIfEqual(status=recv(reciever,buf,size,0),-1,"recv");
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

void _sendHeader(int sender,int tipoProceso,int len){
	Header header= _createHeader(tipoProceso, len);
	internalSend(sender,&header,sizeof(Header));
}

Header _createHeader(int tp,int size){
	Header header;
	header.tipoOperacion= tp;
	header.tamanio= size;
	return header;
}


timeVal _setTimeVal(int seconds, int microseconds){
	timeVal time;
	time.tv_sec= seconds;
	time.tv_usec= microseconds;
	return time;
}