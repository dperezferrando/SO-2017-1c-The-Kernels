#ifndef __PROCESS__
#define __PROCESS__

#include "Configuration.h"
#include "globales.h"
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include <math.h>


ProcessControl* PIDFind(int);

PCB* fromNewToReady();
PCB* fromReadyToExecute();
PCB* fromExecuteToFinished();
PCB* createProcess(char*, int);
PCB* _fromTo(t_queue*, t_queue*, int);
PCB* _fromQueueToList(t_queue*, t_list*, int);
PCB* _fromQueueToQueue(t_queue*, t_queue*, int);
PCB* _fromListToQueue(t_list*, t_queue*, int PID, int);

void killProcess(int*);
void modifyProcessState(int, int);
void _processChangeStateToList(t_list*, PCB*, int);
void _processChangeStateToQueue(t_queue*, PCB*, int);

#endif
