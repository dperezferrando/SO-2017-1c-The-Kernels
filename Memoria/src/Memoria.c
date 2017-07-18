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
t_list* cache; // CACHO DE CACHE
char* memoriaUtilizable;
int cantPaginasAdmin; // PAGINAS OCUPADAS POR LA TABLA
configFile* config;
pthread_t conexionKernel, esperarCPUS, consolaMemoria;
pthread_mutex_t memoriaSem;
pthread_mutex_t cacheSem;
pthread_mutex_t retardoSem;
t_log* logFile;




int main(int argc, char** argsv) {

	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Memoria/Debug/memoria.conf", leerArchivoConfig, keys);
	levantarLog();
	arrancarMemoria();
	levantarSockets();
	puts("[MEMORIA]: ESPERANDO AL KERNEL");
	int conexion = lAccept(kernel, KERNEL_ID);
	lSend(conexion, &config->marco_size, 104, sizeof(int));
	pthread_create(&conexionKernel, NULL, (void *) conexion_kernel, conexion);
	pthread_create(&esperarCPUS, NULL, (void *) esperar_cpus, NULL);
	pthread_create(&consolaMemoria, NULL, (void*) recibir_comandos, NULL);
	pthread_join(consolaMemoria, NULL);
	pthread_join(conexionKernel, NULL);
	pthread_join(esperarCPUS, NULL);
//	morirElegantemente();
	return EXIT_SUCCESS;
}

void levantarLog()
{
	if(fopen(config->log, "r") != NULL)
		remove(config->log);
	logFile = log_create(config->log, "MEMORIA", 0, 1);
}

void arrancarMemoria()
{
	memoria = malloc(config->marco_size*config->marcos);
	if(memoria == NULL)
	{
		log_info(logFile,"[ERROR]: CAPO, una cosa nomas queria decirte: tu pc no tiene RAM, malloc fallo, salu2");
		exit(EXIT_FAILURE);
	}
	entradaTabla* pointer = (entradaTabla*)memoria;
	cantPaginasAdmin = ceil((double)(sizeof(entradaTabla)*config->marcos)/(double)config->marco_size);
	printf("[MEMORIA]:PAGINAS ADMINISTRATIVAS: %i | Tamanio entrada: %i\n", cantPaginasAdmin, sizeof(entradaTabla));
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
			pointer->pagina = -1;
			pointer->pid = -1;
		}
		pointer++;
	}
	memoriaUtilizable = (char*)pointer;
	if(config->entradas_cache != 0)
	{
		cache = list_create();
		pthread_mutex_init(&cacheSem, NULL);
	}
	pthread_mutex_init(&memoriaSem, NULL);

}


void mostrarTablaPaginas()
{
	entradaTabla* pointer = (entradaTabla*)memoria;
	int i;
	for(i = 0;i<config->marcos;i++)
	{
		printf("[TABLA]: FRAME: %i | PID: %i | PAG: %i\n", pointer->frame, pointer->pid, pointer->pagina);
		pointer++;
	}
}

void levantarSockets()
{
	kernel = getBindedSocket(config->ip_propia, config->puerto_kernel);
	cpu = getBindedSocket(config->ip_propia, config->puerto_cpu);
	lListen(kernel, 5);
	lListen(cpu, 5);
}

