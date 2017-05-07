/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "CPU.h"

AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_asignarValorCompartida				= asignarValorCompartida
};


int main(int argc, char** argsv) {
	// COSAS DEL KERNEL COMENTADAS PORQUE ESTA RIP

	configFile* config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/CPU/Debug/CPU.conf", leer_archivo_configuracion, keys);
	kernel = getConnectedSocket(config->ip_Kernel, config->puerto_Kernel, CPU_ID);
	memoria = getConnectedSocket(config->ip_Memoria, config->puerto_Memoria, CPU_ID);
	pthread_t conexionKernel, conexionMemoria;
	pthread_create(&conexionKernel, NULL, conexion_kernel, NULL);
	pthread_create(&conexionMemoria, NULL, conexion_memoria, NULL);
	pthread_join(conexionMemoria, NULL);
	pthread_join(&conexionKernel, NULL);
	free(config);
	close(kernel);
	close(memoria);
	return EXIT_SUCCESS;


}

void conexion_kernel() // CODIGO DE TESTEO; NO TIENE QUE VER CON EL ENUNCIADO
{
	puts("KERNEL CONECTADO - ENVIANDO MENSAJE <3");
	lSend(kernel, "Hola soy un test", 1, sizeof(char)*17); // TESTING

}

void conexion_memoria() // CODIGO DE TESTEO; NO TIENE QUE VER CON EL ENUNCIADO. SE MANDAN MENSAJES A MEMORIA. "SALIR" PARA TERMINAR
{
	puts("HILI");

	char mensaje[10];
	scanf("%s", mensaje);
	while(strcmp(mensaje, "SALIR"))
	{
		lSend(memoria, mensaje, 1, sizeof(char)*10);
		scanf("%s", mensaje);
	}
	lSend(memoria, NULL, 2, 0);
}
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




