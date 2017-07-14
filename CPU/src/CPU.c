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
	levantarLog();
	toBeKilled = 0;
	signal(SIGUSR1, sig_handler);
	iniciarConexiones();
	while(toBeKilled == 0) // EN UN FUTURO ESTO SE CAMBIA POR EL ENVIO DE LA SEÑAL SIGUSR1
	{
		if(esperarPCB() == -1)
		{
			log_info(logFile, "[MURIO EL KERNEL]");
			break;
		}
		informarAMemoriaDelPIDActual();
		estado = OK;
		int rafagas = 0;
		while(estado == OK)
		{

			char* linea = pedirInstruccionAMemoria(pcb, tamanioPagina);
			log_info(logFile, "[PEDIR INSTRUCCION NRO %i | RAFAGA: %i]: %s\n",  pcb->programCounter, rafagas, linea);
			analizadorLinea(linea, &primitivas, &primitivas_kernel);
			free(linea);
			pcb->programCounter++;
			pcb->rafagasTotales++;
			rafagas++;
			if(quantum != 0 && rafagas == quantum && estado == OK)
			{
				if(toBeKilled == 0)
					estado = EXPULSADO;
				else
					estado = KILLED;
			}
			puts("---------------------------------------------------------------------------------------------------------------");
			usleep(quantumSleep*1000);
		}
		log_info(logFile, "[PCB EXPULSADO]: PID: %i | ESTADO: %i\n", pcb->pid, estado);
		serializado pcbSerializado = serializarPCB(pcb);
		lSend(kernel, pcbSerializado.data, estado, pcbSerializado.size);
		free(pcbSerializado.data);

		destruirPCB(pcb);
	}
	free(config);
	close(kernel);
	close(memoria);
	log_destroy(logFile);
	return EXIT_SUCCESS;

}

void sig_handler(int signo)
{
  if (signo == SIGUSR1)
  {
	  log_warning(logFile, "[SEÑAL DE DESCONEXION] NOS VIMOS PAPUS");
	  toBeKilled = 1;
  }
}

void levantarLog()
{
	if(fopen(config->log, "r") != NULL)
		remove(config->log);
	logFile = log_create(config->log, "CPU", 1, 1);
}
void iniciarConexiones()
{
	memoria = getConnectedSocket(config->ip_Memoria, config->puerto_Memoria, CPU_ID);
	kernel = getConnectedSocket(config->ip_Kernel, config->puerto_Kernel, CPU_ID);
	log_info(logFile, "[ESPERANDO INFORMACION]");
	recibirInformacion();
}
void recibirInformacion()
{
	Mensaje* informacion = lRecv(kernel);
	memcpy(&stackSize, informacion->data, sizeof(int));
	memcpy(&tamanioPagina, informacion->data+sizeof(int), sizeof(int));
	memcpy(&quantum, informacion->data+sizeof(int)*2, sizeof(int));
	log_info(logFile, "[RECIBIDO DE KERNEL]: STACK SIZE: %i | TAMANIO PAGINA: %i | QUANTUM: %i\n", stackSize, tamanioPagina, quantum);
	destruirMensaje(informacion);
}

int esperarPCB()
{
	log_info(logFile, "[ESPERANDO PCB]");
	Mensaje* mensaje = lRecv(kernel);
	if(mensaje->header.tipoOperacion == -1)
		return mensaje->header.tipoOperacion;
	pcb = deserializarPCB(mensaje->data+sizeof(int));
	memcpy(&quantumSleep, mensaje->data, sizeof(int));
	log_warning(logFile, "[PCB RECIBIDO]: PID: %i | QUANTUM SLEEP: %i\n", pcb->pid, quantumSleep);
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
	posicionEnMemoria posicion = obtenerPosicionMemoria(pagina, offset, size);
	char* instruccion = leerEnMemoria(posicion);
	return instruccion;
}

posicionEnMemoria obtenerPosicionMemoria(int pagina, int offset, int size)
{
	posicionEnMemoria posicion;
	posicion.pagina = pagina;
	posicion.offset= offset;
	posicion.size = size;
	return posicion;
}