void conexion_kernel(int conexion)
{
	puts("[MEMORIA]: KERNEL CONECTADO - ESPERANDO MENSAJES <3");
	while(1)
	{
		Mensaje* mensaje = lRecv(conexion);
		switch(mensaje->header.tipoOperacion)
		{
			case -1:
				// MURIO LA CONEXION
				puts("MURIO EL KERNEL /FF");
				morirElegantemente();
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
				log_info(logFile,"[ARRANCAR PROCESO]: PID: %i | Paginas: %i\n", pid, cantidadPaginas);
				lSend(conexion, NULL, 104,0);
				inicializarPrograma(pid, cantidadPaginas, script, mensaje->header.tamanio);
				free(script);
				break;
			}
			case 2:
			{
				// ESCRIBIR EN MEMORIA
				log_info(logFile,"[HEAP]: KERNEL PIDE ESCRIBIR");
				int pid;
				pedidoEscrituraDelKernel* pedido = malloc(sizeof(pedidoEscrituraMemoria));
				memcpy(&pid,mensaje->data,sizeof(int));
				memcpy(&pedido->posicion.pagina,mensaje->data+sizeof(int),sizeof(int));
				memcpy(&pedido->posicion.offset,mensaje->data+(sizeof(int)*2),sizeof(int));
				memcpy(&pedido->posicion.size,mensaje->data+(sizeof(int)*3),sizeof(int));
				pedido->data = malloc(pedido->posicion.size);
				memcpy(pedido->data, mensaje->data+(sizeof(int)*4),pedido->posicion.size);
				log_info(logFile,"[HEAP]: KERNEL PIDE ESCRIBIR PID: %i | PAG: %i | OFFSET: %i | SIZE: %i\n", pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
				escribirDondeCorresponda(pid, pedido);

				free(pedido);
				break;
			}
			case 3:
			{
				//OTORGAR PAGINAS HEAP

				int pid;
				memcpy(&pid, mensaje->data, sizeof(int));
				log_info(logFile,"[HEAP]: KERNEL PIDE PAGINA PARA PID: %i\n", pid);
				if(!sePuedenAsignarPaginas(pid, 1))
				{
					lSend(conexion, NULL,-2,0);
					break;
				}
				int paginaAsignada = damePaginaHeap(pid);
				log_info(logFile,"[HEAP]: PAGINA ASIGNADA: %i\n", paginaAsignada);
				lSend(conexion, &paginaAsignada, 104,sizeof(int));
				crearEntradas(pid, 1, paginaAsignada);
				log_info(logFile,"[OTORGAR PAGINAS HEAP]: PID: %i | Paginas: %i\n", pid, 1);
				//https://github.com/sisoputnfrba/foro/issues/652
				break;

			}
			case 4:
			{
				// LIBERAR PAGINA HEAP
				int pid, pagina;
				memcpy(&pid, mensaje->data, sizeof(int));
				memcpy(&pagina, mensaje->data+sizeof(int), sizeof(int));
				entradaTabla* pointer;
				pointer = obtenerEntradaDe(pid, pagina);
				pointer->pagina = -1;
				pointer->pid = -1;
				break;
			}
			case 5:
			{
				// LEER
				int pid;
				posicionEnMemoria* posicion = malloc(sizeof(posicionEnMemoria));
				memcpy(&pid,mensaje->data,sizeof(int));
				memcpy(&posicion->pagina,mensaje->data+sizeof(int),sizeof(int));
				memcpy(&posicion->offset,mensaje->data+(sizeof(int)*2),sizeof(int));
				memcpy(&posicion->size,mensaje->data+(sizeof(int)*3),sizeof(int));
				char* lectura = leerDondeCorresponda(pid, posicion);
				lSend(conexion, lectura, 104, posicion->size);
				free(posicion);
				free(lectura);
				break;

			}
			case 9:
			{
				// FINALIZAR PROCESO
				int pid;
				memcpy(&pid, mensaje->data, sizeof(int));
				log_info(logFile,"[AJUSTICIANDO PROCESO]: PID: %i\n", pid);
				finalizarPrograma(pid);
			}
		}
		destruirMensaje(mensaje);

	}

}

bool frameValido(int frame)
{
	return frame > 0 && frame < config->marcos;
}
void finalizarPrograma(int pid)
{
	int pag = 0, i = 0;
	entradaTabla* pointer = obtenerEntradaDe(pid, pag);
	int tamanio = tamanioProceso(pid);
	while(pag < tamanio)
	{
		if(pointer != NULL)
		{
			pointer->pid = -1;
			pointer->pagina = -1;
			pag++;
		}
		i++;
		pointer = obtenerEntradaDe(pid, i);
	}
//	while(pointer != NULL);//pointer->pid == pid && frameValido(pointer->frame));
	bool pidDistintoA(entradaCache* entrada)
	{
		return entrada->pid != pid;
	}
	bool pidIgualA(entradaCache* entrada)
	{
		return entrada->pid == pid;
	}
	if(config->entradas_cache != 0)
	{
		pthread_mutex_lock(&cacheSem);
		t_list* aux= list_filter(cache, &pidDistintoA);
		t_list* losotros = list_filter(cache, &pidIgualA);
		list_destroy_and_destroy_elements(losotros, &destruirEntradaCache);
		list_destroy(cache);
		cache = aux;
		pthread_mutex_unlock(&cacheSem);
	}

}

