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
#define CONFIG_FILE "memoria.conf"

typedef struct {
	int puerto;
	int marcos;
	int marco_size;
	int entradas_cache;
	int cache_x_proc;
	char reemplazo_cache[4];
	int retardo_memoria;
} configFile;

configFile leerArchivoConfig(t_config* configHandler)
{
	configFile config;
	config.puerto = config_get_int_value(configHandler, "PUERTO");
	config.marcos = config_get_int_value(configHandler, "MARCOS");
	config.marco_size = config_get_int_value(configHandler, "MARCO_SIZE");
	config.entradas_cache = config_get_int_value(configHandler, "ENTRADAS_CACHE");
	config.cache_x_proc = config_get_int_value(configHandler, "CACHE_X_PROC");
	strcpy(config.reemplazo_cache,config_get_string_value(configHandler, "REEMPLAZO_CACHE"));
	config.retardo_memoria = config_get_int_value(configHandler, "RETARDO_MEMORIA");
	config_destroy(configHandler);
	return config;
}

void imprimirConfig(configFile config)
{
	puts("--------PROCESO MEMORIA--------");
	printf("ESUCHANDO EN PUERTO: %i | CANTIDAD DE MARCOS: %i | TAMAÑO DE MARCOS: %i\n", config.puerto, config.marcos, config.marco_size);
	printf("ENTRADAS CACHE: %i | CACHE X PROC: %i | ALGORITMO REEMPLAZO: %s\n", config.entradas_cache, config.cache_x_proc, config.reemplazo_cache);
	printf("RETARDO DE MEMORIA: %i\n", config.retardo_memoria);
	puts("--------PROCESO MEMORIA--------");
}

bool archivoConfigCompleto(t_config* configHandler)
{
	char* keys[7] = {"PUERTO", "MARCOS", "MARCO_SIZE", "ENTRADAS_CACHE", "CACHE_X_PROC", "REEMPLAZO_CACHE", "RETARDO_MEMORIA"};
	bool archivoValido = true;
	int i = 0;
	for(i = 0;i<7;i++)
	{
		if(!config_has_property(configHandler, keys[i]))
		{
			archivoValido = false;
			puts("ERROR: ARCHIVO DE CONFIGURACION INCOMPLETO");
			i = 7;
		}
	}
	return archivoValido;
}

bool rutaCorrecta(t_config* configHandler)
{
	if(configHandler == NULL)
	{
		puts("ERROR: RUTA INCORRECTA PARA ARCHIVO DE CONFIGURACION");
		return false;
	}
	else return true;
}

bool archivoConfigValido(t_config* configHandler)
{
	return rutaCorrecta(configHandler) && archivoConfigCompleto(configHandler);

}


int main(int argc, char** argsv) {
	t_config* configHandler = config_create(argsv[1]);
	if(!archivoConfigValido(configHandler))
		return EXIT_FAILURE;
	configFile config = leerArchivoConfig(configHandler);
	imprimirConfig(config);
	return EXIT_SUCCESS;
}
