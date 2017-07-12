#include "HeapManagementTest.h"


void testSerializationMemoryRequest(){
	MemoryRequest mr;
	mr.pid=1;
	mr.size=10;
	void* msg = malloc(sizeof(char)*10);
	memcpy(msg,"Holangas",sizeof(char)*8);
	void* serial= serializeMemReq(mr,1,2,10,msg);
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
	sendMemoryRequest(mr,pp,10);
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
	if(!newPage) resp= grabarPedido(po,*mr,hm,offset);

	list_destroy(ownedPages);
	free(po);
	free(pp);
	free(mr);
	free(res);
	free(hm);

	int offs= *offset;

	free(offset);

	if(newPage){
		CU_ASSERT_EQUAL(resp,1);
		CU_ASSERT_EQUAL(offs,0);
	}
	else {
		CU_ASSERT_EQUAL(resp,0);
		CU_ASSERT_NOT_EQUAL(offs,0);
	}

}



void freeMemoryTest(){
	config->PAG_SIZE= 256;
	ownedPages= list_create();
	PageOwnership* pp= malloc(sizeof(PageOwnership));
	pp->pid=1;
	pp->idpage=1;
	initializePageOwnership(pp);
	HeapMetadata aux;
	aux.isFree=0;
	aux.size= 4;
	occupyPageSize(pp,&aux);
	list_add(ownedPages,pp);
	list_size(pp->occSpaces);
	MemoryRequest* mr= malloc(sizeof(MemoryRequest));
	mr->pid= 1;
	mr->size= 56;
	res=malloc(sizeof(Mensaje));
	res->header.tipoOperacion=1;
	res->data= malloc(sizeof(int));
	int a= 10;
	list_size(pp->occSpaces);
	memcpy(res->data,&a,sizeof(int));
	HeapMetadata* hm= initializeHeapMetadata(mr->size);
	int* offset= malloc(sizeof(int));
	grabarPedido(pp,*mr,hm,offset);
	list_size(pp->occSpaces);
	freeMemory(pp->pid,pp->idpage,*offset);

	int cantLibres=0,cantOcupados=0,i;

	for(i=0;i<list_size(pp->occSpaces);i++)
	{
		HeapMetadata* aux = list_get(pp->occSpaces,i);
		if(aux->isFree==true)cantLibres++;else cantOcupados++;
	}



	list_destroy(ownedPages);
	free(pp);
	free(mr);
	free(res);
	free(hm);
	free(offset);

	CU_ASSERT_EQUAL(cantLibres,2);
	CU_ASSERT_EQUAL(cantOcupados,1);

}


/*void defraggingTest(){
	config->PAG_SIZE= 512;

	t_list* page= list_create();

	void* memPage= malloc(512);
	void* aux= malloc (512);

	HeapMetadata* hm1= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm2= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm3= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm4= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm5= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm6= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm7= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm8= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm9= malloc(sizeof(HeapMetadata));

	hm1->isFree=0;
	hm1->size=50;
	hm2->isFree=1;
	hm2->size=50;
	hm3->isFree=0;
	hm3->size=80;
	hm4->isFree=0;
	hm4->size=30;
	hm5->isFree=0;
	hm5->size=50;
	hm6->isFree=1;
	hm6->size=50;
	hm7->isFree=0;
	hm7->size=30;
	hm8->isFree=0;
	hm8->size=80;
	hm9->isFree=1;
	hm9->size=20;

	list_add(page,hm1);
	list_add(page,hm2);
	list_add(page,hm3);
	list_add(page,hm4);
	list_add(page,hm5);
	list_add(page,hm6);
	list_add(page,hm7);
	list_add(page,hm8);
	list_add(page,hm9);

	memcpy(aux,hm1,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"A ver",50);
	memcpy(memPage,aux,sizeof(HeapMetadata)+50);


	memcpy(aux,hm2,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",50);
	memcpy(memPage+50+sizeof(HeapMetadata),aux,50);


	memcpy(aux,hm3,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"si esto",80);
	memcpy(memPage+100+sizeof(HeapMetadata)*2,aux,80);


	memcpy(aux,hm4,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"queda",30);
	memcpy(memPage+180+sizeof(HeapMetadata)*3,aux,30);


	memcpy(aux,hm5,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"en orden",50);
	memcpy(memPage+210+sizeof(HeapMetadata)*4,aux,50);


	memcpy(aux,hm6,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",50);
	memcpy(memPage+260+sizeof(HeapMetadata)*5,aux,50);


	memcpy(aux,hm7,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"o no",30);
	memcpy(memPage+310+sizeof(HeapMetadata)*6,aux,30);


	memcpy(aux,hm8,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"queda",80);
	memcpy(memPage+340+sizeof(HeapMetadata)*7,aux,80);


	memcpy(aux,hm9,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",20);
	memcpy(memPage+420+sizeof(HeapMetadata)*8,aux,120);

	res->data= memPage;

	void* newmemPage= defragging(1,1,page);

	HeapMetadata* rest = list_get(page,0);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,50);

	rest = list_get(page,1);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,80);

	rest = list_get(page,2);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,30);

	rest = list_get(page,3);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,50);

	rest = list_get(page,4);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,30);

	rest = list_get(page,5);
	CU_ASSERT_EQUAL(rest->isFree,0);
	CU_ASSERT_EQUAL(rest->size,80);

	rest = list_get(page,6);
	CU_ASSERT_EQUAL(rest->isFree,1);
	CU_ASSERT_EQUAL(rest->size,136);

	char* contenidoPagina= malloc(100);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata),50);
	char* control= malloc(5);
	memcpy(control,contenidoPagina,5);

	printf("\n1: %s\n",contenidoPagina);

	//printf("A ver = %s ;",control);

	free(control);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata)*2+50,80);
	control= malloc(7);
	memcpy(control,contenidoPagina,7);

	printf("2: %s\n",contenidoPagina);

	//printf("si esto = %s ;",control);

	free(control);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata)*3+130,30);
	control= malloc(5);
	memcpy(control,contenidoPagina,5);

	printf("3: %s\n",contenidoPagina);

	//printf("queda = %s ;",control);

	free(control);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata)*4+160,50);
	control= malloc(8);
	memcpy(control,contenidoPagina,8);

	printf("4: %s\n",contenidoPagina);

	//printf("en orden = %s ;",control);

	free(control);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata)*5+210,30);
	control= malloc(4);
	memcpy(control,contenidoPagina,4);

	printf("5: %s\n",contenidoPagina);

	//printf("o no = %s ;",control);

	free(control);

	memcpy(contenidoPagina,newmemPage+sizeof(HeapMetadata)*6+240,80);
	control= malloc(5);
	memcpy(control,contenidoPagina,5);

	printf("6: %s\n",contenidoPagina);

	//printf("queda = %s ; en orden.",control);

}*/

