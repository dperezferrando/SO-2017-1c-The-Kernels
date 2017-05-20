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





int main(int argc, char** argsv) {
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/CPU/Debug/CPU.conf", leer_archivo_configuracion, keys);
	iniciarConexiones();
	while(1) // EN UN FUTURO ESTO SE CAMBIA POR EL ENVIO DE LA SEÑAL SIGUSR1
	{
		esperarPCB();
		informarAMemoriaDelPIDActual();
		while(pcb->programCounter<5) // HARDCODEADO
		{
			char* linea = pedirInstruccionAMemoria(pcb, tamanioPagina);
			//analizadorLinea(linea, &primitivas, &primitivas_kernel);
			printf("Instruccion [%i]: %s\n", pcb->programCounter, linea);
			free(linea);
			pcb->programCounter++;
		}
		PCBSerializado pcbSerializado = serializarPCB(pcb);
		lSend(kernel, pcbSerializado.data, 1, pcbSerializado.size);
	}
	free(pcb);
	free(config);
	close(kernel);
	close(memoria);
	return EXIT_SUCCESS;

}

void iniciarConexiones()
{
	kernel = getConnectedSocket(config->ip_Kernel, config->puerto_Kernel, CPU_ID);
	memoria = getConnectedSocket(config->ip_Memoria, config->puerto_Memoria, CPU_ID);
	puts("Esperando Tamaño Paginas");
	Mensaje* tamanioPaginas = lRecv(kernel);
	memcpy(&tamanioPagina, tamanioPaginas->data, sizeof(int));
	free(tamanioPaginas);
}

void esperarPCB()
{
	puts("Esperando PCB");
	Mensaje* mensaje = lRecv(kernel);
	puts("PCB RECIBIDO");
	pcb = deserializarPCB(mensaje->data);
	destruirMensaje(mensaje);
}

void informarAMemoriaDelPIDActual()
{
	char* pid = malloc(sizeof(int));
	memcpy(pid, &pcb->pid, sizeof(int));
	lSend(memoria, pid, 1, sizeof(int));
	free(pid);
}

char* pedirInstruccionAMemoria(PCB* pcb, int tamanioPagina)
{
	int pagina = pcb->indiceCodigo[pcb->programCounter].offset/tamanioPagina;
	int offset = pcb->indiceCodigo[pcb->programCounter].offset%tamanioPagina;
	int size = pcb->indiceCodigo[pcb->programCounter].longitud;
	printf("MANDO A MEMORIA ORDEN A LEER PAG: %i - OFFSET: %i - SIZE: %i\n", pagina, offset, size);
	posicionEnMemoria* posicion = obtenerPosicionMemoria(pagina, offset, size);
	lSend(memoria, posicion, 2, sizeof(posicionEnMemoria));
	free(posicion);
	Mensaje* respuesta = lRecv(memoria);
	char* instruccion = malloc(size);
	memcpy(instruccion, respuesta->data, size);
	instruccion[size] = '\0';
	destruirMensaje(respuesta);
	return instruccion;
}

posicionEnMemoria* obtenerPosicionMemoria(int pagina, int offset, int size)
{
	posicionEnMemoria* posicion = malloc(sizeof(posicionEnMemoria));
	posicion->pagina = pagina;
	posicion->offset= offset;
	posicion->size = size;
	return posicion;
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

// FUNCIONES ANSISOP PARSER
t_puntero definirVariable(t_nombre_variable identificador){
	t_puntero posicionMemoria;
	Mensaje* m;
	lSend(memoria,(t_nombre_variable) identificador,0,strlen(identificador)+1);
	m = lRecv(memoria);
	posicionMemoria=(t_puntero) m-> data;
	return posicionMemoria;

}


t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	t_puntero desplazamiento;
	Mensaje* m;
	lSend(memoria,(t_nombre_variable) identificador,0,strlen(identificador)+1);
	m=lRecv(memoria);
	desplazamiento=(t_puntero) m->data;
	if(desplazamiento==-1){
		puts("Error");
	}

	return desplazamiento;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){//falta ersolver
	t_valor_variable valorAsignado=valor;
	lSend(kernel, (t_valor_variable) valorAsignado,0,sizeof(t_valor_variable));
	return valorAsignado;

}
