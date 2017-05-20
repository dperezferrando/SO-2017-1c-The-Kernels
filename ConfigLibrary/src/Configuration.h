#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/config.h>


bool rutaCorrecta(t_config* configHandler);
bool archivoConfigValido(t_config* configHandler,char* []);
bool archivoConfigCompleto(t_config* configHandler, char* []);
void* configurate(char* ,void*(t_config*), char* []);
char* serializacionPcb (PCB);
PCB deserializacionPcb (char*);

typedef struct indCod{
	int* pagina;
	int* offset;
	int* longitud;
}indCod;

typedef struct IDIndCod{
	int ID;
	int* pagina;
	int* offset;
	int* longitud;
}IDIndCod;

typedef struct indEtq{
	//diccionario de etiquetas a posicion de etiqueta
}indEtq;

typedef struct indStk{
	IDIndCod argumentos;
	IDIndCod variables;
	int  posicionDeRetorno;
	indCod variableDeRetorno;
}indStk;

typedef struct PCB{
	int pid;
	int programCounter;
	int cantPaginasCodigo;
	indCod indiceCodigo;
	indEtq indiceEtiqueta;
	indStk indiceStack;
	int exitCode;
}PCB;
