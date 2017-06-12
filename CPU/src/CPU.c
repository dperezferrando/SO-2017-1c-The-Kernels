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
		if(esperarPCB() == -1)
		{
			puts("MURIO EL KERNEL");
			break;
		}
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

int esperarPCB()
{
	puts("Esperando PCB");
	Mensaje* mensaje = lRecv(kernel);
	if(mensaje->header.tipoOperacion == -1)
		return mensaje->header.tipoOperacion;
	puts("PCB RECIBIDO");
	pcb = deserializarPCB(mensaje->data);
	//pcb->indiceStack = crearIndiceDeStack();// AGUJEROS
	int op = mensaje->header.tipoOperacion;
	destruirMensaje(mensaje);
	return op;
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
	puts("DEFINIR VARIABLES");
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

t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	printf("OBTENER POSICION VARIABLE %c\n", identificador);
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
	lSend(kernel, (t_valor_variable) valorAsignado,ASIGNARCOMPARTIDA,sizeof(t_valor_variable));
	return valorAsignado;

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida nombre){
	int tamanio = strlen(nombre)+1;
	char * nombreVariable = malloc(tamanio);
	char* bCero= "/0";

	memcpy(nombreVariable, nombre, strlen(nombre));
	memcpy(nombreVariable+(strlen(nombre)), bCero,strlen(bCero));


	lSend(kernel, (char*) nombreVariable, OBTENERCOMPARTIDA,tamanio);

	Mensaje *m = lRecv(kernel);
	t_valor_variable valor = (t_valor_variable) m->data;

	return valor;
}

void asignar(t_puntero direccionReal, t_valor_variable valor)
{
	printf("ASIGNACION\n");
	posicionEnMemoria posicion = convertirADireccionLogica(direccionReal);
	escribirEnMemoria(posicion, valor);
	printf("ENVIO PEDIDO DE ESCRITURA A MEMORIA PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", posicion.pagina, posicion.offset, posicion.size, valor);


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


void escribirEnMemoria(posicionEnMemoria posicion, t_valor_variable valor)
{
	pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
	pedido->posicion = posicion;
	pedido->valor = valor;
	lSend(memoria, pedido, 3,sizeof(pedidoEscrituraMemoria));
	free(pedido);
}

char* leerEnMemoria(posicionEnMemoria posicion)
{
	lSend(memoria, &posicion, LEER, sizeof(posicionEnMemoria));
	Mensaje* respuesta = lRecv(memoria);
	char* data = malloc(respuesta->header.tamanio);
	memcpy(data, respuesta->data, respuesta->header.tamanio);
	destruirMensaje(respuesta);
	return data;

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar)
{
	puts("LLAMAR CON RETORNO");
	pcb->nivelDelStack++;
	pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	int returnpos = pcb->programCounter;
	//pcb->programCounter = metadata_buscar_etiqueta(etiqueta, pcb->indiceEtiqueta, pcb->sizeIndiceEtiquetas);
	variable var;
	var.identificador = '\0';
	var.posicion= convertirADireccionLogica(dondeRetornar);
	pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno = returnpos;
	pcb->indiceStack[pcb->nivelDelStack].variableDeRetorno = var;
	pcb->indiceStack[pcb->nivelDelStack].variables = list_create();
	pcb->indiceStack[pcb->nivelDelStack].argumentos = list_create();
	printf("[STACK LEVEL: %i a %i] LLAMA A %s - SE GUARDA RETURN POS: %i y LA RETURN VAR ES: PAG: %i OFFSET: %i SIZE: %i\n",pcb->nivelDelStack-1, pcb->nivelDelStack, etiqueta, returnpos, var.posicion.pagina, var.posicion.offset, var.posicion.size);
	irAlLabel(etiqueta);
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta)
{
	puts("LLAMAR SIN RETORNO");
}

t_valor_variable dereferenciar(t_puntero posicion)
{
	puts("DEREFERENCIAR");
	posicionEnMemoria direccionLogica = convertirADireccionLogica(posicion);
	char* info = leerEnMemoria(direccionLogica);
	t_valor_variable valor = (int)(*info);
	printf("OBTENGO DE MEMORIA EL SIGUIENTE VALOR: %i\n", valor);
	free(info);
	return valor;

}

void finalizar(void)
{
	printf("FINALIZAR [STACK LEVEL: %i]\n", pcb->nivelDelStack);
	if(pcb->nivelDelStack == 0)
		finaliza = 1;
	else
	{
		pcb->programCounter = pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno;
		list_clean_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].argumentos, free);
		list_clean_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].variables, free);
		pcb->nivelDelStack--;
		pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	}

}



