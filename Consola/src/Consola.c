/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
#define CONFIG_FILE "consola.conf"
const char* keys[3] = {"IP_KERNEL", "PUERTO_KERNEL", "NULL"};

typedef struct {
	char ip_kernel[16];
	char puerto_kernel[5];

} configFile;

void imprimirConfig(configFile* config) {
	puts("--------PROCESO CONSOLA--------");
	printf("IP KERNEL: %s", config->ip_kernel);
	printf("\n");
	printf("PUERTO KERNEL: %s\n", config->puerto_kernel);
	puts("--------PROCESO CONSOLA--------");
}

configFile* leerArchivoConfig(t_config* configHandler)
{
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->ip_kernel, config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->puerto_kernel, config_get_string_value(configHandler, "PUERTO_KERNEL"));
	config_destroy(configHandler);
	imprimirConfig(config);

	return config;
}


int main(void) {
	configFile* config;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Consola/Debug/consola.conf", leerArchivoConfig, keys);
	int socket = getConnectedSocket(config->ip_kernel, config->puerto_kernel, CONSOLA_ID);
	char mensaje[25];
	puts("Ingresa el mensaje a enviar. Maximo 24 caracteres, sin espacios.");
	scanf("%s", mensaje);
	lSend(socket, mensaje, strlen(mensaje)+1);
	puts("Enviado!");
	free(config);
	return 0;
}
