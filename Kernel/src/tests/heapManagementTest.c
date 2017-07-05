#include "HeapManagementTest.h"


void testSerializationMemoryRequest(){
	MemoryRequest mr;
	mr.pid=1;
	mr.size=10;
	void* serial= serializeMemReq(mr);
	MemoryRequest mr2;
	mr2= deserializeMemReq(serial);
	CU_ASSERT_EQUAL(mr.pid,mr2.pid);
	CU_ASSERT_EQUAL(mr.size,mr2.size);
}

void sendMemoryRequestNewPageTest(){
	sendMemoryRequestTest(1);
}

void sendMemoryRequestNotNewPageTest(){
	sendMemoryRequestTest(0);
}

void grabarPedidoTestNewPage(){
	grabarPedidoTest(1);
}

void grabarPedidoTestNotNewPage(){
	grabarPedidoTest(0);
}

void pageToStoreTestPageUnavailable(){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	ownedPages= list_create();
	po->idpage=1;
	po->pid=1;
	initializePageOwnership(po);
	HeapMetadata* hm= list_get(po->occSpaces,0);
	hm->isFree=0;
	hm->size= config->PAG_SIZE-sizeof(HeapMetadata);
	list_replace_and_destroy_element(po->occSpaces,0,hm,&free);
	list_add(ownedPages,po);
	MemoryRequest mr;
	mr.pid=1;
	mr.size= config->PAG_SIZE-(sizeof(HeapMetadata)*3);
	int res= pageToStore(mr);
	CU_ASSERT_EQUAL(res,-1);
	list_destroy(ownedPages);
}

void pageToStoreTestPageAvailable(){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	ownedPages= list_create();
	po->idpage=1;
	po->pid=1;
	initializePageOwnership(po);
	list_add(ownedPages,po);
	MemoryRequest mr;
	mr.pid=1;
	mr.size= config->PAG_SIZE-(sizeof(HeapMetadata)*3);
	int res= pageToStore(mr);
	CU_ASSERT_NOT_EQUAL(res,-1);
	list_destroy(ownedPages);
}


void occupyPageSizeTest(){

	config->PAG_SIZE= 256;
	PageOwnership* po= malloc(sizeof(PageOwnership));
	HeapMetadata* hmPop;
	po->idpage=1;
	po->pid=1;
	initializePageOwnership(po);
	HeapMetadata* hm= malloc(sizeof(HeapMetadata));
	hm->isFree= 0;
	hm->size= 56;


	occupyPageSize(po,hm);

	hmPop= list_get(po->occSpaces,0);
	CU_ASSERT_EQUAL(hmPop->isFree,0);
	CU_ASSERT_EQUAL(hmPop->size,56);
	hmPop= list_get(po->occSpaces,1);
	CU_ASSERT_EQUAL(hmPop->isFree,1);
	CU_ASSERT_EQUAL(hmPop->size,184);


	HeapMetadata* hm2= malloc(sizeof(HeapMetadata));
	hm2->isFree= 0;
	hm2->size=30;


	occupyPageSize(po,hm2);

	hmPop= list_get(po->occSpaces,1);
	CU_ASSERT_EQUAL(hmPop->isFree,0);
	CU_ASSERT_EQUAL(hmPop->size,30);
	hmPop= list_get(po->occSpaces,2);
	CU_ASSERT_EQUAL(hmPop->isFree,1);
	CU_ASSERT_EQUAL(hmPop->size,146);


	HeapMetadata* hm3= malloc(sizeof(HeapMetadata));
	hm3->isFree= 0;
	hm3->size=70;


	occupyPageSize(po,hm3);

	hmPop= list_get(po->occSpaces,2);
	CU_ASSERT_EQUAL(hmPop->isFree,0);
	CU_ASSERT_EQUAL(hmPop->size,70);
	hmPop= list_get(po->occSpaces,3);
	CU_ASSERT_EQUAL(hmPop->isFree,1);
	CU_ASSERT_EQUAL(hmPop->size,68);


	free(hmPop);
	free(po);
	free(hm);
}

void sendMemoryRequestTest(int newPage){
	config->PAG_SIZE=256;
	MemoryRequest mr;
	mr.pid=1;
	mr.size= 50;
	ownedPages= list_create();
	PageOwnership* po= malloc(sizeof(PageOwnership));
	po->pid=1;
	po->idpage=1;
	initializePageOwnership(po);
	if(!newPage) list_add(ownedPages,po);
	PageOwnership* pp= malloc(sizeof(PageOwnership));
	pp->pid=1;
	//sendMemoryRequest(mr,4,"hola",pp); no compila, to be fixed
	if(newPage) {CU_ASSERT_EQUAL(pp->idpage,-1);}
	else {CU_ASSERT_NOT_EQUAL(pp->idpage,-1);}
	list_destroy(ownedPages);
	list_destroy(po->control);
	list_destroy(po->occSpaces);
	free(po);
	free(pp);
}

void grabarPedidoTest(int newPage){
	config->PAG_SIZE= 256;
	ownedPages= list_create();
	PageOwnership* pp= malloc(sizeof(PageOwnership));
	if(!newPage){
		pp->pid=1;
		pp->idpage=1;
		initializePageOwnership(pp);
		HeapMetadata aux;
		aux.isFree=0;
		aux.size= 4;
		occupyPageSize(pp,&aux);
		list_add(ownedPages,pp);
	}
	PageOwnership* po= malloc(sizeof(PageOwnership));
	MemoryRequest* mr= malloc(sizeof(MemoryRequest));
	mr->pid= newPage ? -1 : 1;
	mr->size= 56;
	po->idpage= newPage ? -1 : 1;
	res=malloc(sizeof(Mensaje));
	res->header.tipoOperacion=1;
	res->data= malloc(sizeof(int));
	int a= 10;
	memcpy(res->data,&a,sizeof(int));
	HeapMetadata* hm= initializeHeapMetadata(mr->size);
	//sendMemoryRequest(*mr,4,"hola",po); no compila, to be fixed
	int* offset= malloc(sizeof(int));
	int resp= grabarPedido(po,*mr,hm,offset);

	list_destroy(ownedPages);
	free(po);
	free(pp);
	free(mr);
	free(res);
	free(hm);


	if(newPage){
		CU_ASSERT_EQUAL(resp,1);
	}
	else {
		CU_ASSERT_EQUAL(resp,0);
	}

}