void agregarACache(int pid, int pagina)
{
	char* puntero = obtenerPosicionAOperar(pid, pagina, 0);
	entradaCache* entrada = malloc(sizeof(entradaCache));
	entrada->pagina = pagina;
	entrada->pid = pid;
	entrada->data = malloc(config->marco_size);
	memcpy(entrada->data, puntero, config->marco_size);
	int cantEntradasProceso = cantidadEntradasCacheDelProceso(pid);
//	pthread_mutex_lock(&cacheSem);
	int size = list_size(cache);
//	pthread_mutex_unlock(&cacheSem);
	if( size < config->entradas_cache && cantEntradasProceso < config->cache_x_proc)
	{
//		pthread_mutex_lock(&cacheSem);
		list_add_in_index(cache, 0, entrada);
//		pthread_mutex_unlock(&cacheSem);
	}
	else
		reemplazarLRU(entrada);
}

int cantidadEntradasCacheDelProceso(int pid)
{
	bool esEntradaDelProceso(entradaCache* entrada)
	{
		return entrada->pid == pid;
	}
	return list_count_satisfying(cache, esEntradaDelProceso);
}

void destruirEntradaCache(entradaCache* entrada)
{
	free(entrada->data);
	free(entrada);
}

void reemplazarLRU(entradaCache* entrada)
{
//	pthread_mutex_lock(&cacheSem);
	int tamanioLista = list_size(cache);
//	pthread_mutex_unlock(&cacheSem);
	if(cantidadEntradasCacheDelProceso(entrada->pid) < config->cache_x_proc && tamanioLista >= config->entradas_cache)
	{
		int indexMenosUsado = tamanioLista-1;
//		pthread_mutex_lock(&cacheSem);
		list_remove_and_destroy_element(cache, indexMenosUsado, destruirEntradaCache);
		list_add_in_index(cache, 0, entrada);
//		pthread_mutex_unlock(&cacheSem);
	}
	else
	{
		int index = 0, i = 0;
		void calcularIndexMenosUsado(entradaCache* unaEntrada)
		{
			if(entrada->pid == unaEntrada->pid)
				i++;
			if(i<config->cache_x_proc)
				index++;
		}
//		pthread_mutex_lock(&cacheSem);
		list_iterate(cache, calcularIndexMenosUsado);
		list_remove_and_destroy_element(cache, index, destruirEntradaCache);
		list_add_in_index(cache, 0, entrada);
//		pthread_mutex_unlock(&cacheSem);
	}
}

bool existeEnCache(int pid, int pagina)
{
	bool mismoPIDyPagina(entradaCache* unaEntrada)
	{
		return pid == unaEntrada->pid && pagina == unaEntrada->pagina;
	}
	return list_any_satisfy(cache, mismoPIDyPagina);
}

int escribirBytesCache(int pid, int pagina, int offset, int tamanio, void* buffer)
{
	if(tamanio+offset > config->marco_size)
		return 0;
	char* punteroAFrame = obtenerPosicionAOperarEnCache(pid, pagina, offset);
	memcpy(punteroAFrame, buffer, tamanio);
	escribirBytes(pid, pagina, offset, tamanio, buffer);
	return 1;
}

char* obtenerPosicionAOperarEnCache(int pid, int pagina, int offset)
{
	bool mismoPIDyPagina(entradaCache* unaEntrada)
	{
		return pid == unaEntrada->pid && pagina == unaEntrada->pagina;
	}
	entradaCache* entrada = list_remove_by_condition(cache, mismoPIDyPagina);
	char* punteroAFrame = entrada->data+offset;
	list_add_in_index(cache, 0, entrada);
	return punteroAFrame;
}

