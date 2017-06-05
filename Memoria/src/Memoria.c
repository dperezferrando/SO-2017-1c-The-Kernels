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
char* memoria; // CACHO DE MEMORIA
char* memoriaUtilizable;
int cantPaginasAdmin; // PAGINAS OCUPADAS POR LA TABLA
configFile* config;

char* deserializarScript(void* data, int* pid, int* paginasTotales, int* tamanioArchivo);

int main(int argc, char** argsv) {

	pthread_t conexionKernel, esperarCPUS, consolaMemoria;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Memoria/Debug/memoria.conf", leerArchivoConfig, keys);
	arrancarMemoria();
	levantarSockets();
	puts("ESPERANDO AL KERNEL");
	int conexion = lAccept(kernel, KERNEL_ID);
	pthread_create(&conexionKernel, NULL, (void *) conexion_kernel, conexion);
	pthread_create(&esperarCPUS, NULL, (void *) esperar_cpus, NULL);
	pthread_create(&consolaMemoria, NULL, (void*) recibir_comandos, NULL);
	pthread_join(&consolaMemoria, NULL);
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
	entradaTabla* pointer = (entradaTabla*)memoria;
	cantPaginasAdmin = ceil((double)(sizeof(entradaTabla)*config->marcos)/(double)config->marco_size);
	printf("PAGINAS ADMINISTRATIVAS: %i | Tamanio entrada: %i\n", cantPaginasAdmin, sizeof(entradaTabla));
	int i;
	int j = 0;
	for(i = 0;i<config->marcos;i++)
	{
		pointer->frame = i;

		if(i<cantPaginasAdmin)
		{
			pointer->pagina = j;
			pointer->pid = -2;
			j++;
		}
		else
		{
			pointer->pagina = 1;
			pointer->pid = -1;
		}
		pointer++;
	}
	memoriaUtilizable = (char*)pointer;

}

