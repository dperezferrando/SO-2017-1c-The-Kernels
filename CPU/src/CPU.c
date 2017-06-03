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
		while(finaliza == 0) // HARDCODEADO
		{
			char* linea = pedirInstruccionAMemoria(pcb, tamanioPagina);
			printf("Instruccion [%i]: %s\n", pcb->programCounter, linea);
			analizadorLinea(linea, &primitivas, &primitivas_kernel);
			free(linea);
			pcb->programCounter++;
		}
		serializado pcbSerializado = serializarPCB(pcb);
		lSend(kernel, pcbSerializado.data, 1, pcbSerializado.size);
		free(pcb);
	}
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
	Mensaje* algoritmo = lRecv(kernel);
	memcpy(&quantum,algoritmo->data, sizeof(int));
	free(tamanioPaginas);
}

void esperarPCB()
{
	puts("Esperando PCB");
	Mensaje* mensaje = lRecv(kernel);
	puts("PCB RECIBIDO");
	pcb = deserializarPCB(mensaje->data);
	//pcb->indiceStack = crearIndiceDeStack();// AGUJEROS
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
	posicionEnMemoria unaPosicion = calcularPosicion();
	variable* unaVariable = malloc(sizeof(variable));
	unaVariable->identificador = identificador;
	unaVariable->posicion = unaPosicion;
	if(isdigit(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].argumentos, unaVariable);
	else if(isalpha(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].variables, unaVariable);
	t_puntero direccionReal = convertirADireccionReal(unaVariable->posicion);
	printf("Definiendo variable %c [STACK LEVEL: %i]: PAG: %i | OFFSET: %i | Size: %i:\n", unaVariable->identificador, pcb->nivelDelStack, unaVariable->posicion.pagina, unaVariable->posicion.offset, unaVariable->posicion.size);
	return direccionReal;

}

posicionEnMemoria calcularPosicion()
{
	posicionEnMemoria unaPosicion;
	unaPosicion.size = 4;
	if(pcb->nivelDelStack == 0)
	{
		if(list_is_empty(pcb->indiceStack[0].variables))
		{
			unaPosicion.pagina = pcb->cantPaginasCodigo;
			unaPosicion.offset = 0;
		}
		else
		{
			variable* ultimaVariable = obtenerUltimaVariable(pcb->indiceStack[0].variables);
			posicionEnMemoria posicionUltimaVariable = ultimaVariable->posicion;
			unaPosicion.offset = posicionUltimaVariable.offset+4;
			if(unaPosicion.offset > tamanioPagina)
			{
				unaPosicion.pagina = posicionUltimaVariable.pagina+1;
				unaPosicion.offset = 0;
			}
			else
				unaPosicion.pagina = posicionUltimaVariable.pagina;
		}
	}
	else
	{
		if(list_is_empty(pcb->indiceStack[pcb->nivelDelStack].variables))
		{
			variable* ultimaVariable = obtenerUltimaVariable(pcb->indiceStack[pcb->nivelDelStack].argumentos);
			posicionEnMemoria posicionUltimaVariable = ultimaVariable->posicion;
			unaPosicion.offset = posicionUltimaVariable.offset+4;
			if(unaPosicion.offset > tamanioPagina)
			{
				unaPosicion.pagina = posicionUltimaVariable.pagina+1;
				unaPosicion.offset = 0;
			}
			else
				unaPosicion.pagina = posicionUltimaVariable.pagina;
		}
		else
		{
			variable* ultimaVariable = obtenerUltimaVariable(pcb->indiceStack[pcb->nivelDelStack].variables);
			posicionEnMemoria posicionUltimaVariable = ultimaVariable->posicion;
			unaPosicion.offset = posicionUltimaVariable.offset+4;
			if(unaPosicion.offset > tamanioPagina)
			{
				unaPosicion.pagina = posicionUltimaVariable.pagina+1;
				unaPosicion.offset = 0;
			}
			else
				unaPosicion.pagina = posicionUltimaVariable.pagina;
		}
	}
	return unaPosicion;
}

/*
 * LA FUNCION QUE VA A EVITAR LA REPITICION DE LOGICA SIDA DE ARRIBA
posicionEnMemoria generarPosicionEnBaseAUltimaVariableDe(t_list* lista)
{
	posicionEnMemoria posicion;
	posicion.size = 4;

}*/

t_puntero convertirADireccionReal(posicionEnMemoria unaPosicion)
{
	return (unaPosicion.pagina*tamanioPagina) + unaPosicion.offset;
}

posicionEnMemoria convertirADireccionLogica(t_puntero posicionReal)
{
	posicionEnMemoria posicion;
	posicion.pagina = floor((double)posicionReal/(double)tamanioPagina);
	posicion.offset = posicionReal % tamanioPagina;
	posicion.size = 4;
	return posicion;
}

variable* obtenerUltimaVariable(t_list* listaVariables)
{
	int ultimaPos = list_size(listaVariables)-1;
	variable* unaVariable = list_get(listaVariables, ultimaPos);
	return unaVariable;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	t_list* lista;
	if(isalpha(identificador))
		lista = pcb->indiceStack[pcb->nivelDelStack].variables;
	else
		lista = pcb->indiceStack[pcb->nivelDelStack].argumentos;
	bool elIdEsElMismo(variable* variable)
	{
		return variable->identificador == identificador;
	}
	variable* unaVariable = list_find(lista, elIdEsElMismo);
	t_puntero direccionReal = convertirADireccionReal(unaVariable->posicion);
	printf("OBTENGO DIRECCION REAL VARIABLE '%c': %i\n", identificador, direccionReal);
	return direccionReal;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){//falta ersolver
	t_valor_variable valorAsignado=valor;
	lSend(kernel, (t_valor_variable) valorAsignado,0,sizeof(t_valor_variable));
	return valorAsignado;

}

void asignar(t_puntero direccionReal, t_valor_variable valor)
{
	posicionEnMemoria posicion = convertirADireccionLogica(direccionReal);
	escribirEnMemoria(posicion, valor);
	printf("ASIGNACION\n");

}

void escribirEnMemoria(posicionEnMemoria posicion, t_valor_variable valor)
{
	pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
	pedido->posicion = posicion;
	pedido->valor = valor;
	lSend(memoria, pedido, 3,sizeof(pedidoEscrituraMemoria));
	free(pedido);
}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar)
{
	puts("LLAMARCONRETORNO");
}

void finalizar(void)
{
	finaliza = 1;
	puts("FINALIZAR");
}

void irAlLabel(t_nombre_etiqueta nombre)
{
	puts("IR A LABEL");
}

void retornar(t_valor_variable valorDeRetorno)
{
	puts("RETORNAR A NIVEL ANTERIOR DE STACK");
}
