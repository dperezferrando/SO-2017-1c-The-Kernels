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

void sendMemoryRequestTest(int viable){
	MemoryRequest mr;
	mr.pid=1;
	mr.size= viable==1 ? config->PAG_SIZE+1 : config->PAG_SIZE-1;
}
