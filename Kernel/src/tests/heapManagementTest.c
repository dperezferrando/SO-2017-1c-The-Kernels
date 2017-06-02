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
	PageOwnership* p2= list_get(ownedPages,0);
	HeapMetadata* hm= list_get(p2->occSpaces,1);
	CU_ASSERT_NOT_EQUAL(res,-1);
	//CU_ASSERT_NOT_EQUAL(hm->isFree,1);
	//CU_ASSERT_NOT_EQUAL(hm->size,8);
	list_destroy(ownedPages);
}

void sendMemoryRequestTest(int viable){
	MemoryRequest mr;
	mr.pid=1;
	mr.size= viable==1 ? config->PAG_SIZE+1 : config->PAG_SIZE-1;

}
