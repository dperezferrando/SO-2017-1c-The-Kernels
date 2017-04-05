typedef struct sockadrr sockAddr;
typedef struct addrinfo addrInfo;

int internalSocket(char*, char*,int (int,struct sockaddr *,socklen_t));
void internalRecv(int, void*, int);
void internalSend(int, const void*, int);
void errorIf(int (int,int), int, int, char*);
struct addrinfo getaddrinfocall(char* ip, char* port);
int isEqual(int a, int b);
int isNotEqual(int a, int b);
void errorIfEqual(int value, int test, char* toPrint);
void errorIfNotEqual(int value, int test, char* toPrint);
void sendHeader(int,int,int);
Header createHeader(int,int);