configFile* leer_archivo_configuracion(t_config* configHandler){

	configFile* config = malloc(sizeof(configFile));
	strcpy(config->puerto_Kernel, config_get_string_value(configHandler, "PUERTO_KERNEL"));
	strcpy(config->puerto_Memoria, config_get_string_value(configHandler, "PUERTO_MEMORIA"));
	strcpy(config->ip_Kernel, config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->ip_Memoria, config_get_string_value(configHandler, "IP_MEMORIA"));
	strcpy(config->log, config_get_string_value(configHandler, "LOG"));
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
	int limiteStack = pcb->cantPaginasCodigo+stackSize;
	log_info(logFile, "[DEFINIR VARIABLE - STACK LEVEL: %i]: '%c'\n", pcb->nivelDelStack, identificador);
	posicionEnMemoria unaPosicion = calcularPosicion(pcb->nivelDelStack);
	variable* unaVariable = malloc(sizeof(variable));
	unaVariable->identificador = identificador;
	unaVariable->posicion = unaPosicion;
	if(isdigit(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].argumentos, unaVariable);
	else if(isalpha(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].variables, unaVariable);
	t_puntero direccionReal = convertirADireccionReal(unaVariable->posicion);
	if(unaPosicion.pagina >= limiteStack)
	{
		log_error(logFile, "[DEFINIR VARIABLE]: STACK OVER FLOW PAPU - PROGRAMA ABORTADO");
		estado = STKOF;
	}
	log_info(logFile, "[DEFINIR VARIABLE - STACK LEVEL: %i]: '%c' | PAG: %i | OFFSET: %i | Size: %i:\n", pcb->nivelDelStack, unaVariable->identificador, unaVariable->posicion.pagina, unaVariable->posicion.offset, unaVariable->posicion.size);
	return direccionReal;

}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	log_info(logFile, "[OBTENER POSICION VARIABLE - STACK LEVEL: %i]: '%c'\n", pcb->nivelDelStack, identificador);
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
	log_info(logFile, "[OBTENER POSICION VARIABLE - STACK LEVEL: %i]: '%c' -> %i\n", pcb->nivelDelStack, identificador, direccionReal);
	return direccionReal;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	log_info(logFile, "[ASIGNAR COMPARTIDA]: VAR: %s | VALOR: %i\n", variable, valor);
	int len = strlen(variable)+1;
	int size = len + sizeof(int)*3;
	char* data = malloc(size);
	memcpy(data, &len, sizeof(int));
	memcpy(data +sizeof(int), variable, len);
	memcpy(data + sizeof(int) + len, &valor, sizeof(int));
	memcpy(data + sizeof(int)*2 + len, &pcb->pid, sizeof(int));
	lSend(kernel, data, ASIGNARCOMPARTIDA, size);
	free(data);
/*	Mensaje* respuesta = lRecv(kernel);
	int valorAsignado;
	memcpy(&valorAsignado, respuesta->data, sizeof(int));*/
//	destruirMensaje(respuesta);
	return valor;

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida nombre){

	log_info(logFile, "[VALOR COMPARTIDA]: SE PIDE EL VALOR DE: %s\n", nombre);
	int len = strlen(nombre)+1;
	int size = len + sizeof(int)*2;
	char* data = malloc(size);
	memcpy(data, &pcb->pid, sizeof(int));
	memcpy(data+sizeof(int), &len, sizeof(int));
	memcpy(data+sizeof(int)*2, nombre, len);
	lSend(kernel, data, OBTENERCOMPARTIDA, size);
	Mensaje* respuesta = lRecv(kernel);
	int valor;
	memcpy(&valor, respuesta->data, sizeof(int));
	log_info(logFile, "[VALOR COMPARTIDA]: EL VALOR DE %s es: %i\n", nombre, valor);
	destruirMensaje(respuesta);
	free(data);
	return valor;
}