char* solicitarBytesACache(int pid, int pagina, int offset, int tamanio)
{
	char* punteroAFrame = obtenerPosicionAOperarEnCache(pid, pagina, offset);
	char* data = malloc(tamanio);
	memcpy(data, punteroAFrame, tamanio);
	return data;
}

void mostrarEntradaCache(entradaCache* entrada)
{

	printf("[CACHE]: PID: %i | PAGINA: %i | CONTENIDO: %s\n", entrada->pid, entrada->pagina, entrada->data);
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
		log_info(logFile,"COMANDO: %s ARG: %s\n", comando[0], comando[1]);
		if(!strcmp(comando[0], "dump"))
		{
			if(comando[1] == NULL)
				puts("COMANDO INVALIDO");
			else if(!strcmp(comando[1], "estructuras"))
				mostrarTablaPaginas();
			else if(!strcmp(comando[1], "cache"))
			{
				if(config->entradas_cache == 0)
					puts("[CACHE]: LA CACHE ESTA DESACTIVADA");
				else
				{
					puts("----------------DUMP CACHE----------------");
					printf("ENTRADAS MAXIMA CACHE: %i | ENTRADAS MAXIMAS POR PROCESO: %i\n", config->entradas_cache, config->cache_x_proc);
					pthread_mutex_lock(&cacheSem);
					list_iterate(cache, mostrarEntradaCache);
					pthread_mutex_unlock(&cacheSem);
					puts("----------------DUMP CACHE----------------");
				}
			}
			else if(!strcmp(comando[1], "contenido"))
				mostrarDatosProcesos();
		}
		else if(!strcmp(comando[0], "retardo"))
		{

			pthread_mutex_lock(&retardoSem);
			config->retardo_memoria = atoi(comando[1]);
			pthread_mutex_unlock(&retardoSem);
			printf("[MEMORIA]: NUEVO RETARDO SETEADO: %ims\n", config->retardo_memoria);
		}
		else if(!strcmp(comando[0], "flush"))
		{
			pthread_mutex_lock(&cacheSem);
			list_clean_and_destroy_elements(cache, destruirEntradaCache);
			pthread_mutex_unlock(&cacheSem);
		}
		else if(!strcmp(comando[0], "size"))
		{
			if(!strcmp(comando[1], "memory"))
			{
				printf("[MEMORIA]: CANTIDAD TOTAL DE FRAMES: %i | OCUPADOS: %i (%i ADMIN) | LIBRES: %i\n", config->marcos, cantidadFramesOcupados(), cantPaginasAdmin, cantidadFramesLibres());
			}
			else
			{
				int pid = atoi(comando[1]);
				int cantPaginas = tamanioProceso(pid);
				printf("[TAMANIO PROCESO]: PID %i: %i\n", pid, cantPaginas);
			}
		}
		else
			puts("COMANDO INVALIDO");
		free(comando[0]);
		free(comando[1]);
		free(comando);
		free(entrada);
		//pthread_exit(NULL);

	}
}

