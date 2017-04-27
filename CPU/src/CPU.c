/*
 ============================================================================
 Name        : CPU.c
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

configFile* leer_archivo_configuracion(t_config* configHandler){

	configFile* config = malloc(sizeof(configFile));
	strcpy(config->puerto_Kernel, config_get_string_value(configHandler, "PUERTO_KERNEL"));
	strcpy(config->puerto_Memoria, config_get_string_value(configHandler, "PUERTO_MEMORIA"));
	strcpy(config->ip_Kernel, config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->ip_Memoria, config_get_string_value(configHandler, "IP_MEMORIA"));
	config_destroy(configHandler);
	imprimir(config);
	return config;
}

void imprimir(configFile* c){
	puts("--------PROCESO CPU--------");
	printf("IP KERNEL: %s\n", c->ip_Kernel);
	printf("PUERTO KERNEL: %s\n", c->puerto_Kernel);
	printf("IP MEMORIA: %s\n", c->ip_Memoria);
	printf("PUERTO MEMORIA: %s\n", c->puerto_Memoria);
	puts("--------PROCESO CPU--------");

}



int main(int argc, char** argsv) {

	configFile* config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/CPU/Debug/CPU.conf", leer_archivo_configuracion, keys);
	int conexion = getConnectedSocket(config->ip_Kernel, config->puerto_Kernel, CPU_ID);
	puts("Esperando el mensaje del Kernel");
	Mensaje* info = lRecv(conexion);
	while(info->header.tipoOperacion != -1)
	{
		char mensaje[25];
		strcpy(mensaje,info->data);
		destruirMensaje(info);
		printf("MENSAJE RECIBIDO: %s\n", mensaje);
		info = lRecv(conexion);
	}
	destruirMensaje(info);
	free(config);
	return EXIT_SUCCESS;


}




