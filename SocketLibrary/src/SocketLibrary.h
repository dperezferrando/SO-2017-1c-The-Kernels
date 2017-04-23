#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>

#define KERNEL_ID 0
#define MEMORIA_ID 1
#define CPU_ID 2
#define FS_ID 3
#define CONSOLA_ID 4

typedef struct sockadrr sockAddr;
typedef struct addrinfo addrInfo;
typedef struct timeval timeVal;
typedef struct socketHandler{
	fd_set* readSockets;
	fd_set* writeSockets;
	int nfds;
}socketHandler;

typedef struct Header {
	int tamanio;
} Header;


// PRIVADAS

int internalSocket(char*, char*,int (int,const struct sockaddr *,socklen_t));
int internalRecv(int, void*, int);
int _isEqual(int, int);
void internalSend(int, void*, int);
void _errorIf(int (int,int), int, int, char*);
struct addrinfo _getaddrinfocall(char*, char*);
int _isNotEqual(int, int);
void errorIfEqual(int, int, char*);
void errorIfNotEqual(int, int, char*);
void _sendHeader(int,int);
Header _createHeader(int);
timeVal _setTimeVal(int, int);
int _getFirstSocket(addrInfo*, int (int,const struct sockaddr *,socklen_t));

// PUBLICAS

socketHandler initializeSocketHandler();
void destroySocketHandler(socketHandler);
socketHandler lSelect(socketHandler, int);
socketHandler copySocketHandler(socketHandler);
int lAccept(int, int);
int getBindedSocket(char*, char*);
int isReading(int, socketHandler);
int isWriting(int, socketHandler);
int getConnectedSocket(char*, char*, int);
int enviarHandShake(int, int);
int recibirHandShake(int, int);


//-----------------------------------------------------//

void lListen(int,int);
void* lRecv(int);
void clrReaders(socketHandler*);
void clrWriters(socketHandler*);
void clrHandler(socketHandler*);
int recieveHeader(int, Header*);
void lSend(int, const void*, int);
void addReadSocket(int, socketHandler*);
void rmvReadSocket(int, socketHandler*);
void closeConnection(int,socketHandler*);
void addWriteSocket(int, socketHandler*);
void rmvWriteSocket(int, socketHandler*);

//-----------------------------------------------------//