int tamanioProceso(int pid)
{
	entradaTabla* puntero = (entradaTabla*)memoria + cantPaginasAdmin;
	int i = 0;
	do
	{
		if(puntero->pid == pid)
			i++;
		puntero++;
	}
	while(frameValido(puntero->frame));
	return i;
/*	entradaTabla* puntero = (entradaTabla*)memoria+cantPaginasAdmin;
	int contador = 0;
	while(puntero->frame != (config->marcos-1))
	{
		if(puntero->pid == pid)
			contador++;
		puntero++;
	}
	return contador;*/
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

void mostrarDatosProcesos()
{
	entradaTabla* puntero = (entradaTabla*)memoria+cantPaginasAdmin;
	do
	{
		if(puntero->pid >= 0)
		{
			char* punteroAFrame = obtenerPosicionAOperar(puntero->pid, puntero->pagina, 0);
			char* data = malloc(config->marco_size);
			memcpy(data, punteroAFrame, config->marco_size);
			printf("[DATOS PROCESO]: FRAME: %i | PID: %i | PAG: %i | DATA:\n %s\n", puntero->frame, puntero->pid, puntero->pagina, data);
			free(data);
		}
		puntero++;
	}
	while(frameValido(puntero->frame));
}

int cantidadFramesLibres()
{
	return config->marcos - cantidadFramesOcupados();
}

char* deserializarScript(void* data, int* pid, int* paginasTotales, int* tamanioArchivo)
{
	*tamanioArchivo = *tamanioArchivo-(sizeof(int)*2);
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
		log_info(logFile,"[MEMORIA]: NUEVA CONEXION CPU\n");
		pthread_t conexionCPU;
		pthread_create(&conexionCPU, NULL, (void *) conexion_cpu, conexion);

	}
}

