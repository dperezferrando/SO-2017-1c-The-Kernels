/*
 ============================================================================
 Name        : ClientePrueba.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

#include "SocketLibrary.h"

#define LOCALHOST "127.0.0.1"
#define PUERTO "7171"

int main(void) {

	int s= getConnectedSocket(LOCALHOST,PUERTO);
	char mensaje[5];
	strcpy(mensaje,"Hola");
	Header header;
	header.tipoProceso=1;
	header.tamanio=sizeof(mensaje);
	send(s,&header,sizeof(Header),0);
	send(s,mensaje,sizeof(mensaje),0);
	return EXIT_SUCCESS;
}
