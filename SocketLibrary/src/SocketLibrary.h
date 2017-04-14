#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>

#include "Header.h"
#include "InternalSocketFunctions.h"
#define KERNEL_ID 0
#define MEMORIA_ID 1
#define CPU_ID 2
#define FS_ID 3
#define CONSOLA_ID 4


int lAccept(int, int);
int getBindedSocket(char*, char*);
int isReading(int, socketHandler);
int isWriting(int, socketHandler*);
int getConnectedSocket(char*, char*, int);
int enviarHandShake(int, int);
int recibirHandShake(int, int);


//-----------------------------------------------------//

void lListen(int,int);
void* lRecv(int);
void clrReaders(socketHandler*);
void clrWriters(socketHandler*);
void clrHandler(socketHandler*);
void recieveHeader(int, Header*);
void lSend(int, const void*, int);
void addReadSocket(int, socketHandler*);
void rmvReadSocket(int, socketHandler*);
void closeConnection(int,socketHandler*);
void addWriteSocket(int, socketHandler*);
void rmvWriteSocket(int, socketHandler*);

//-----------------------------------------------------//

socketHandler initializeSocketHandler();
socketHandler lSelect(socketHandler, int);
socketHandler copySocketHandler(socketHandler);



