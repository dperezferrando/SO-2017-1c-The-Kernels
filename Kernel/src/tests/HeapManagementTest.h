#ifndef __HEAP_MANAG_TEST__
#define __HEAP_MANAG_TEST__

#include "../ConnectionCore.h"
#include "CUnit/Basic.h"

void freeMemoryTest();
void defraggingTest();
void occupyPageSizeTest();
void globalVariableTest();
void grabarPedidoTestNewPage();
void grabarPedidoTestNotNewPage();
void pageToStoreTestPageAvailable();
void sendMemoryRequestNewPageTest();
void pageToStoreTestPageUnavailable();
void testSerializationMemoryRequest();
void sendMemoryRequestNotNewPageTest();


#endif
