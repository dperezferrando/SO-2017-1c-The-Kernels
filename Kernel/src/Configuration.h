#ifndef __config__
#define __config__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/config.h>


bool rutaCorrecta(t_config* configHandler);
bool archivoConfigValido(t_config* configHandler,char* []);
bool archivoConfigCompleto(t_config* configHandler, char* []);
void* configurate(char* ,void*(t_config*), char* []);

typedef struct __attribute__((packed)) indCod{
	int offset;
	int longitud;
}indCod;

typedef struct __attribute__((packed)) IDIndCod{
	int ID;
	int* pagina;
	int* offset;
	int* longitud;
}IDIndCod;

typedef struct __attribute__((packed)) indEtq{
	//diccionario de etiquetas a posicion de etiqueta
}indEtq;

typedef struct __attribute__((packed)) indStk{
	IDIndCod argumentos;
	IDIndCod variables;
	int  posicionDeRetorno;
	indCod variableDeRetorno;
}indStk;

typedef struct __attribute__((packed)) PCB{
	int pid;
	int programCounter;
	int cantPaginasCodigo;
	indCod* indiceCodigo;
	indEtq indiceEtiqueta;
	indStk indiceStack;
	int exitCode;
	int sizeIndiceCodigo;
}PCB;

typedef struct __attribute__((packed)) posicionEnMemoria {
	int pagina;
	int offset;
	int size;
} posicionEnMemoria;

#endif