void mostrarTablaPaginas()
{
	entradaTabla* pointer = (entradaTabla*)memoria;
	int i;
	for(i = 0;i<config->marcos;i++)
	{
		printf("Tabla: FRAME: %i | PID: %i | PAG: %i\n", pointer->frame, pointer->pid, pointer->pagina);
		pointer++;
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
				// MURIO LA CONEXION
				puts("MURIO EL KERNEL /FF");
				exit(EXIT_FAILURE);
				break;
			case 1:
			{
				// INICAR PROCESO
				int pid, cantidadPaginas;
				char* script = deserializarScript(mensaje->data, &pid, &cantidadPaginas, &(mensaje->header.tamanio));
				if(!sePuedenAsignarPaginas(pid, cantidadPaginas))
				{
					lSend(conexion, NULL,-2,0);
					break;
				}
				printf("ARRANCANDO PROCESO PID: %i | Paginas: %i\n", pid, cantidadPaginas);
				lSend(conexion, NULL, 104,0);
				inicializarPrograma(pid, cantidadPaginas, script, mensaje->header.tamanio);
				free(script);
				break;
			}
			case 2:
			{
				// ESCRIBIR EN MEMORIA [REPITE LOGICA NOSHIT]
				int pid;
				pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
				memcpy(&pid, mensaje->data, sizeof(int));
				memcpy(pedido, mensaje->data, sizeof(pedidoEscrituraMemoria));
				printf("CPU GUARDA INFO EN PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, pedido->valor);
				int* valorPuntero = &pedido->valor;
				escribirBytes(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, valorPuntero);
				char* linea = solicitarBytes(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
				int valor;
				memcpy(&valor, linea, sizeof(int));
				printf("TESTO - LEYENDO VALOR EN LA POSICION GUARDADA, VALOR = %i\n", valor);
				free(linea);
				free(pedido);
				break;
			}
			case 3:
			{
				//OTORGAR PAGINAS HEAP
				int pid, cantidadPaginas;
				memcpy(&pid, mensaje->data, sizeof(int));
				memcpy(&cantidadPaginas, mensaje->data, sizeof(int));
				if(!sePuedenAsignarPaginas(pid, cantidadPaginas))
				{
					lSend(conexion, NULL,-2,0);
					break;
				}
				lSend(conexion, NULL, 104,0);
				crearEntradas(pid, cantidadPaginas);
				printf("OTORGANDO PAGINAS HEAP A PROCESO: %i | Paginas: %i\n", pid, cantidadPaginas);
				//https://github.com/sisoputnfrba/foro/issues/652
				break;

			}
			case 9:
			{
				// FINALIZAR PROCESO
				int pid;
				memcpy(&pid, mensaje->data, sizeof(int));
				printf("EL KERNEL MANDO A AJUSTICIAR EL PROCESO: %i\n", pid);
				finalizarPrograma(pid);
			}
		}
		destruirMensaje(mensaje);

	}

}

void finalizarPrograma(int pid)
{
	int pag = 0;
	entradaTabla* pointer;
	do
	{
		pointer = obtenerEntradaDe(pid, pag);
		pointer->pid = -1;
		pointer->pagina = -1;
		pag++;
		pointer++;
	}
	while(pointer->pid == pid);

}

char* leerCaracteresEntrantes() {
	int i, caracterLeido;
	char* cadena = malloc(30);
	for(i = 0; (caracterLeido= getchar()) != '\n'; i++)
		cadena[i] = caracterLeido;
	cadena[i] = '\0';
	return cadena;
}

void recibir_comandos()
{
	while(1)
	{
		char* entrada = leerCaracteresEntrantes();
		char** comando = string_split(entrada, " ");
		printf("COMANDO: %s ARG: %s\n", comando[0], comando[1]);
		if(!strcmp(comando[0], "dump"))
		{
			if(!strcmp(comando[1], "estructuras"))
				mostrarTablaPaginas();
			else if(!strcmp(comando[1], "cache"))
				puts("TODO");
			else if(!strcmp(comando[1], "contenido"))
				puts("TODO");
		}
		else if(!strcmp(comando[0], "retardo"))
		{
			config->retardo_memoria = atoi(comando[1]);
			printf("NUEVO RETARDO SETEADO: %ims\n", config->retardo_memoria);
		}
		else if(!strcmp(comando[0], "flush"))
			puts("TODO");
		else if(!strcmp(comando[0], "size"))
		{
			if(!strcmp(comando[1], "memory"))
			{
				printf("CANTIDAD TOTAL DE FRAMES: %i | OCUPADOS: %i (%i ADMIN) | LIBRES: %i\n", config->marcos, cantidadFramesOcupados(), cantPaginasAdmin, cantidadFramesLibres());
			}
			else
			{
				int pid = atoi(comando[1]);
				int cantPaginas = tamanioProceso(pid);
				printf("CANTIDAD DE PAGINAS PROCEO PID %i: %i\n", pid, cantPaginas);
			}
		}

		free(comando[0]);
		free(comando[1]);
		free(entrada);

	}
}

int tamanioProceso(int pid)
{
	entradaTabla* puntero = (entradaTabla*)memoria+cantPaginasAdmin;
	int contador = 0;
	while(puntero->frame != (config->marcos-1))
	{
		if(puntero->pid == pid)
			contador++;
		puntero++;
	}
	return contador;
}

int cantidadFramesOcupados()
{
	entradaTabla* puntero = (entradaTabla*)memoria+cantPaginasAdmin;
	int contador = cantPaginasAdmin;
	while(puntero->frame != (config->marcos-1))
	{
		if(puntero->pid > -1)
			contador++;
		puntero++;
	}

	return contador;
}

int cantidadFramesLibres()
{
	return config->marcos - cantidadFramesOcupados();
}

char* deserializarScript(void* data, int* pid, int* paginasTotales, int* tamanioArchivo)
{
	*tamanioArchivo = *tamanioArchivo-sizeof(int);
	char* archivo = malloc(*tamanioArchivo);
	memcpy(pid, data, sizeof(int));
	memcpy(paginasTotales, data + sizeof(int), sizeof(int));
	memcpy(archivo, data+sizeof(int)*2, *tamanioArchivo);
	return archivo;
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
		int pidActual;
		switch(mensaje->header.tipoOperacion)
		{
			case -1:
				puts("DESCONEXION ABRUPTA");
				conectado = 0;
				break;

			case 1:
				memcpy(&pidActual, mensaje->data, sizeof(int));
				printf("CPU CAMBIO A PROCESO PID: %i\n", pidActual); // TESTING

				break;

			case 2:
			{
				// LEER
				posicionEnMemoria* posicion = malloc(sizeof(posicionEnMemoria));
				memcpy(posicion, mensaje->data, sizeof(posicionEnMemoria));
				printf("CPU QUIERE LEER PAG: %i | OFFSET: %i | SIZE: %i\n", posicion->pagina, posicion->offset, posicion->size);
				char* linea = solicitarBytes(pidActual, posicion->pagina, posicion->offset, posicion->size);
				lSend(conexion, linea, 3, posicion->size);
				free(linea);
				free(posicion);
				break;
			}

			case 3:
			{
				// ESCRIBIR EN MEMORIA
				pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
				memcpy(pedido, mensaje->data, sizeof(pedidoEscrituraMemoria));
				printf("CPU GUARDA INFO EN PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, pedido->valor);
				int* valorPuntero = &pedido->valor;
				escribirBytes(pidActual, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, valorPuntero);
				char* linea = solicitarBytes(pidActual, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
				int valor;
				memcpy(&valor, linea, sizeof(int));
				printf("TESTO - LEYENDO VALOR EN LA POSICION GUARDADA, VALOR = %i\n", valor);
				free(linea);
				free(pedido);
				break;
			}

			case 4:
				puts("EL CPU SE TOMA EL PALO");
				conectado = 0;
				break;


		}
		destruirMensaje(mensaje);
	}
	close(conexion);

}


entradaTabla* obtenerEntradaAproximada(int pid, int pagina)
{
	int frameAprox = bestHashingAlgorithmInTheFuckingWorld(pid, pagina);
	entradaTabla* pointer = (entradaTabla*)memoria+frameAprox;
	return pointer;
}

entradaTabla* obtenerEntradaDe(int pid, int pagina)
{
	entradaTabla* pointer = obtenerEntradaAproximada(pid, pagina);
	while(pointer->pid != pid || pointer->pagina != pagina)
		pointer++;
	return pointer;
}

char* obtenerPosicionAOperar(int pid, int pagina, int offset)
{
	entradaTabla* pointer = obtenerEntradaDe(pid, pagina);
	char* punteroAFrame = memoria + (pointer->frame*config->marco_size)+offset;
	return punteroAFrame;
}

char* solicitarBytes(int pid, int pagina, int offset, int tamanio)
{
	char* punteroAFrame = obtenerPosicionAOperar(pid, pagina, offset);
	char* data = malloc(tamanio);
	memcpy(data, punteroAFrame, tamanio);
	return data;
}

int escribirBytes(int pid, int pagina, int offset, int tamanio, void* buffer)
{
	if(tamanio+offset > config->marco_size)
		return 0;
	char* punteroAFrame = obtenerPosicionAOperar(pid, pagina, offset);
	memcpy(punteroAFrame, buffer, tamanio);
	return 1;
}

int bestHashingAlgorithmInTheFuckingWorld(int pid, int pagina)
{
	return cantPaginasAdmin; // TO DO OBV
}

void inicializarPrograma(int pid, int cantidadPaginas, char* archivo, int tamanio)
{
	crearEntradas(pid, cantidadPaginas);
	escribirCodigoPrograma(pid, archivo, tamanio);

}

void escribirCodigoPrograma(int pid, char* archivo, int tamanio)
{
	int paginas = ceil((double)tamanio/(double)config->marco_size);
	int i;
	if(paginas == 1)
		escribirBytes(pid, 0, 0, tamanio, archivo);
	else
	{
		for(i = 0;i<paginas-1;i++)
		{
			escribirBytes(pid, i, 0, config->marco_size, archivo);
			archivo += config->marco_size;
		}
		escribirBytes(pid, paginas-1, 0, tamanio % config->marco_size, archivo);
	}
}


int sePuedenAsignarPaginas(int pid, int cantidadDePaginas)
{
	entradaTabla* comienzo = obtenerEntradaAproximada(pid, 0);
	entradaTabla* pointer = comienzo;
	int i =0, j = comienzo->frame;
	do
	{
		if(pointer->pid == -1)
		{
			i++;
		}
		pointer++;
		j++;
		if(j == config->marcos)
		{
			j=0;
			pointer = (entradaTabla*)memoria;


		}
	/*	puts("-----------");
		printf("FRAME POINTER: %i\n", pointer->frame);
		printf("FRAME UTILIZABLE: %i\n", ((entradaTabla*)memoriaUtilizable)->frame);
		printf("FRAME COMIENZO: %i\n", comienzo->frame);
		puts("-----------");*/

	}
	while(i < cantidadDePaginas && pointer != comienzo);
	return i == cantidadDePaginas;
}

void crearEntradas(int pid, int cantidadPaginas)
{
	entradaTabla* pointer = obtenerEntradaAproximada(pid, 0);
	int i =0;
	while(i<cantidadPaginas)
	{
		if(pointer->pid == -1)
		{
			pointer->pid = pid;
			pointer->pagina = i;
			i++;
		}
		pointer++;
	}

}

/*void recibirComandos()
{
	char comando[]
	puts("PROCESO MEMORIA - INGRESA UN COMANDO");
	scanf("%s", comando);
}
*/
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
