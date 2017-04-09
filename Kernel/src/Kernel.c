/*
 ============================================================================
 Name        : Kernel.c
 Author      : Santiago M. Lorenzo
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Listen.h"


int main(void) {
	puts("!!!Hello Kernel!!!\n"); /* prints !!!Hello World!!! */
	handler();
	puts("Todo ok\n");
	return 0;
}

/*
 * Por lo que estoy pensando se requeriría id de proceso Y id de operacion
 * porque uno poodría querer manejar de cierta manera una operacion por un proceso
 * y de otra manera distinta una operacion por otro proceso
 *
 */

