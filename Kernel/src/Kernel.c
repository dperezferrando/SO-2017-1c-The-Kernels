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
#include "globales.h"
//#include "KernelConfiguration.h"
const char* keys[16] = {"PUERTO_PROG", "PUERTO_CPU", "IP_MEMORIA", "PUERTO_MEMORIA", "IP_FS", "PUERTO_FS", "QUANTUM", "QUANTUM_SLEEP", "ALGORITMO", "GRADO_MULTIPROG", "SEM_IDS", "SEM_INIT", "SHARED_VARS", "STACK_SIZE", "PAG_SIZE", "NULL"};


int main(int argc, char** argsv) {
	puts("!!!Hello Kernel!!!\n"); /* prints !!!Hello World!!! */
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Kernel/Debug/config.conf", readConfigFile, keys);
	procesos = list_create();
	handler(config);
	destruirConfig(config);
	//list_clean_and_destroy_elements(destruirPCB);
	return 0;
}

