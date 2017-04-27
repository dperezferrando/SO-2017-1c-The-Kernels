#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"

const char* keys[3] = {"PUERTO", "PUNTO_MONTAJE" , "NULL"};

typedef struct {
	char puerto[5];
	char punto_montaje [15];
} configFile;

void imprimirConfig(configFile*);
configFile* leerArchivoConfig(t_config*);

configFile* leerArchivoConfig(t_config* configHandler)
{
	configFile* config= malloc(sizeof(configFile));
	strcpy(config->puerto, config_get_string_value(configHandler, "PUERTO"));
	strcpy(config->punto_montaje,config_get_string_value(configHandler, "PUNTO_MONTAJE"));
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}

void imprimirConfig(configFile* config)
{
	puts("--------PROCESO FILESYSTEM--------");
	printf("ESCUCHANDO EN PUERTO: %s | PUNTO_MONTAJE %s\n", config->puerto, config->punto_montaje);
	puts("--------PROCESO FILESYSTEM--------");
}

int main(int argc, char** argsv) {
	configFile* config;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/FileSistem/Debug/filesystem.conf", leerArchivoConfig, keys);
	int socket = getBindedSocket("127.0.0.1", config->puerto);
	free(config);
	lListen(socket, 5);
	int conexion = lAccept(socket, KERNEL_ID);
	puts("Esperando mensaje del Kernel");
	Mensaje* info = lRecv(conexion);
	while(info->header.tipoOperacion != -1)
	{
		char mensaje[25];
		strcpy(mensaje,info->data);
		printf("MENSAJE RECIBIDO: %s\n", mensaje);
		destruirMensaje(info);
		info = lRecv(conexion);
	}
	destruirMensaje(info);
	return EXIT_SUCCESS;
}