void defraggingTest(){
	config->PAG_SIZE= 512;

	t_list* page= list_create();

	void* memPage= malloc(512);
	void* aux= malloc (512);

	HeapMetadata* hm1= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm2= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm3= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm4= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm5= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm6= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm7= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm8= malloc(sizeof(HeapMetadata));
	HeapMetadata* hm9= malloc(sizeof(HeapMetadata));

	hm1->isFree=0;
	hm1->size=50;
	hm2->isFree=1;
	hm2->size=50;
	hm3->isFree=1;
	hm3->size=80;
	hm4->isFree=1;
	hm4->size=30;
	hm5->isFree=0;
	hm5->size=50;
	hm6->isFree=1;
	hm6->size=50;
	hm7->isFree=1;
	hm7->size=30;
	hm8->isFree=0;
	hm8->size=80;
	hm9->isFree=1;
	hm9->size=20;

	list_add(page,hm1);
	list_add(page,hm2);
	list_add(page,hm3);
	list_add(page,hm4);
	list_add(page,hm5);
	list_add(page,hm6);
	list_add(page,hm7);
	list_add(page,hm8);
	list_add(page,hm9);

	memcpy(aux,hm1,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"A ver",50);
	memcpy(memPage,aux,sizeof(HeapMetadata)+50);


	memcpy(aux,hm2,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",50);
	memcpy(memPage+50+sizeof(HeapMetadata),aux,50);


	memcpy(aux,hm3,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"si esto",80);
	memcpy(memPage+100+sizeof(HeapMetadata)*2,aux,80);


	memcpy(aux,hm4,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"queda",30);
	memcpy(memPage+180+sizeof(HeapMetadata)*3,aux,30);


	memcpy(aux,hm5,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"en orden",50);
	memcpy(memPage+210+sizeof(HeapMetadata)*4,aux,50);


	memcpy(aux,hm6,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",50);
	memcpy(memPage+260+sizeof(HeapMetadata)*5,aux,50);


	memcpy(aux,hm7,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"o no",30);
	memcpy(memPage+310+sizeof(HeapMetadata)*6,aux,30);


	memcpy(aux,hm8,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"queda",80);
	memcpy(memPage+340+sizeof(HeapMetadata)*7,aux,80);


	memcpy(aux,hm9,sizeof(HeapMetadata));
	memcpy(aux+sizeof(HeapMetadata),"",20);
	memcpy(memPage+420+sizeof(HeapMetadata)*8,aux,120);

	res->data= memPage;

	void* newPage= defragging(1,1,page);

	int i;

	/*for(i=0;list_size(page)>i;i++){
		HeapMetadata* hm43= list_get(page,i);
		printf("%d HM: is free %d, size %d     -----    ",i,hm43->isFree,hm43->size);
	}*/

	HeapMetadata* result= list_get(page,1);

	CU_ASSERT_EQUAL(result->size,170);

	memcpy(result,newPage+55,sizeof(HeapMetadata));

	CU_ASSERT_EQUAL(result->size,170);

	result= list_get(page,3);

	CU_ASSERT_EQUAL(result->size,85);

	memcpy(result,newPage+285,sizeof(HeapMetadata));

	CU_ASSERT_EQUAL(result->size,85);


}

void globalVariableTest(){
	GlobalVariable* gb= findGlobalVariable(config->SHARED_VARS[0]+1);
	CU_ASSERT_EQUAL(0,strcmp(config->SHARED_VARS[0]+1,gb->name));
	//puts("\n---------------------------------------------------------");
	//printf("\nEl nombre es: %s, y debería ser: %s\n",gb->name,"Global");
	//puts("---------------------------------------------------------");
	gb= findGlobalVariable(config->SHARED_VARS[1]+1);
	CU_ASSERT_EQUAL(0,strcmp(config->SHARED_VARS[1]+1,gb->name));
	//puts("---------------------------------------------------------");
	//printf("\nEl nombre es: %s, y debería ser: %s\n",gb->name,"UnaVar");
	//puts("---------------------------------------------------------");
	gb= findGlobalVariable(config->SHARED_VARS[2]+1);
	CU_ASSERT_EQUAL(0,strcmp(config->SHARED_VARS[2]+1,gb->name));
	//puts("---------------------------------------------------------");
	//printf("\nEl nombre es: %s, y debería ser: %s\n",gb->name,"tiempo3");
	//puts("---------------------------------------------------------\n");
}