void conexion_cpu(int conexion)
{

	int conectado = 1;
	while(conectado)
	{
		log_info(logFile,"[MEMORIA]: PROCESANDO CPU SOCKET: %i\n", conexion);
		Mensaje* mensaje = lRecv(conexion);
		int pidActual;
		switch(mensaje->header.tipoOperacion)
		{
			case -1:
				log_info(logFile,"[CPU %i]: DESCONEXION ABRUPTA\n", conexion);
				conectado = 0;
				break;

			case 1:
				memcpy(&pidActual, mensaje->data, sizeof(int));
				log_info(logFile,"[CPU %i]: CAMBIO A PROCESO PID: %i\n", conexion, pidActual); // TESTING

				break;

			case 2:
			{
				// LEER
				posicionEnMemoria* posicion = malloc(sizeof(posicionEnMemoria));
				memcpy(posicion, mensaje->data, sizeof(posicionEnMemoria));
				if(!pedidoValido(pidActual, posicion))
					lSend(conexion, NULL, -5, 0);
				else {
					log_info(logFile,"[CPU %i]: LEER PAG: %i | OFFSET: %i | SIZE: %i\n", conexion, posicion->pagina, posicion->offset, posicion->size);
					char* linea = leerDondeCorresponda(pidActual, posicion);
					lSend(conexion, linea, 2, posicion->size);
					free(linea);
				}
				free(posicion);
				break;
			}

			case 3:
			{
				// ESCRIBIR EN MEMORIA
				pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
				memcpy(&pedido->posicion, mensaje->data, sizeof(posicionEnMemoria));
				pedido->valor = malloc(pedido->posicion.size);
				memcpy(pedido->valor, mensaje->data+sizeof(posicionEnMemoria),pedido->posicion.size);
				if(!pedidoValido(pidActual, &pedido->posicion))
					lSend(conexion, NULL, -5, 0);
				else
				{
					log_info(logFile,"[CPU %i]: GUARDAR INFO EN PAG: %i | OFFSET: %i | SIZE: %i\n", conexion, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
					escribirDondeCorresponda(pidActual, pedido);
					lSend(conexion,NULL, 104, 0);
				}
				free(pedido->valor);
				free(pedido);
				break;
			}

			case 4:
				log_info(logFile,"[CPU %i]: EL CPU SE TOMA EL PALO\n", conexion);
				conectado = 0;
				break;


		}
		destruirMensaje(mensaje);
	}
	close(conexion);
	pthread_exit(NULL);

}

int pedidoValido(int pid, posicionEnMemoria* posicion)
{
	return obtenerEntradaDe(pid, posicion->pagina) != NULL;
}

char* leerDondeCorresponda(int pid, posicionEnMemoria* posicion)
{
	pthread_mutex_lock(&cacheSem);
	char* linea;
	if(config->entradas_cache > 0 && existeEnCache(pid, posicion->pagina))
	{
		log_info(logFile,"[CACHE]: EXISTE EN CACHE");
		linea = solicitarBytesACache(pid, posicion->pagina, posicion->offset, posicion->size);
	}
	else
	{
		log_info(logFile,"[CACHE]: NO EXISTE EN CACHE - RETARDO");
		pthread_mutex_lock(&retardoSem);
		usleep(1000*config->retardo_memoria);
		pthread_mutex_unlock(&retardoSem);
		log_info(logFile,"[CACHE]: SLEEP LISTO, SOLICITO Y AGREGO A CACHE");
		linea = solicitarBytes(pid, posicion->pagina, posicion->offset, posicion->size);
		if(config->entradas_cache != 0)
			agregarACache(pid, posicion->pagina);
	}
	pthread_mutex_unlock(&cacheSem);
	return linea;
}

void escribirDondeCorresponda(int pid, pedidoEscrituraMemoria* pedido)
{
	pthread_mutex_lock(&cacheSem);
	if(config->entradas_cache > 0 && existeEnCache(pid, pedido->posicion.pagina))
	{
		log_info(logFile,"[CACHE]: EXISTE EN CACHE PAGINA: %i\n", pedido->posicion.pagina);
		escribirBytesCache(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, pedido->valor);
	//	linea = solicitarBytes(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
	}
	else
	{
		log_info(logFile,"[CACHE]: NO EXISTE EN CACHE - RETARDO");
		pthread_mutex_lock(&retardoSem);
		usleep(1000*config->retardo_memoria);
		pthread_mutex_unlock(&retardoSem);
		log_info(logFile,"[CACHE]: SLEEP LISTO, SOLICITO Y AGREGO A CACHE");
		int estado = escribirBytes(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size, pedido->valor);
		if(estado == 0)
			log_info(logFile,"[CACHE]: OFFSET MAYOR A MARCO (ESTO DEBERIA SER UNREACHABLE, SI LO VES SE ROMPIO TODO)");
		else if(estado == -1)
			log_info(logFile, "[CACHE]: NO EXISTE LA PAGINA EN MEMORIA - POSIBLEMENTE EL PROCESO HAYA SIDO AJUSTICIADO");
		else
		{
			log_info(logFile,"[CACHE]: SE ESCRIBIO EN MEMORIA - AGREGANDO A CACHE LA PAGINA");
			if(config->entradas_cache != 0)
			{
				agregarACache(pid, pedido->posicion.pagina);
		//		linea = solicitarBytesACache(pid, pedido->posicion.pagina, pedido->posicion.offset, pedido->posicion.size);
			}
		}
	}
	pthread_mutex_unlock(&cacheSem);

}


entradaTabla* obtenerEntradaAproximada(int pid, int pagina)
{
	int frameAprox = bestHashingAlgorithmInTheFuckingWorld(pid, pagina);
	pthread_mutex_lock(&memoriaSem);
	entradaTabla* pointer = (entradaTabla*)memoria+frameAprox;
	pthread_mutex_unlock(&memoriaSem);
	return pointer;
}

int damePaginaHeap(int pid)
{
	int i = 0;
	while(obtenerEntradaDe(pid, i) != NULL)
		i++;
	return i;
}

entradaTabla* obtenerEntradaDe(int pid, int pagina)
{
	entradaTabla* pointer = obtenerEntradaAproximada(pid, pagina);
	entradaTabla* aux = pointer;
	while((pointer->pid != pid || pointer->pagina != pagina) && frameValido(pointer->frame))
		pointer++;

	if(pointer->pid != pid || pointer->pagina != pagina)
	{
		pointer = (entradaTabla*)memoria+cantPaginasAdmin;
		while(pointer != aux)
		{
			if(pointer->pid == pid && pointer->pagina == pagina)
				return pointer;
			pointer++;
		}
		return NULL;
	}
	else
		return pointer;
}

char* obtenerPosicionAOperar(int pid, int pagina, int offset)
{
	entradaTabla* pointer = obtenerEntradaDe(pid, pagina);
	if(pointer == NULL)
		return NULL;
	pthread_mutex_lock(&memoriaSem);
	char* punteroAFrame = memoria + (pointer->frame*config->marco_size)+offset;
	pthread_mutex_unlock(&memoriaSem);
	return punteroAFrame;
}

char* solicitarBytes(int pid, int pagina, int offset, int tamanio)
{
	char* punteroAFrame = obtenerPosicionAOperar(pid, pagina, offset);
	char* data = malloc(tamanio);
	pthread_mutex_lock(&memoriaSem);
	memcpy(data, punteroAFrame, tamanio);
	pthread_mutex_unlock(&memoriaSem);
	return data;
}

int escribirBytes(int pid, int pagina, int offset, int tamanio, void* buffer)
{
	if(tamanio+offset > config->marco_size)
		return 0;
	char* punteroAFrame = obtenerPosicionAOperar(pid, pagina, offset);
	if(punteroAFrame == NULL)
		return -1;
	pthread_mutex_lock(&memoriaSem);
	memcpy(punteroAFrame, buffer, tamanio);
	pthread_mutex_unlock(&memoriaSem);
	return 1;
}

int bestHashingAlgorithmInTheFuckingWorld(int pid, int pagina)
{
	char str1[20];
	char str2[20];
	sprintf(str1, "%d", pid);
	sprintf(str2, "%d", pagina);
	strcat(str1, str2);
	int aux = atoi(str1);
	int marcosNoAdmin = (config->marcos-1) - cantPaginasAdmin;
	int frame = ( aux % marcosNoAdmin) + cantPaginasAdmin;
	return frame;
}

void inicializarPrograma(int pid, int cantidadPaginas, char* archivo, int tamanio)
{
	crearEntradas(pid, cantidadPaginas, 0);
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

	}
	while(i < cantidadDePaginas && pointer != comienzo);
	return i == cantidadDePaginas;
}

