#ifndef __LISTEN__
#define __LISTEN__

#include "SocketLibrary.h"
#include "ConnectionCore.h"
#include "KernelConfiguration.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>



#define LOCALHOST "127.0.0.1"
#define BACKLOG 10

void handler();
void destroyConnHandler(connHandle*);
socketHandler updateSockets(connHandle);
connHandle initializeConnectionHandler();
void initialize(configFile*, connHandle*);

#endif
