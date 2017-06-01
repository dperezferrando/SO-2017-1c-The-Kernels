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

void pageToStoreTestPageAvailable(){
	_pageToStoreTest(1);
}

void pageToStoreTestPageUnavailable(){
	_pageToStoreTest(0);
}

void _pageToStoreTest(int pageAvailable){
	PageOwnership* po= malloc(sizeof(PageOwnership));
	ownedPages= list_create();
	po->idpage=1;
	po->pid=1;
	MemoryRequest mr;
	mr.pid=1;
	mr.size= config->PAG_SIZE-sizeof(HeapMetadata)*2;
	config->PAG_SIZE = pageAvailable==1 ? config->PAG_SIZE : 1+sizeof(HeapMetadata);
	initializePageOwnership(po);
	int res= pageToStore(mr);
	list_destroy(ownedPages);
	if(pageAvailable==1) {CU_ASSERT_NOT_EQUAL(res,-1);}
	else {CU_ASSERT_EQUAL(res,-1);}
}

void sendMemoryRequestTest(int viable){
	MemoryRequest mr;
	mr.pid=1;
	mr.size= viable==1 ? config->PAG_SIZE+1 : config->PAG_SIZE-1;

}
