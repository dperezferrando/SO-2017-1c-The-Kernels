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
		estado = OK;
		while(estado == OK)
		{
			char* linea = pedirInstruccionAMemoria(pcb, tamanioPagina);
			printf("[PEDIR INSTRUCCION NRO %i]: %s\n", pcb->programCounter, linea);
			analizadorLinea(linea, &primitivas, &primitivas_kernel);
			free(linea);
			pcb->programCounter++;
		}
		serializado pcbSerializado = serializarPCB(pcb);
		lSend(kernel, pcbSerializado.data, estado, pcbSerializado.size);
		free(pcbSerializado.data);
		destruirPCB(pcb);
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
	puts("Esperando Informacion");
	recibirInformacion();
}
void recibirInformacion()
{
	Mensaje* informacion = lRecv(kernel);
	memcpy(&stackSize, informacion->data, sizeof(int));
	memcpy(&tamanioPagina, informacion->data+sizeof(int), sizeof(int));
	memcpy(&quantum, informacion->data+sizeof(int)*2, sizeof(int));
	destruirMensaje(informacion);
}

int esperarPCB()
{
	puts("Esperando PCB");
	Mensaje* mensaje = lRecv(kernel);
	if(mensaje->header.tipoOperacion == -1)
		return mensaje->header.tipoOperacion;
	puts("PCB RECIBIDO");
	pcb = deserializarPCB(mensaje->data);
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
	printf("[DEFINIR VARIABLE - STACK LEVEL: %i]: '%c'\n", pcb->nivelDelStack, identificador);
	posicionEnMemoria unaPosicion = calcularPosicion(pcb->nivelDelStack);
	variable* unaVariable = malloc(sizeof(variable));
	unaVariable->identificador = identificador;
	unaVariable->posicion = unaPosicion;
	if(isdigit(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].argumentos, unaVariable);
	else if(isalpha(unaVariable->identificador))
		list_add(pcb->indiceStack[pcb->nivelDelStack].variables, unaVariable);
	t_puntero direccionReal = convertirADireccionReal(unaVariable->posicion);
	printf("[DEFINIR VARIABLE - STACK LEVEL: %i]: '%c' | PAG: %i | OFFSET: %i | Size: %i:\n", pcb->nivelDelStack, unaVariable->identificador, unaVariable->posicion.pagina, unaVariable->posicion.offset, unaVariable->posicion.size);
	return direccionReal;

}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	printf("[OBTENER POSICION VARIABLE - STACK LEVEL: %i]: '%c'\n", pcb->nivelDelStack, identificador);
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
	printf("[OBTENER POSICION VARIABLE - STACK LEVEL: %i]: '%c' -> %i\n", pcb->nivelDelStack, identificador, direccionReal);
	return direccionReal;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){//falta ersolver
	t_valor_variable valorAsignado=valor;
	lSend(kernel, (t_valor_variable) &valorAsignado,ASIGNARCOMPARTIDA,sizeof(t_valor_variable));
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
	printf("[ASIGNAR VARIABLE - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	posicionEnMemoria posicion = convertirADireccionLogica(direccionReal);
	escribirEnMemoria(posicion, valor);
	printf("[ASIGNAR VARIABLE - STACK LEVEL: %i]: PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", pcb->nivelDelStack, posicion.pagina, posicion.offset, posicion.size, valor);


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
	int total = posicion.offset + posicion.size;
	int segundoSize;
	char* instruccion;
	printf("[LEER EN MEMORIA]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
	if(total <= tamanioPagina)
	{
		instruccion = enviarPedidoLecturaMemoria(posicion);
		instruccion[posicion.size-1] = '\0';
	}
	else
	{
		instruccion = malloc(posicion.size+1);
		char* puntero = instruccion;
		segundoSize = total-tamanioPagina;
		posicion.size -= segundoSize;
		printf("[LEER EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		char* primeraParte = enviarPedidoLecturaMemoria(posicion);
		memcpy(puntero, primeraParte, posicion.size);
		puntero+= posicion.size;
		free(primeraParte);
		posicion.pagina++;
		posicion.offset = 0;
		posicion.size = segundoSize;
		char* segundaParteInstruccion = enviarPedidoLecturaMemoria(posicion);
		segundaParteInstruccion[posicion.size-1] = '\0';
		memcpy(puntero, segundaParteInstruccion, posicion.size);
		free(segundaParteInstruccion);
		printf("[LEER EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
	}

	return instruccion;
}


void escribirEnMemoria(posicionEnMemoria posicion, t_valor_variable valor)
{
	printf("[ESCRIBIR EN MEMORIA]: PAG: %i | OFFSET: %i | SIZE: %i | VALOR: %i\n", posicion.pagina, posicion.offset, posicion.size, valor);
	int limiteStack = pcb->cantPaginasCodigo+stackSize;
	int total = posicion.offset + posicion.size;
	if(posicion.pagina >= limiteStack)
	{
		puts("[ESCRIBIR EN MEMORIA]: STACK OVER FLOW PAPU - PROGRAMA ABORTADO");
		estado = ABORTADO;
		return;
	}

	if(total <= tamanioPagina)
		enviarPedidoEscrituraMemoria(posicion, valor);
	else
	{
		int segundoSize;
		segundoSize = total-tamanioPagina;
		posicion.size -= segundoSize;
		int valorAEnviar;
		int* a = &valor;
		char* puntero = (char*)a;
		memcpy(&valorAEnviar, puntero, posicion.size);
		puntero += posicion.size;
		printf("[ESCRIBIR EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		enviarPedidoEscrituraMemoria(posicion, valorAEnviar);
		posicion.pagina++;
		posicion.offset = 0;
		posicion.size = segundoSize;
		memcpy(&valorAEnviar, puntero, posicion.size);
		printf("[ESCRIBIR EN MEMORIA - PEDIDO PARTIDO]: PAG: %i | OFFSET: %i | SIZE: %i\n", posicion.pagina, posicion.offset, posicion.size);
		if(posicion.pagina >= limiteStack)
		{
			puts("[ESCRIBIR EN MEMORIA]: STACK OVER FLOW PAPU - PROGRAMA ABORTADO");
			estado = ABORTADO;
			return;
		}

		enviarPedidoEscrituraMemoria(posicion, valorAEnviar);

	}
}

void enviarPedidoEscrituraMemoria(posicionEnMemoria posicion, t_valor_variable valor)
{
	pedidoEscrituraMemoria* pedido = malloc(sizeof(pedidoEscrituraMemoria));
	pedido->posicion = posicion;
	pedido->valor = valor;
	lSend(memoria, pedido, 3,sizeof(pedidoEscrituraMemoria));
	free(pedido);
}

char* enviarPedidoLecturaMemoria(posicionEnMemoria posicion)
{
	lSend(memoria, &posicion, LEER, sizeof(posicionEnMemoria));
	Mensaje* respuesta = lRecv(memoria);
	char* data = malloc(respuesta->header.tamanio+1);
	memcpy(data, respuesta->data, respuesta->header.tamanio);
	destruirMensaje(respuesta);
	return data;

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar)
{
	printf("[LLAMAR CON RETORNO - STACK LEVEL: %i]\n", pcb->nivelDelStack);
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
	printf("[LLAMAR CON RETORNO - STACK LEVEL: %i a %i]: LLAMA A '%s' - RETURN POS: %i | RETURN VAR: [PAG: %i | OFFSET: %i | SIZE: %i]\n",pcb->nivelDelStack-1, pcb->nivelDelStack, etiqueta, returnpos, var.posicion.pagina, var.posicion.offset, var.posicion.size);
	irAlLabel(etiqueta);
}

void llamarSinRetorno(t_nombre_etiqueta etiqueta)
{

	printf("[LLAMAR SIN RETORNO - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	pcb->nivelDelStack++;
	pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	int returnpos = pcb->programCounter;
	pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno = returnpos;
	pcb->indiceStack[pcb->nivelDelStack].variables = list_create();
	pcb->indiceStack[pcb->nivelDelStack].argumentos = list_create();
	printf("[LLAMAR SIN RETORNO - STACK LEVEL: %i a %i]: LLAMA A '%s' - RETURN POS: %i\n",pcb->nivelDelStack-1, pcb->nivelDelStack, etiqueta, returnpos);
	irAlLabel(etiqueta);
}

t_valor_variable dereferenciar(t_puntero posicion)
{
	printf("[DEREFERENCIAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	posicionEnMemoria direccionLogica = convertirADireccionLogica(posicion);
	char* info = leerEnMemoria(direccionLogica);
	t_valor_variable valor;
	memcpy(&valor, info, sizeof(int));
	printf("[DEREFERENCIAR - STACK LEVEL: %i]: OBTENGO DE MEMORIA EL SIGUIENTE VALOR: %i\n", pcb->nivelDelStack, valor);
	free(info);
	return valor;

}

void finalizar(void)
{
	printf("[FINALIZAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	if(pcb->nivelDelStack == 0)
		estado = TERMINO;
	else
	{
		pcb->programCounter = pcb->indiceStack[pcb->nivelDelStack].posicionDeRetorno;
		list_destroy_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].argumentos, free);
		list_destroy_and_destroy_elements(pcb->indiceStack[pcb->nivelDelStack].variables, free);
		pcb->nivelDelStack--;
		printf("[FINALIZAR - STACK LEVEL: %i A %i]\n", pcb->nivelDelStack+1, pcb->nivelDelStack);
		pcb->indiceStack = realloc(pcb->indiceStack, sizeof(indStk)*(pcb->nivelDelStack+1));
	}

}
void irAlLabel(t_nombre_etiqueta nombre){
	printf("[IR A LABEL - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	t_puntero_instruccion instruccionParaPCB;
	instruccionParaPCB = metadata_buscar_etiqueta(nombre, pcb->indiceEtiqueta, pcb->sizeIndiceEtiquetas);
	pcb->programCounter = instruccionParaPCB-1;
	printf("[IR A LABEL - STACK LEVEL: %i]: '%s' | PC: %i\n", pcb->nivelDelStack, nombre, pcb->programCounter);
}

void retornar(t_valor_variable valorDeRetorno){
	printf("[RETORNAR - STACK LEVEL: %i]\n", pcb->nivelDelStack);
	t_puntero dirReal = convertirADireccionReal(pcb->indiceStack[pcb->nivelDelStack].variableDeRetorno.posicion);
	asignar(dirReal,valorDeRetorno);
	printf("[RETORNAR - STACK LEVEL: %i]: RETORNO VALOR: %i\n", pcb->nivelDelStack, valorDeRetorno);

}
	//PRIMITIVAS ANSISOP KERNEL

void wait(t_nombre_semaforo nombre){
	int tamanio = strlen(nombre);
	//Envio el nombre del semaforo, uso string_substring_until porque el parser agarra el nombre con el /n
	lSend(kernel, nombre, WAIT, tamanio);
	//El kernel me responde si tengo que bloquear
	Mensaje *m = lRecv(kernel);
	//Bloqueado = 3, No bloqueado = 0
	int bloqueado = m->header.tipoOperacion;
	if(bloqueado == BLOQUEADO){
		puts("[WAIT - PROCESO BLOQUEADO POR SEMAFORO]");
		//Envio el pid al kernel para que lo guarde en la cola del semaforo
		lSend(kernel, &pcb->pid, WAIT, sizeof(int));
		//Para salir del while
		estado = BLOQUEADO;
	}
	destruirMensaje(m);
}

void signal(t_nombre_semaforo nombre){
	int tamanio = strlen(nombre);
	lSend(kernel, nombre, SIGNAL, tamanio);
}

t_puntero reservar(t_valor_variable nroBytes){
	int tamanioAReservar = nroBytes;
	lSend(kernel, &tamanioAReservar, RESERVAR_MEMORIA_HEAP, sizeof(tamanioAReservar));

	Mensaje *m =lRecv(kernel);
	t_puntero puntero= (t_puntero) m->data;
	if(puntero<0){
		puts("No se pudo reservar memoria en el heap");
	}

	return puntero;

}


void liberar(t_puntero puntero ){
	lSend(kernel, &puntero, LIBERAR_PUNTERO, sizeof(t_puntero));
	return;
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
	printf("TAM RUTA ES: %i\n", tamRuta);
	printf("TAM PERMISO ES: %i\n", tamPermiso);
}

t_descriptor_archivo abrir(t_direccion_archivo ruta , t_banderas flags){
	t_descriptor_archivo fileDescriptor;
	serializado rutaYFlagsSerializados;
	char* cadenaFlags = flagsDecentes(flags);
	rutaYFlagsSerializados = serializarRutaPermisos(ruta,cadenaFlags);
	printf("EL SIZE ES: %i\n", rutaYFlagsSerializados.size);
	lSend(kernel, rutaYFlagsSerializados.data, ABRIR_ARCHIVO, rutaYFlagsSerializados.size);
	Mensaje* mensaje = lRecv(kernel);
	if (mensaje->header.tipoOperacion == -3) {
		puts("NO EXISTE ARCHIVO");
		estado = ACCESO_ARCHIVO_INEXISTENTE;
		fileDescriptor = -1;
	}
	else {
		fileDescriptor = *(int*)mensaje->data;
		printf("EL FD ES %i\n", fileDescriptor);
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
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
		estado = INTENTAR_BORRAR_ARCHIVO_EN_USO;
}

void cerrar(t_descriptor_archivo fileDescriptor) {
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	lSend(kernel, &fi, CERRAR_ARCHIVO, sizeof(fileInfo));
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
		estado = ARCHIVO_NULO;
}

void moverCursor(t_descriptor_archivo fileDescriptor, t_valor_variable cursor){
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	fi.cursor = cursor;
	lSend(kernel, &fi, MOVER_CURSOR_ARCHIVO, sizeof(fileInfo));
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
		estado = ARCHIVO_NULO;
}


void escribir(t_descriptor_archivo fileDescriptor, void* info, t_valor_variable tamanio){
 	 fileInfo fi;
 	 fi.fd = fileDescriptor;
 	 fi.pid = pcb->pid;
 	 fi.tamanio = tamanio;
 	 fi.cursor = -1;
 	 printf("[ESCRIBIR]: ESCRIBO '%s' | FD: %i | SIZE: %i\n", (char*)info, fileDescriptor, tamanio);
 	 serializado escrituraSerializada = serializarPedidoEscrituraFS((char*)info, fi);
 	 lSend(kernel, escrituraSerializada.data, ESCRIBIR_ARCHIVO, escrituraSerializada.size);
 	 Mensaje* m = lRecv(kernel);
 	if(m->header.tipoOperacion == -3)
 		estado = ESCRITURA_ARCHIVO_SIN_PERMISO;
}

void leer(t_descriptor_archivo fileDescriptor, t_puntero info, t_valor_variable tamanio){
	fileInfo fi;
	fi.fd = fileDescriptor;
	fi.pid = pcb->pid;
	fi.tamanio = tamanio;
	lSend(kernel, &fi, LEER_ARCHIVO, sizeof(fileInfo));
	Mensaje* m = lRecv(kernel);
	if(m->header.tipoOperacion == -3)
		estado = LECTURA_ARCHIVO_SIN_PERMISO;
}


/*t_descriptor_archivo abrir(t_direccion_archivo direccion, t_banderas banderas){
+	pedidoAperturaArchivo pedido;
+	pedido.dir=direccion;
+	pedido.flags = banderas;
+	int sizePedido= strlen(direccion)+sizeof(banderas);
+	pedidoAperturaArchivo* enviarPedido = serializarPedidoApertura(pedido);
+
+	lSend(kernel, enviarPedido, ABRIR_ARCHIVO, sizePedido);
+
+	Mensaje *m = lRecv(kernel);
+	t_descriptor_archivo fd= (t_descriptor_archivo) m->data;
+
+	return fd;
+}
+
+pedidoAperturaArchivo* serializarPedidoApertura(pedidoAperturaArchivo request){
+	pedidoAperturaArchivo* ret = malloc(sizeof(pedidoAperturaArchivo));
+	memcpy(&ret,&request.dir, strlen(request.dir));//no se si agregar el barra cero
+	memcpy(&ret+strlen(request.dir),&request.flags, sizeof(t_banderas));
+	return ret;
+	free(ret);
+}
+*/
