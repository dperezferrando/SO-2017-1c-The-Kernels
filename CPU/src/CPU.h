
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <pthread.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
const char* keys[5]={"IP_KERNEL", "PUERTO_KERNEL", "IP_MEMORIA", "PUERTO_MEMORIA", "NULL"};

typedef struct configFile{
	char ip_Kernel[16];
	char puerto_Kernel[5];
	char ip_Memoria[16];
	char puerto_Memoria[5];
} configFile;

void imprimir(configFile*);
configFile* leer_archivo_configuracion(t_config*);
void conexion_memoria();
void conexion_kernel();
