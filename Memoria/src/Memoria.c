/*
 ============================================================================
 Name        : Memoria.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Memoria.h"

// VARIABLES GLOBALES PORQUE ACA VALE TODO VIEJA
int kernel, cpu; // SOCKETS
void* memoria; // CACHO DE MEMORIA
configFile* config;

int main(int argc, char** argsv) {

	pthread_t conexionKernel, esperarCPUS;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Memoria/Debug/memoria.conf", leerArchivoConfig, keys);
	levantarSockets();
	puts("ESPERANDO AL KERNEL");
	int conexion = lAccept(kernel, KERNEL_ID);
	pthread_create(&conexionKernel, NULL, (void *) conexion_kernel, conexion);
	pthread_create(&esperarCPUS, NULL, (void *) esperar_cpus, NULL);
	arrancarMemoria();
	pthread_join(conexionKernel, NULL);
	pthread_join(esperarCPUS, NULL);
	free(config);
	free(memoria);
	close(kernel);
	close(cpu);
	return EXIT_SUCCESS;
}

void arrancarMemoria()
{
	memoria = malloc(config->marco_size*config->marcos);
	if(memoria == NULL)
	{
		puts("CAPO, una cosa nomas queria decirte: tu pc no tiene RAM, malloc fallo, salu2");
		exit(EXIT_FAILURE);
	}
}
void levantarSockets()
{
	kernel = getBindedSocket("127.0.0.1", config->puerto_kernel);
	cpu = getBindedSocket("127.0.0.1", config->puerto_cpu);
	lListen(kernel, 5);
	lListen(cpu, 5);
}

void conexion_kernel(int conexion)
{

	puts("KERNEL CONECTADO - ESPERANDO MENSAJE <3");
	while(1)
	{
		Mensaje* mensaje = lRecv(conexion);
		switch(mensaje->header.tipoOperacion)
		{
			case -1:
				puts("MURIO EL KERNEL /FF");
				exit(EXIT_FAILURE);
				break;
			case 1:
				puts("ARRANCAR PROCESO");
				int pid;
				int tamanioArchivo = mensaje->header.tamanio-sizeof(int);
				char* archivo = malloc(tamanioArchivo);
				memcpy(&pid, mensaje->data, sizeof(int));
				memcpy(archivo, mensaje->data+sizeof(int), tamanioArchivo);
				printf("Archivo recibido: %s\n", archivo);
				inicializarPrograma(pid, 1, archivo, tamanioArchivo);
				free(archivo);
				char* test = malloc(tamanioArchivo);
				memcpy(test, memoria, tamanioArchivo);
				printf("Archivo leido desde memoria: %s\n", test);
				//lSend(kernel, 1,2, sizeof(int));
				break;
		}
		destruirMensaje(mensaje);

	}

}

void esperar_cpus()
{
	while(1)
	{
		int conexion = lAccept(cpu, CPU_ID);
		printf("NUEVA CONEXION CPU\n");
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, (void *) conexion_cpu, conexion);

	}
}

void conexion_cpu(int conexion)
{
	int conectado = 1;
	while(conectado)
	{
		printf("Procesando CPU SOCKET: %i\n", conexion);
		Mensaje* mensaje = lRecv(conexion);
		switch(mensaje->header.tipoOperacion)
		{
			case -1:
				puts("DESCONEXION ABRUPTA");
				conectado = 0;
				break;

			case 1:
				printf("MENSAJE CPU: %s\n", mensaje->data); // TESTING
				break;

			case 2:
				puts("EL CPU SE TOMA EL PALO");
				conectado = 0;
				break;

		}
		destruirMensaje(mensaje);
	}
	close(conexion);

}

void escribirBytes(int pid, int pagina, int offset, int tamanio, void* buffer)
{
	memcpy(memoria, buffer, tamanio);
}



void inicializarPrograma(int pid, int cantidadPaginas, char* archivo, int tamanio)
{
	crearEntrada(pid, cantidadPaginas);
	escribirBytes(pid, 1, 0, tamanio, archivo);
}

void crearEntrada(int pid, int cantidadPaginas)
{
	entradaTabla unaEntrada;
	unaEntrada.frame = 0;
	unaEntrada.pagina = 0;
	unaEntrada.pid = pid;
	memcpy(memoria, &unaEntrada, sizeof(unaEntrada));
	memoria += sizeof(unaEntrada);
}

void imprimirConfig(configFile* config)
{
	puts("--------PROCESO MEMORIA--------");
	printf("ESCUCHANDO EN PUERTO KERNEL: %s | ESCUCHANDO EN PUERTO CPU: %s \n", config->puerto_kernel, config->puerto_cpu);
	printf("CANTIDAD DE MARCOS: %i | TAMAÑO DE MARCOS: %i\n", config->marcos, config->marco_size);
	printf("ENTRADAS CACHE: %i | CACHE X PROC: %i | ALGORITMO REEMPLAZO: %s\n", config->entradas_cache, config->cache_x_proc, config->reemplazo_cache);
	printf("RETARDO DE MEMORIA: %i\n", config->retardo_memoria);
	puts("--------PROCESO MEMORIA--------");
}

configFile* leerArchivoConfig(t_config* configHandler)
{
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->puerto_kernel, config_get_string_value(configHandler, "PUERTO_KERNEL"));
	strcpy(config->puerto_cpu, config_get_string_value(configHandler, "PUERTO_CPU"));
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
