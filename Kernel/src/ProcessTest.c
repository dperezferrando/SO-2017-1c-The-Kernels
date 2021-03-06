#include "ProcessTest.h"

//-----------------------------------------------------Tests Variables-----------------------------------------------------//


void testReadyProcessMultiprogOK(){

	testReadyProcess(1);

}

void testReadyProcessMultiprogNotOK(){

	testReadyProcess(0);

}

void testExecuteProcessCPUOk(){
	testExecuteProcess(1);
}

void testExecuteProcessCPUNotOk(){
	testExecuteProcess(0);
}

void testCPUReturnsProcessToReady(){
	testCPUReturnsProcess(1);
}

void testCPUReturnsProcessToBlocked(){
	testCPUReturnsProcess(3);
}

//--------------------------------------------------------------Tests Core-----------------------------------------------------------------//


void testNewProcess(){
	initializeProcessQueuesAndLists();//inicializo las listas
	initializePCB();//pcb fantasma
	newProcess(pcb, -1, "", 0);//procedimiento

	//------------------------------------------Asserts----------------------------------------//
	CU_ASSERT_EQUAL(queue_size(colaNew),1);
	pcb= queue_pop(colaNew);
	CU_ASSERT_EQUAL(pcb->pid,1);

	//-----------------------------------------------Destruction--------------------------------------------------//

	free(pcb);
}

void testReadyProcess(int multiprogOK){
	initializeProcessQueuesAndLists();//inicializo las listas
	//initializeConfigFile();
	//initializePCBinReady();
	initializePCB();
	newProcess(pcb, -1, "",0);
	int expected=1,cantNew=0,cantReady=1,state=1;
	if(!multiprogOK){
		expected= -1;
		cantNew=1;
		cantReady=0;
		state=0;
		config->GRADO_MULTIPROG= 0;
	}
	else{
		config->GRADO_MULTIPROG= 1;
	}

	int res= readyProcess();

	//------------------------------------------Asserts----------------------------------------//

	CU_ASSERT_EQUAL(res,expected);
	CU_ASSERT_EQUAL(queue_size(colaNew),cantNew);
	CU_ASSERT_EQUAL(queue_size(colaReady),cantReady);
	CU_ASSERT_EQUAL(list_size(process),1);
	ProcessControl* auxpc= list_get(process,0);
	CU_ASSERT_EQUAL(auxpc->pid,pcb->pid);
	CU_ASSERT_EQUAL(auxpc->state, state);

	//-----------------------------------------------Destruction--------------------------------------------------//

	free(pcb);
	free(auxpc);
}

void testExecuteProcess(int cpuOK){
	initializeProcessQueuesAndLists();
	initializePCB();
	newProcess(pcb, -1, "",0);
	config->GRADO_MULTIPROG= 1;
	readyProcess();
	int sizeReady=0,sizeExecute=1,expected=1;
	if(cpuOK) queue_push(colaCPUS, (int*)10);
	puts("1");
	int state= executeProcess();
	if(!cpuOK){
		sizeReady=1;
		sizeExecute=0;
		expected=-1;
	}

	CU_ASSERT_EQUAL(state,expected);
	CU_ASSERT_EQUAL(queue_size(colaReady),sizeReady);
	CU_ASSERT_EQUAL(list_size(executeList),sizeExecute);
}


void testCPUReturnsProcess(int processState){
	initializeProcessQueuesAndLists();
	initializePCB();
	newProcess(pcb,-1, "", 0);
	config->GRADO_MULTIPROG= 1;
	readyProcess();
	queue_push(colaCPUS, (int*)10);
	int state= executeProcess();
	CU_ASSERT_EQUAL(state,1);
	pcb->programCounter=2;
	cpuReturnsProcessTo(pcb,processState);

	int size= processState==1?queue_size(colaReady):list_size(blockedList);

	CU_ASSERT_EQUAL(size,1);

	PCB* newPCB= processState==1?queue_pop(colaReady):list_get(blockedList,0);

	CU_ASSERT_EQUAL(newPCB->programCounter,2);

	ProcessControl* pc= PIDFind(pcb->pid);

	CU_ASSERT_EQUAL(pc->state,processState);

}

//----------------------------------------------------------------Tests No Core-------------------------------------------------------------------//

void testPIDFind(){
	process = list_create();
	ProcessControl* pc1= malloc(sizeof(ProcessControl));
	pc1->pid= 1;
	pc1->state= 0;
	list_add(process,pc1);
	ProcessControl* pc2= malloc(sizeof(ProcessControl));
	pc2->pid= 3;
	pc2->state= 1;
	list_add(process,pc2);

	ProcessControl* pc3= PIDFindAndRemove(1);
	CU_ASSERT_EQUAL(pc3->pid,pc1->pid);
	CU_ASSERT_EQUAL(pc3->state,pc1->state);

	ProcessControl* pc4= PIDFindAndRemove(3);
	CU_ASSERT_EQUAL(pc4->pid,pc2->pid);
	CU_ASSERT_EQUAL(pc4->state,pc2->state);

	free(pc1);
	free(pc2);

}


