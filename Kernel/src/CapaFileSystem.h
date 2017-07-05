#ifndef __CAPAFS__
#define __CAPAFS__
#include <stdlib.h>
#include "globales.h"
#include "commons/string.h"

tablaDeProceso* crearTablaDeProceso(int pid);
void crearEstructurasFSProceso(int pid);
void deserializarInfoArchivo(char* data, int* pid, char** ruta, char** permisos);
int abrirArchivo(int pid, char* ruta, char* permisos);
entradaTablaGlobalFS* buscarEnTablaGlobal(char* ruta);
bool agregarEntradaGlobal(char* ruta);
bool archivoValido(char* ruta);
int agregarEntradaTablaProceso(entradaTablaGlobalFS* entradaGlobal, int pid, char* permisos);
bool moverCursorArchivo(fileInfo info);
entradaTablaFSProceso* buscarEnTablaDelProceso(int pid, int fd);
char* leerArchivo(fileInfo info);
serializado serializarPedidoLectura(char* ruta, int offset, int size);
serializado serializarPedidoEscritura(char* ruta, int offset, int size, char* data);
#endif
