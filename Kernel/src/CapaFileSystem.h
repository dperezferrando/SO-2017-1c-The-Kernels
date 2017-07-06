#ifndef __CAPAFS__
#define __CAPAFS__
#include <stdlib.h>
#include "globales.h"
#include "commons/string.h"


bool archivoValido(char* ruta);
bool cerrarArchivo(int pid, int fd);
bool borrarArchivo(int pid, int fd);
bool moverCursorArchivo(fileInfo info);
bool escribirArchivo(fileInfo info, char* data);


char* leerArchivo(fileInfo info);


int abrirArchivo(int pid, char* ruta, char* permisos);
int agregarEntradaTablaProceso(entradaTablaGlobalFS* entradaGlobal, int pid, char* permisos);


void crearEstructurasFSProceso(int pid);
void destruirTablaProceso(tablaDeProceso* tabla);
void imprimirPorPantalla(fileInfo info, char* data);
void destruirEntradaGlobal(entradaTablaGlobalFS* entrada);
void cerrarArchivoEnTablaGlobal(entradaTablaGlobalFS* entrada);
void destruirEntradaTablaProceso(entradaTablaFSProceso* entrada);
void deserializarInfoArchivo(char* data, int* pid, char** ruta, char** permisos);


entradaTablaGlobalFS* buscarEnTablaGlobal(char* ruta);
entradaTablaFSProceso* buscarEnTablaDelProceso(int pid, int fd);
entradaTablaGlobalFS* agregarEntradaGlobal(char* ruta, char* permisos);


serializado serializarPedidoLectura(char* ruta, int offset, int size);
serializado serializarPedidoEscritura(char* ruta, int offset, int size, char* data);


tablaDeProceso* crearTablaDeProceso(int pid);



#endif
