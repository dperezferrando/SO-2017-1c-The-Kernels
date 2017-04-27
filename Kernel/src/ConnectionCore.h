#include <commons/collections/list.h>
typedef struct connectionHandler{
	socketHandler memoria;
	socketHandler fs;
	socketHandler cpu;
	socketHandler consola;
	int listenCPU;
	int listenConsola;
}connHandle;


bool fsSock(int, connHandle*);
bool memSock(int, connHandle*);
bool cpuSock(int, connHandle*);
bool consSock(int, connHandle*);
bool isListener(int, connHandle);
void handleSockets(Mensaje**, connHandle*, socketHandler);
