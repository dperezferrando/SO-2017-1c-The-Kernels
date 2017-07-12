#ifndef __PROC_TEST__
#define __PROC_TEST__
#include "Process.h"
#include "CUnit/Basic.h"

PCB* pcb;
ProcessControl* pc;

/*t_queue* colaNew;
t_queue* colaCPUS;
t_queue* colaReady;
t_list* blockedList;
t_list* executeList;
t_queue* colaFinished;

t_list* process;

configFile* config;*/

int initializePCB();
int initializePCBinNew();
int initializePCBinReady();
int initializePCBinList(t_list*);
int destroyProcessQueuesAndLists();
int initializeProcessQueuesAndLists();
int initializePCBinQueue(t_queue* queue);

void testPIDFind();
void testNewProcess();
void testReadyProcess(int);
void freProcessPagesTest();
void testModifyProcessState();
void testExecuteProcessCPUOk();
void testExecuteProcessCPUNotOk();
void testReadyProcessMultiprogOK();
void testCPUReturnsProcessToReady();
void testCPUReturnsProcessToBlocked();
void testReadyProcessMultiprogNotOK();

#endif
