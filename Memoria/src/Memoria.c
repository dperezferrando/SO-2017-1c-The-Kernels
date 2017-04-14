/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "Configuration.h"
#include "SocketLibrary.h"
#define CONFIG_FILE "memoria.conf"
const char* keys[8] = {"PUERTO", "MARCOS", "MARCO_SIZE", "ENTRADAS_CACHE", "CACHE_X_PROC", "REEMPLAZO_CACHE", "RETARDO_MEMORIA", "NULL"};

typedef struct {
	char puerto[5];
	int marcos;
	int marco_size;
	int entradas_cache;
	int cache_x_proc;
	char reemplazo_cache[4];
	int retardo_memoria;
} configFile;

void imprimirConfig(configFile* config)
{
	puts("--------PROCESO MEMORIA--------");
	printf("ESCUCHANDO EN PUERTO: %s | CANTIDAD DE MARCOS: %i | TAMAÑO DE MARCOS: %i\n", config->puerto, config->marcos, config->marco_size);
	printf("ENTRADAS CACHE: %i | CACHE X PROC: %i | ALGORITMO REEMPLAZO: %s\n", config->entradas_cache, config->cache_x_proc, config->reemplazo_cache);
	printf("RETARDO DE MEMORIA: %i\n", config->retardo_memoria);
	puts("--------PROCESO MEMORIA--------");
}

configFile* leerArchivoConfig(t_config* configHandler)
{
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->puerto, config_get_string_value(configHandler, "PUERTO"));
	config->marcos = config_get_int_value(configHandler, "MARCOS");
	config->marco_size = config_get_int_value(configHandler, "MARCO_SIZE");
	config->entradas_cache = config_get_int_value(configHandler, "ENTRADAS_CACHE");
	config->cache_x_proc = config_get_int_value(configHandler, "CACHE_X_PROC");
	strcpy(config->reemplazo_cache,config_get_string_value(configHandler, "REEMPLAZO_CACHE"));
	config->retardo_memoria = config_get_int_value(configHandler, "RETARDO_MEMORIA");
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}

int main(int argc, char** argsv) {
	configFile* config;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Memoria/Debug/memoria.conf", leerArchivoConfig, keys);
	int socket = getBindedSocket("127.0.0.1", config->puerto);
	free(config);
	lListen(socket, 5);
	int conexion = lAccept(socket);
	if(recibirHandShake(conexion) != KERNEL_ID)
	{
		close(conexion);
		puts("Proceso equivocado");
	}
	else
	{
		int* confirmacion = malloc(sizeof(int));
		*confirmacion = 1;
		lSend(conexion, 0, confirmacion, sizeof(confirmacion));
		free(confirmacion);
		int operacion;
		char mensaje[25];
		strcpy(mensaje,lRecv(conexion, &operacion));
		printf("MENSAJE: %s", mensaje);
		//free(mensaje);

	}
	return EXIT_SUCCESS;
}