/*void finalizar(void){
	indStk* aFinalizar;
	aFinalizar=list_get(pcb->indiceStack, list_size(pcb->indiceStack)-1);

	if(list_size(pcb->indiceStack)-1==0){
		//finalizarProceso(pcb->pid);
	}
	else{
		list_clean_and_destroy_elements(aFinalizar->argumentos, *liberar);
		list_clean_and_destroy_elements(aFinalizar->variables, *liberar);
		free(aFinalizar);
		//pcb->programCounter= obtenerpcanterior
	}

}*/




void irAlLabel(t_nombre_etiqueta nombre){
	puts("IR A LABEL");
	t_puntero_instruccion instruccionParaPCB;
	instruccionParaPCB = metadata_buscar_etiqueta(nombre, pcb->indiceEtiqueta, pcb->sizeIndiceEtiquetas);
	pcb->programCounter = instruccionParaPCB-1;
	printf("IR A LABEL: %s PC: %i\n", nombre, pcb->programCounter);
}

void retornar(t_valor_variable valorDeRetorno){
	puts("ASIGNAR VARIABLE RETORNO");
	t_puntero dirReal = convertirADireccionReal(pcb->indiceStack[pcb->nivelDelStack].variableDeRetorno.posicion);
	asignar(dirReal,valorDeRetorno);

}
	//PRIMITIVAS ANSISOP KERNEL

void wait(t_nombre_semaforo nombre){
	int tamanio = strlen(nombre)+1;
	char * nombreSemaforo = malloc(tamanio);
	char* bCero= "/0";
	memcpy(nombreSemaforo, nombre, strlen(nombre));
	memcpy(nombreSemaforo+(strlen(nombre)), bCero,strlen(bCero));
	lSend(kernel, (char*) nombreSemaforo, WAIT, strlen(nombreSemaforo));
	Mensaje *m =lRecv(kernel);
	int bloqueado = (int) m->data;//No se si va esto
	if(bloqueado){
		puts("Se bloqueo maestro");
	}
	free(nombreSemaforo);
}

void signal(t_nombre_semaforo nombre){
	int tamanio = strlen(nombre)+1;
	char * nombreSemaforo = malloc(tamanio);
	char* bCero= "/0";
	memcpy(nombreSemaforo, nombre, strlen(nombre));
	memcpy(nombreSemaforo+(strlen(nombre)), bCero,strlen(bCero));
	lSend(kernel, (char*) nombreSemaforo, SIGNAL, strlen(nombreSemaforo));
}
	/*
	t_puntero reservar(t_valor_variable){
		return;
	}
	void liberar(t_puntero){
		return;
	}

	t_descriptor_archivo abrir(t_direccion_archivo, t_banderas){
		return;
	}

	void borrar(t_descriptor_archivo){
		return;
	}

	void cerrar(t_descriptor_archivo){
		return;
	}

	void moverCursor(t_descriptor_archivo, t_valor_variable){
		return;
	}

	void escribir(t_descriptor_archivo, void*, t_valor_variable){
		return;
	}

	void leer(t_descriptor_archivo, t_puntero, t_valor_variable){
		return;
	}
	*/