void crearEntradas(int pid, int cantidadPaginas, int paginaInicial)
{
	entradaTabla* pointer = obtenerEntradaAproximada(pid, paginaInicial);
	int j = paginaInicial;
	int i = 0;
	while(i<cantidadPaginas)
	{
	//	pthread_mutex_lock(&memoriaSem);
		if(pointer->pid == -1)
		{
			pointer->pid = pid;
			pointer->pagina = j;
			j++;
			i++;
			pointer = obtenerEntradaAproximada(pid, j);
		}
		else
			pointer++;
		if(pointer->frame < 0 || pointer->frame >= config->marcos)
		{
			log_info(logFile,"NULL");
			pointer = (entradaTabla*)memoria+cantPaginasAdmin;
		}

//		pthread_mutex_unlock(&memoriaSem);



	}

}


void morirElegantemente()
{
	free(config);
	free(memoria);
	pthread_mutex_lock(&cacheSem);
	list_destroy_and_destroy_elements(cache, destruirEntradaCache);
	pthread_mutex_unlock(&cacheSem);
	close(kernel);
	close(cpu);

}


void imprimirConfig(configFile* config)
{
	puts("--------PROCESO MEMORIA--------");
	printf("ESCUCHANDO EN PUERTO KERNEL: %s | ESCUCHANDO EN PUERTO CPU: %s \n", config->puerto_kernel, config->puerto_cpu);
	printf("CANTIDAD DE MARCOS: %i | TAMAÑO DE MARCOS: %i\n", config->marcos, config->marco_size);
	printf("ENTRADAS CACHE: %i | CACHE X PROC: %i | ALGORITMO REEMPLAZO: %s\n", config->entradas_cache, config->cache_x_proc, config->reemplazo_cache);
	printf("RETARDO DE MEMORIA: %i | IP PROPIA: %s\n", config->retardo_memoria, config->ip_propia);
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
	strcpy(config->ip_propia, config_get_string_value(configHandler, "IP_PROPIA"));
	strcpy(config->log, config_get_string_value(configHandler, "LOG"));
	config_destroy(configHandler);
	imprimirConfig(config);
	if(config->entradas_cache > 0 && config->cache_x_proc == 0)
	{
		config->cache_x_proc++;
		puts("[ERROR]: SI LA CACHE ESTA ACTIVADA NO PUEDE HABER 0 ENTRADAS X PROC. SE SUBIO A 1");
	}
	return config;
}


