#ifndef __PROCESS__
#define __PROCESS__

#include "Configuration.h"
#include "globales.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>

int PIDFind(int);

void killProcess(int*);
void modifyProcessState(int, int);

PCB* fromNewToReady();
PCB* fromReadyToExecute();
PCB* createProcess(char*, int);
PCB* _fromTo(t_queue*, t_queue*, int);

#endif
