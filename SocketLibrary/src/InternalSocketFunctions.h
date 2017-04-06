typedef struct sockadrr sockAddr;
typedef struct addrinfo addrInfo;
typedef struct timeval timeVal;

int internalSocket(char*, char*,int (int,struct sockaddr *,socklen_t));
void internalRecv(int, void*, int);
void internalSend(int, const void*, int);
void _errorIf(int (int,int), int, int, char*);
struct addrinfo _getaddrinfocall(char*, char*);
int _isEqual(int, int);
int _isNotEqual(int, int);
void errorIfEqual(int, int, char*);
void errorIfNotEqual(int, int, char*);
void _sendHeader(int,int,int);
Header _createHeader(int,int);
timeVal _setTimeVal(int, int);
