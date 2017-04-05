typedef struct addrinfo addrInfo;
typedef struct sockadrr sockAddr;

int getBindedSocket(char*, char*);
int getConnectedSocket(char*, char*);
void lListen(int,int);
int lAccept(int, sockAddr*);
void lRecv(int, void*);
void lSend(int, const void*, int);