void testModifyProcessState() {
	process = list_create();
	ProcessControl* pc1= malloc(sizeof(ProcessControl));
	pc1->pid= 1;
	pc1->state= 0;
	list_add(process,pc1);
	modifyProcessState(1,1);

	ProcessControl* pc2= PIDFindAndRemove(1);
	CU_ASSERT_EQUAL(pc2->state, 1);

	free(pc1);
}

void freProcessPagesTest(){
	ownedPages= list_create();
	PageOwnership* po1 = malloc(sizeof(PageOwnership));
	PageOwnership* po2 = malloc(sizeof(PageOwnership));
	po1->pid=0;
	po1->idpage=1;
	po2->pid=0;
	po2->idpage=2;
	initializePageOwnership(po1);
	HeapMetadata* hm1= malloc(sizeof(HeapMetadata));
	hm1->isFree= 1;
	hm1->size=100;
	list_add_in_index(po1->occSpaces,0,hm1);
	HeapMetadata* hmaux = list_get(po1->occSpaces,1);
	hmaux->size= hmaux->size-hm1->size;
	initializePageOwnership(po2);
	HeapMetadata* hm2= malloc(sizeof(HeapMetadata));
	hm2->isFree= 1;
	hm2->size=100;
	list_add_in_index(po2->occSpaces,0,hm2);
	hmaux = list_get(po2->occSpaces,1);
	hmaux->size= hmaux->size-hm1->size;

	list_add(ownedPages,po1);
	list_add(ownedPages,po2);

	freeProcessPages(0);

	CU_ASSERT_EQUAL(0,list_size(ownedPages));

	list_destroy(ownedPages);

}


//-----------------------------------------Funciones de Inicializacion-----------------------------------------------------//


int initializeProcessQueuesAndLists(){
	if(colaNew!=NULL) destroyProcessQueuesAndLists();
	colaNew = queue_create();
	colaCPUS = queue_create();
	colaReady = queue_create();
	blockedList = list_create();
	executeList = list_create();
	colaFinished=queue_create();

	process= list_create();

	return 0;
}

int initializeConfigFile(){
	/*const char* keys[16] = {"PUERTO_PROG", "PUERTO_CPU", "IP_MEMORIA", "PUERTO_MEMORIA", "IP_FS", "PUERTO_FS", "QUANTUM", "QUANTUM_SLEEP", "ALGORITMO", "GRADO_MULTIPROG", "SEM_IDS", "SEM_INIT", "SHARED_VARS", "STACK_SIZE", "PAG_SIZE", "NULL"};
	config = configurate("/home/utnso/workspace/tp-2017-1c-The-Kernels/Kernel/Debug/config.conf", readConfigFile, keys);*/
	return 0;
}

int initializePCB(){
	//pcb= createProcess("aa",sizeof("aa"));
	pcb= malloc(sizeof(PCB));
	pcb->pid=1;
	pcb->programCounter=1;
	pcb->exitCode=1;
	indCod* ic= malloc(sizeof(indCod));
	indStk* is= malloc(sizeof(indStk));
	ic->longitud=1;
	ic->offset=3;

	is->argumentos= list_create();
	is->variables= list_create();
	is->posicionDeRetorno= 0;

	variable a;
	a.identificador= 'a';
	posicionEnMemoria p;
	p.offset=0;
	p.pagina=0;
	p.size=0;
	a.posicion=p;

	is->variableDeRetorno=a;

	pcb->indiceCodigo= ic;
	pcb->sizeIndiceCodigo= sizeof(ic);
	pcb->indiceEtiqueta= "hola";
	pcb->sizeIndiceEtiquetas= 4;
	pcb->exitCode= 0;
	pcb->nivelDelStack=0;
	return 0;
}

int initializePCBinQueue(t_queue* queue){
	initializeProcessQueuesAndLists();
	initializePCB();
	queue_push(queue,pcb);
	return queue_size(queue)-1;
}

int initializePCBinList(t_list* list){
	initializeProcessQueuesAndLists();
	initializePCB();
	list_add(list,pcb);
	return list_size(list);
}

int initializePCBinNew(){
	ProcessControl auxpc;
	auxpc.pid= 1;
	auxpc.state= 0;
	list_add(process,&auxpc);
	return initializePCBinQueue(colaNew);
}

int initializePCBinReady(){
	ProcessControl auxpc;
	auxpc.pid= 1;
	auxpc.state= 1;
	list_add(process,&auxpc);
	return initializePCBinQueue(colaReady);
}


//-----------------------------------------Funciones de Destruccion-----------------------------------------------------//

int destroyProcessQueuesAndLists(){

	list_destroy(process);
	queue_destroy(colaNew);
	queue_destroy(colaCPUS);
	queue_destroy(colaReady);
	list_destroy(blockedList);
	list_destroy(executeList);
	queue_destroy(colaFinished);

	return 0;
}
