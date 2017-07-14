#ifndef __config__
#define __config__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

// COLORES
#define ROJO    "\x1b[31m"
#define VERDE   "\x1b[32m"
#define AMARILLO "\x1b[33m"
#define AZUL    "\x1b[34m"
#define ROSA "\x1b[35m"
#define CELESTE    "\x1b[36m"
#define RESET  "\x1b[0m"

//Globales para serializar OpFS
char* pathOpFS;
char* bufferOpFS;
int offsetOpFS;
int sizeOpFS;

typedef struct serializado
{
	char* data;
	int size;
} serializado;

typedef struct __attribute__((packed)) indCod{
	int offset;
	int longitud;
}indCod;

typedef struct __attribute__((packed)) posicionEnMemoria {
	int pagina;
	int offset;
	int size;
} posicionEnMemoria;


typedef struct __attribute__ ((packed)) variable {
	char identificador;
	posicionEnMemoria posicion;
} variable;

typedef struct __attribute__((packed)) indStk{
	t_list* argumentos;
	t_list* variables;
	int  posicionDeRetorno;
	variable variableDeRetorno;
}indStk;

typedef struct __attribute__((packed)) PCB{
	int pid;
	int programCounter;
	int cantPaginasCodigo;
	indCod* indiceCodigo;
	char* indiceEtiqueta;
	indStk* indiceStack;
	int exitCode;
	int sizeIndiceCodigo;
	int sizeIndiceEtiquetas;
	int nivelDelStack;
	int rafagasTotales;
}PCB;

typedef struct __attribute__ ((packed)) pedidoEscrituraMemoria {
	posicionEnMemoria posicion;
	int valor;
} pedidoEscrituraMemoria;

typedef struct __attribute__ ((packed)) fileInfo {
	int pid;
	int fd;
	int cursor;
	int tamanio;
} fileInfo;

bool rutaCorrecta(t_config* configHandler);
bool archivoConfigValido(t_config* configHandler,char* []);
bool archivoConfigCompleto(t_config* configHandler, char* []);
void* configurate(char* ,void*(t_config*), char* []);
serializado serializarPCB (PCB* pcb);
serializado serializarOpFS(int);
PCB* deserializarPCB (char*);
serializado serializarIndiceDeStack(indStk* indiceStack, int);
indStk* deserializarIndiceDeStack(serializado indiceSerializado, int);
indStk* crearIndiceDeStack();
void mostrarVariable(variable*);
void mostrarIndiceDeStack(indStk*, int);
void destruirIndiceDeStack(indStk*, int);
void destruirPCB(PCB*);
#endif