void asignar(t_puntero direccionReal, t_valor_variable valor)
{
	log_info(logFile, "[ASIGNAR VARIABLE - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	posicionEnMemoria posicion = convertirADireccionLogica(direccionReal);
	char* aux = malloc(posicion.size);
	memcpy(aux, &valor, posicion.size);
	escribirEnMemoria(posicion, aux);
	free(aux);
	log_info(logFile, "[ASIGNAR VARIABLE - STACK LEVEL: %i]: PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", pcb->nivelDelStack, posicion.pagina, posicion.offset, posicion.size, valor);


}


posicionEnMemoria calcularPosicion(int nivelDelStack)
{
	posicionEnMemoria unaPosicion;
	unaPosicion.size = 4;
	if(nivelDelStack == 0)
	{
		if(list_is_empty(pcb->indiceStack[0].variables))
		{
			unaPosicion.pagina = pcb->cantPaginasCodigo;
			unaPosicion.offset = 0;
		}
		else
			unaPosicion = generarPosicionEnBaseAUltimaVariableDe(pcb->indiceStack[0].variables);
	}
	else
	{
		if(list_is_empty(pcb->indiceStack[nivelDelStack].variables))
		{
			if(!list_is_empty(pcb->indiceStack[nivelDelStack].argumentos))
				unaPosicion = generarPosicionEnBaseAUltimaVariableDe(pcb->indiceStack[nivelDelStack].argumentos);
			else
				unaPosicion = calcularPosicion(nivelDelStack-1);
		}
		else
			unaPosicion = generarPosicionEnBaseAUltimaVariableDe(pcb->indiceStack[nivelDelStack].variables);
	}
	return unaPosicion;
}



posicionEnMemoria generarPosicionEnBaseAUltimaVariableDe(t_list* lista)
{
	posicionEnMemoria posicion;
	posicion.size = 4;
	variable* ultimaVariable = obtenerUltimaVariable(lista);
	posicionEnMemoria posicionUltimaVariable = ultimaVariable->posicion;
	int total = posicionUltimaVariable.offset + posicionUltimaVariable.size;
	if(total >= tamanioPagina)
	{
		posicion.offset = total-tamanioPagina;
		posicion.pagina = posicionUltimaVariable.pagina+1;
	}
	else
	{
		posicion.offset = posicionUltimaVariable.offset+4;
		posicion.pagina = posicionUltimaVariable.pagina;
	}
	return posicion;

}

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




char* leerEnMemoria(posicionEnMemoria posicion)
{
	if(estado != OK)
	{
		log_error(logFile, "[LEER EN MEMORIA]: PROGRAMA ABORTADO -> NO SE LEE NADA");
		return NULL;
	}
	int total = posicion.offset + posicion.size;
	int segundoSize;
	char* instruccion;
	log_info(logFile, "[LEER EN MEMORIA]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
	if(total <= tamanioPagina)
	{
		instruccion = enviarPedidoLecturaMemoria(posicion);
		if(instruccion == NULL)
		{
			log_error(logFile, "[LEER EN MEMORIA]: POSICION INVALIDA");
			estado = STKOF;
			return NULL;
		}
		instruccion[posicion.size-1] = '\0';
	}
	else
	{
		instruccion = malloc(posicion.size+1);
		char* puntero = instruccion;
		segundoSize = total-tamanioPagina;
		posicion.size -= segundoSize;
		log_info(logFile, "[LEER EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		char* primeraParte = enviarPedidoLecturaMemoria(posicion);
		if(primeraParte == NULL)
		{
			log_error(logFile, "[LEER EN MEMORIA]: POSICION INVALIDA");
			estado = STKOF;
			return NULL;
		}
		memcpy(puntero, primeraParte, posicion.size);
		puntero+= posicion.size;
		free(primeraParte);
		posicion.pagina++;
		posicion.offset = 0;
		posicion.size = segundoSize;
		char* segundaParteInstruccion = enviarPedidoLecturaMemoria(posicion);
		if(segundaParteInstruccion == NULL)
		{
			log_error(logFile, "[LEER EN MEMORIA]: POSICION INVALIDA");
			estado = STKOF;
			return NULL;
		}
		segundaParteInstruccion[posicion.size-1] = '\0';
		memcpy(puntero, segundaParteInstruccion, posicion.size);
		free(segundaParteInstruccion);
		log_info(logFile, "[LEER EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
	}

	return instruccion;
}


void escribirEnMemoria(posicionEnMemoria posicion, char* valor)
{
	if(estado != OK)
	{
		log_error(logFile, "[ESCRIBIR EN MEMORIA]: PROGRAMA ABORTADO -> NO SE ESCRIBE NADA");
		return;
	}
	log_info(logFile, "[ESCRIBIR EN MEMORIA]: PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %s\n", posicion.pagina, posicion.offset, posicion.size, valor);
	int limiteStack = pcb->cantPaginasCodigo+stackSize;
	int total = posicion.offset + posicion.size;
	/*if(posicion.pagina >= limiteStack)
	{
		log_error(logFile, "[ESCRIBIR EN MEMORIA]: STACK OVER FLOW PAPU - PROGRAMA ABORTADO");
		estado = STKOF;
		return;
	}*/

	if(total <= tamanioPagina)
	{
		if(enviarPedidoEscrituraMemoria(posicion, valor) == -5)
		{
			estado = STKOF;
			log_error(logFile, "[ESCRIBIR EN MEMORIA]: LA POSICION ENVIADA A MEMORIA ES INCORRECTA - POSIBLEMENTE HEAP INVALIDO");
		}
	}
	else
	{
		int segundoSize;
		segundoSize = total-tamanioPagina;
		posicion.size -= segundoSize;
	/*	int valorAEnviar;
		int* a = &valor;
		char* puntero = (char*)a;*/
	/*	memcpy(&valorAEnviar,valor, posicion.size);
		puntero += posicion.size;*/
		log_info(logFile, "[ESCRIBIR EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		if(enviarPedidoEscrituraMemoria(posicion, valor) == -5)
		{
			estado = STKOF;
			log_error(logFile, "[ESCRIBIR EN MEMORIA]: LA POSICION ENVIADA A MEMORIA ES INCORRECTA - POSIBLEMENTE HEAP INVALIDO");
			return;
		}
		valor += posicion.size;
		posicion.pagina++;
		posicion.offset = 0;
		posicion.size = segundoSize;
		//memcpy(&valorAEnviar, puntero, posicion.size);
		log_info(logFile, "[ESCRIBIR EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		if(posicion.pagina >= limiteStack)
		{
			log_error(logFile, "[ESCRIBIR EN MEMORIA]: STACK OVER FLOW PAPU - PROGRAMA ABORTADO");
			estado = STKOF;
			return;
		}

		if(enviarPedidoEscrituraMemoria(posicion, valor) == -5)
		{
			estado = STKOF;
			log_error(logFile, "[ESCRIBIR EN MEMORIA]: LA POSICION ENVIADA A MEMORIA ES INCORRECTA - POSIBLEMENTE HEAP INVALIDO");
			return;
		}

	}
}

int enviarPedidoEscrituraMemoria(posicionEnMemoria posicion, char* valor)
{
	/*pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
	pedido->posicion = posicion;/
	pedido->valor = valor;*/
	int size = posicion.size+sizeof(posicion);
	char* pedido = malloc(size);
	memcpy(pedido, &posicion, sizeof(posicionEnMemoria));
	memcpy(pedido+sizeof(posicionEnMemoria), valor, posicion.size);
	lSend(memoria, pedido, 3,size);
	free(pedido);
	Mensaje* respuesta = lRecv(memoria);
	int op = respuesta->header.tipoOperacion;
	destruirMensaje(respuesta);
	return op;
}

char* enviarPedidoLecturaMemoria(posicionEnMemoria posicion)
{
	lSend(memoria, &posicion, LEER, sizeof(posicionEnMemoria));
	Mensaje* respuesta = lRecv(memoria);
	if(respuesta->header.tipoOperacion == -5)
		return NULL;
	char* data = malloc(respuesta->header.tamanio+1);
	memcpy(data, respuesta->data, respuesta->header.tamanio);
	destruirMensaje(respuesta);
	return data;

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar)
{
	log_info(logFile, "[LLAMAR CON RETORNO - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	pcb->nivelDelStack++;
	pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	int returnpos = pcb->programCounter;
	variable var;
	var.identificador = '\0';
	var.posicion= convertirADireccionLogica(dondeRetornar);
	pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno = returnpos;
	pcb->indiceStack[pcb->nivelDelStack].variableDeRetorno = var;
	pcb->indiceStack[pcb->nivelDelStack].variables = list_create();
	pcb->indiceStack[pcb->nivelDelStack].argumentos = list_create();
	log_info(logFile, "[LLAMAR CON RETORNO - STACK LEVEL: %i a %i]: LLAMA A '%s' - RETURN POS: %i | RETURN VAR: [PAG: %i | OFFSET: %i | SIZE: %i]\n",pcb->nivelDelStack-1, pcb->nivelDelStack, etiqueta, returnpos, var.posicion.pagina, var.posicion.offset, var.posicion.size);
	irAlLabel(etiqueta);
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta)
{

	log_info(logFile, "[LLAMAR SIN RETORNO - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	pcb->nivelDelStack++;
	pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	int returnpos = pcb->programCounter;
	pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno = returnpos;
	pcb->indiceStack[pcb->nivelDelStack].variables = list_create();
	pcb->indiceStack[pcb->nivelDelStack].argumentos = list_create();
	log_info(logFile, "[LLAMAR SIN RETORNO - STACK LEVEL: %i a %i]: LLAMA A '%s' - RETURN POS: %i\n",pcb->nivelDelStack-1, pcb->nivelDelStack, etiqueta, returnpos);
	irAlLabel(etiqueta);
}

t_valor_variable dereferenciar(t_puntero posicion)
{
	log_info(logFile, "[DEREFERENCIAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	posicionEnMemoria direccionLogica = convertirADireccionLogica(posicion);
	char* info = leerEnMemoria(direccionLogica);
	if(info == NULL)
		return 0;
	t_valor_variable valor;
	memcpy(&valor, info, sizeof(int));
	log_info(logFile, "[DEREFERENCIAR - STACK LEVEL: %i]: OBTENGO DE MEMORIA EL SIGUIENTE VALOR: %i\n", pcb->nivelDelStack, valor);
	free(info);
	return valor;

}

void finalizar(void)
{
	log_info(logFile, "[FINALIZAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	if(pcb->nivelDelStack == 0)
		estado = TERMINO;
	else
	{
		pcb->programCounter = pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno;
		list_destroy_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].argumentos, free);
		list_destroy_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].variables, free);
		pcb->nivelDelStack--;
		log_info(logFile, "[FINALIZAR - STACK LEVEL: %i A %i]\n", pcb->nivelDelStack+1, pcb->nivelDelStack);
		pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	}

}
void irAlLabel(t_nombre_etiqueta nombre){
	log_info(logFile, "[IR A LABEL - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	t_puntero_instruccion instruccionParaPCB;
	instruccionParaPCB = metadata_buscar_etiqueta(nombre, pcb->indiceEtiqueta, pcb->sizeIndiceEtiquetas);
	if(instruccionParaPCB == -1)
	{
		log_error(logFile, "[IR A LABEL]: MOSTRO ESTE SCRIPT ESTA ROTISIMO. SALU2");
		estado = ABORTADO;
		return;
	}
	pcb->programCounter = instruccionParaPCB-1;
	log_info(logFile, "[IR A LABEL - STACK LEVEL: %i]: '%s' | PC: %i\n", pcb->nivelDelStack, nombre, pcb->programCounter);
}

void retornar(t_valor_variable valorDeRetorno){
	log_info(logFile, "[RETORNAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	t_puntero dirReal = convertirADireccionReal(pcb->indiceStack[pcb->nivelDelStack].variableDeRetorno.posicion);
	asignar(dirReal,valorDeRetorno);
	log_info(logFile, "[RETORNAR - STACK LEVEL: %i]: RETORNO VALOR: %i\n", pcb->nivelDelStack, valorDeRetorno);

}
	//PRIMITIVAS ANSISOP KERNEL

void wait(t_nombre_semaforo nombre){

	serializado sem = serializarSemaforo(nombre);
	//Envio el nombre del semaforo, uso string_substring_until porque el parser agarra el nombre con el /n
	lSend(kernel, sem.data, WAIT, sem.size);
	//El kernel me responde si tengo que bloquear
	Mensaje *m = lRecv(kernel);
	//Bloqueado = 3, No bloqueado = 0
	int bloqueado = m->header.tipoOperacion;
	if(bloqueado == BLOQUEADO){
		log_info(logFile, "[WAIT - PROCESO BLOQUEADO POR SEMAFORO]");
		//Envio el pid al kernel para que lo guarde en la cola del semaforo
	//	lSend(kernel, &pcb->pid, WAIT, sizeof(int));
		//Para salir del while
		estado = BLOQUEADO;
	}
	free(sem.data);
	destruirMensaje(m);
}

void signalSem(t_nombre_semaforo nombre){
	serializado sem = serializarSemaforo(nombre);
	lSend(kernel, sem.data, SIGNAL, sem.size);
	free(sem.data);
}

serializado serializarSemaforo(char* nombre)
{
	serializado sem;
	int tamanio = strlen(nombre)+1;
	sem.size = tamanio+sizeof(int)*2;
	sem.data = malloc(sem.size);
	memcpy(sem.data, &pcb->pid, sizeof(int));
	memcpy(sem.data+sizeof(int), &tamanio, sizeof(int));
	memcpy(sem.data+sizeof(int)*2, nombre, tamanio);
	return sem;
}

t_puntero reservar(t_valor_variable nroBytes){

	log_info(logFile, "[HEAP]: SE QUIEREN RESERVAR %i BYTES", nroBytes);
	serializado pedido = serializarPedido(nroBytes);
	lSend(kernel, pedido.data, RESERVAR_MEMORIA_HEAP, pedido.size);
	Mensaje* respuesta = lRecv(kernel);
	if(respuesta->header.tipoOperacion == -2)
	{
		log_error(logFile, "[HEAP]: NO HAY ESPACIO DISPONIBLE O SE SUPERO EL TAMANIO MAXIMO");
		estado = ABORTADO;
		return 0;
	}
	posicionEnMemoria posicion;
	posicion.size = 0;
	memcpy(&posicion.pagina, respuesta->data, sizeof(int));
	memcpy(&posicion.offset, respuesta->data+sizeof(int), sizeof(int));
	t_puntero puntero = convertirADireccionReal(posicion);
	destruirMensaje(respuesta);
	log_info(logFile, "[HEAP]: KERNEL DEVUELVE PUNTERO. DIR FISICA: %i | DIR LOGICA: PAG: %i | OFFSET: %i", puntero, posicion.pagina, posicion.offset);
	return puntero;

}

serializado serializarPedido(int num)
{
	serializado pedido;
	pedido.size = sizeof(int)*2;
	pedido.data = malloc(pedido.size);
	memcpy(pedido.data, &pcb->pid, sizeof(int));
	memcpy(pedido.data+sizeof(int), &num, sizeof(int));
	return pedido;

}


void liberar(t_puntero puntero ){

	log_info(logFile, "[HEAP]: SE LIBERA UN PUNTERO");
	posicionEnMemoria posicion = convertirADireccionLogica(puntero);
	int size = sizeof(int)*3;
	char* data = malloc(size);
	memcpy(data, &pcb->pid, sizeof(int));
	memcpy(data+sizeof(int), &posicion.pagina, sizeof(int));
	memcpy(data+sizeof(int)*2, &posicion.offset, sizeof(int));
	lSend(kernel, data, LIBERAR_PUNTERO, size);
	Mensaje* respuesta = lRecv(kernel);
	if(respuesta->header.tipoOperacion == -5)
	{
		estado = STKOF;
		log_error(logFile, "[HEAP]: SE QUISO LIBERAR UNA PAGINA INCORRECTA");
	}
	destruirMensaje(respuesta);
	log_info(logFile, "[HEAP]: SE LIBERA PUNTERO DIR FISICA: %i | DIR LOGICA: PAG: %i | OFFSET: %i", puntero, posicion.pagina, posicion.offset);

}

serializado serializarRutaYPermisos(char* ruta, char* permisos)
{
	// POSIBLES PROBLEMAS CON \0
	int tamanioRuta = strlen(ruta);
	int tamanioPermisos = strlen(permisos);
	serializado infoSerializada;
	infoSerializada.size = tamanioRuta + tamanioPermisos + (sizeof(int)*3);
	infoSerializada.data = malloc(infoSerializada.size);
	memcpy(infoSerializada.data, &pcb->pid, sizeof(int));
	memcpy(infoSerializada.data + sizeof(int), &tamanioRuta, sizeof(int));
	memcpy(infoSerializada.data + sizeof(int) + sizeof(int), ruta, tamanioRuta);
	memcpy(infoSerializada.data + sizeof(int) + sizeof(int) + tamanioRuta, &tamanioPermisos, sizeof(int));
	memcpy(infoSerializada.data + sizeof(int) + sizeof(int) + tamanioRuta + sizeof(int), permisos, tamanioPermisos);
	return infoSerializada;
}

serializado serializarRutaPermisos(char* ruta, char* permisos) {
	serializado s;
	int tamRuta = strlen(ruta);
	int tamPermisos = strlen(permisos);
	s.size = tamRuta+tamPermisos+3*sizeof(int);
	s.data = malloc(s.size);
	memcpy(s.data, &pcb->pid, sizeof(int));
	memcpy(s.data+sizeof(int), &tamRuta, sizeof(int));
	memcpy(s.data+sizeof(int)*2, ruta, tamRuta);;
	memcpy(s.data+sizeof(int)*2+tamRuta, &tamPermisos, sizeof(int));
	memcpy(s.data+sizeof(int)*2+tamRuta+sizeof(int), permisos, tamPermisos);
	return s;
}


serializado serializarPedidoEscrituraFS(char* data, fileInfo info)
{
	serializado pedido;
	pedido.size = sizeof(fileInfo) + info.tamanio;
	pedido.data = malloc(pedido.size);
	memcpy(pedido.data, &info, sizeof(fileInfo));
	memcpy(pedido.data+sizeof(fileInfo), data, info.tamanio);
	return pedido;
}


char* flagsDecentes(t_banderas flags) {
	char* cadenaFlags = string_new();
	if(flags.lectura)
		string_append(&cadenaFlags, "r");
	if(flags.escritura)
		string_append(&cadenaFlags, "w");
	if(flags.creacion)
		string_append(&cadenaFlags, "c");
	return cadenaFlags;
}

void deserializarRutaPermisos(void* data, int* pid, char* ruta, char* permisos) {
	memcpy(pid, data, sizeof(int));
	int tamRuta;
	memcpy(&tamRuta, data+sizeof(int) , sizeof(int));
	memcpy(ruta, data+sizeof(int)*2 , tamRuta);
	ruta[tamRuta] = '\0';
	int tamPermiso;
	memcpy(&tamPermiso, data+sizeof(int)*2+tamRuta , sizeof(int));
	memcpy(permisos, data+sizeof(int)*2+tamRuta+sizeof(int) , tamPermiso);
	permisos[tamPermiso] = '\0';
	log_info(logFile, "TAM RUTA ES: %i\n", tamRuta);
	log_info(logFile, "TAM PERMISO ES: %i\n", tamPermiso);
}

t_descriptor_archivo abrir(t_direccion_archivo ruta , t_banderas flags){
	t_descriptor_archivo fileDescriptor;
	serializado rutaYFlagsSerializados;
	char* cadenaFlags = flagsDecentes(flags);
	rutaYFlagsSerializados = serializarRutaPermisos(ruta,cadenaFlags);
	lSend(kernel, rutaYFlagsSerializados.data, ABRIR_ARCHIVO, rutaYFlagsSerializados.size);
	Mensaje* mensaje = lRecv(kernel);
	if (mensaje->header.tipoOperacion == -3) {
		log_error(logFile, "[ABRIR ARCHIVO]: NO EXISTE ARCHIVO");
		estado = ABORTADO;
		fileDescriptor = -1;
	}
	else {
		fileDescriptor = *(int*)mensaje->data;
		log_info(logFile, "[ABRIR ARCHIVO] FD: %i", fileDescriptor);
	}
	free(cadenaFlags);
	free(rutaYFlagsSerializados.data);
	return fileDescriptor;
}


void borrar(t_descriptor_archivo fileDescriptor){
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	lSend(kernel, &fi, BORRAR_ARCHIVO, sizeof(fileInfo));
	log_info(logFile, "[BORRAR ARCHIVO]: FD: %i", fileDescriptor);
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
	{
		log_error(logFile, "[BORRAR ARCHIVO]: NO EXISTE ARCHIVO O ALGUIEN MAS LO ESTA USANDO");
		estado = ABORTADO;
	}
	destruirMensaje(m);
}

void cerrar(t_descriptor_archivo fileDescriptor) {
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	lSend(kernel, &fi, CERRAR_ARCHIVO, sizeof(fileInfo));
	log_info(logFile, "[CERRAR ARCHIVO] FD: %i", fileDescriptor);
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
	{
		estado = ABORTADO;
		log_error(logFile, "[CERRAR ARCHIVO]: NO EXISTE ARCHIVO");
	}
	destruirMensaje(m);
}

void moverCursor(t_descriptor_archivo fileDescriptor, t_valor_variable cursor){
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	fi.cursor = cursor;
	lSend(kernel, &fi, MOVER_CURSOR_ARCHIVO, sizeof(fileInfo));
	log_info(logFile, "[MOVER CURSOR]: FD: %i | OFFSET: %i\n", fileDescriptor, cursor);
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
	{
		estado = ABORTADO;
		log_error(logFile, "[MOVER CURSOR]: NO EXISTE ARCHIVO");
	}
	destruirMensaje(m);
}


void escribir(t_descriptor_archivo fileDescriptor, void* info, t_valor_variable tamanio){
	fileInfo fi;
 	 fi.fd = fileDescriptor;
 	 fi.pid = pcb->pid;
 	 fi.tamanio = tamanio;
 	 fi.cursor = -1;
 	 char* ay = (char*)info;
 	 log_info(logFile, "[ESCRIBIR ARCHIVO]: ESCRIBO '%s' | FD: %i | SIZE: %i\n", (char*)info, fileDescriptor, tamanio);
 	 serializado escrituraSerializada = serializarPedidoEscrituraFS((char*)info, fi);
 	 lSend(kernel, escrituraSerializada.data, ESCRIBIR_ARCHIVO, escrituraSerializada.size);
 	 Mensaje* m = lRecv(kernel);
 	if(m->header.tipoOperacion == -3)
 	{
 		estado = ABORTADO;
 		log_error(logFile, "[ESCRIBIR ARCHIVO]: NO EXISTE ARCHIVO O PERMISOS INVALIDOS");
 	}
 	destruirMensaje(m);
}

void leer(t_descriptor_archivo fileDescriptor, t_puntero info, t_valor_variable tamanio){
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	fi.tamanio = tamanio;
	lSend(kernel, &fi, LEER_ARCHIVO, sizeof(fileInfo));
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
	{
		estado = ABORTADO;
		log_error(logFile, "[LEER ARCHIVO]: NO EXISTE ARCHIVO O PERMISOS INVALIDOS");
		destruirMensaje(m);
		return;
	}
	char* ay = (char*)m->data;

	log_info(logFile, "[LEER ARCHIVO]: LEO %s | FD: %i | SIZE: %i\n", ay, fileDescriptor, tamanio);
	posicionEnMemoria pos = convertirADireccionLogica(info);
	pos.size = tamanio;
	escribirEnMemoria(pos, ay);
	destruirMensaje(m);
}
