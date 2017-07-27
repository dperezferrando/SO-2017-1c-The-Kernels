/*
 ============================================================================
 Name        : Consola.c
 Author      : Dario Poma
 Version     : 2.0
 Copyright   : ---
 Description : Una hermosa consola...
 ============================================================================
 */

#include "Consola.h"

int main(void) {
	signal(SIGINT, handleExit);
	iniciarSemaforosDeControl();
	leerArchivoDeConfiguracion();
	conectarConKernel();
	atenderPedidos();
	finalizarProceso();
	return EXIT_SUCCESS;
}

void handleExit(int sig)
{
	desconectar();
	//sem_wait(&finalizacionHiloReceptor);
	puts("\nApreta enter para salir");
}

void atenderPedidos() {
	crearListaDeProgramas();
	crearHiloReceptor();
	atenderInstrucciones();
}

void finalizarProceso() {
	liberar(config);
	close(kernel);
	destruirListaDeProgramas();
	//mensajeCerrandoProcesos();
	//sleep(1);
	mensajeConsolaDesconectada();
}

void atenderInstrucciones() {
	Instruccion instruccion;
	instruccion.comando = 0;
	while(morir == 0) {
		instruccion = obtenerInstruccion();
		switch(instruccion.comando) {
			case RUN: iniciar(instruccion.argumento); break;
			case CLOSE: cerrar(instruccion.argumento); break;
			case EXIT: desconectar(); break;
			case CLEAR: limpiar(); break;
			case LIST: mostrarLista(); break;
			case HELP: mensajeAyuda(); break;
			default: break;
		}
	}
}


void iniciar(char* path) {
	crearHiloPrograma(path);
}

void cerrar(char* pidPrograma) {
	sem_wait(&mutexLista);
	Programa* programa = buscarProgramaPorPid(pidPrograma);
	if(existeElPrograma(programa)) {
		enviarAlKernel(obtenerPid(programa), CERRAR_PROCESO);
	}
	else
		mensajeErrorPid();
	sem_post(&mutexLista);
}

void limpiar() {
	system("clear");
	mensajeIngresar();
}

void desbloquearHilos() {
	int i = 0;
	sem_wait(&mutexLista);
	for(i=0; i<list_size(listaDeProgramas); i++)
		sem_post(&((Programa*)list_get(listaDeProgramas, i))->semaforo);
	sem_post(&mutexLista);
}

void desconectar() {
	morir = 1;
	enviarAlKernel(&nada, ABORTAR_PROCESO);
}


void abortar() {
	if(listaDeProgramasEstaVacia())
		sem_post(&destruccionMensaje);
	else {
	desbloquearHilos();
	}
}

void mostrarLista() {
	sem_wait(&mutexLista);
	if(listaDeProgramasEstaVacia()) {
		sem_post(&mutexLista);
		mensajeNoHayProcesos();
	}
	else {
		sem_post(&mutexLista);
		recorrerLista();
	}
	mensajeIngresar();
}

//-----------------------------------FUNCIONES HILO PROGRAMA-----------------------------------

void crearHiloPrograma(char* path) {
	Programa* programa = malloc(sizeof(Programa));
	strcpy(programa->path, path);
	pthread_create(&(programa->hiloPrograma), NULL, (void *) hiloPrograma, programa);
}

void iniciarPrograma(Programa* programa) {
	programa->impresiones = 0;
	programa->tiempoInicio = obtenerTiempo();
	sem_wait(&mutexMensaje);
	programa->pid = pidEnviadoPorKernel();
	sem_post(&mutexMensaje);
	iniciarSemaforo(&programa->semaforo, 0);
	agregarAListaDeProgramas(programa);
}

void hiloPrograma(Programa* programa) {
	pthread_detach(pthread_self());
	FILE* archivo = abrirArchivo(programa->path);
	if(archivoValido(archivo) && esUnArchivo(programa->path)) {
		enviarArchivoAlKernel(archivo);
		sem_wait(&nuevoMensaje);
		sem_wait(&mutexMensaje);
		if(operacionDelMensajeEs(NUEVO_PID)) {
			sem_post(&mutexMensaje);
			iniciarPrograma(programa);
			mensajeInicioPrograma(programa->pid);
			esperarMensajes(programa);
		}
		else {
			sem_post(&mutexMensaje);
			mensajeNoHayEspacio();
			sem_post(&destruccionMensaje);
			free(programa);
		}
	}
	else {
		mensajeErrorArchivo();
		free(programa);
	}
}

void esperarMensajes(Programa* programa) {
	int estado = ACTIVADO;
	while(estado != DESACTIVADO) {
		sem_wait(&programa->semaforo);
		sem_wait(&mutexMensaje);
		switch(tipoOperacion()) {
			case NUEVO_MENSAJE:
				imprimirMensaje(programa->pid, mensaje->data); programa->impresiones++; sem_post(&mutexMensaje); sem_post(&destruccionMensaje);
				break;
			case CERRAR_PROCESO: sem_post(&mutexMensaje);cerrarPrograma(programa); mensajeIngresar(); estado = DESACTIVADO; break;
			case ABORTAR_PROCESO: sem_post(&mutexMensaje); desconectarPrograma(programa); estado = DESACTIVADO; break;
		}
	}
}

void desconectarPrograma(Programa* programa) {
	informacionPrograma(programa);
	eliminarDeListaDeProgramas(programa);
	liberar(programa);
	sem_wait(&mutexLista);
	if(listaDeProgramasEstaVacia()) {
		sem_post(&destruccionMensaje);
	}
	sem_post(&mutexLista);
}

void cerrarPrograma(Programa* programa) {
	informacionPrograma(programa);
	eliminarDeListaDeProgramas(programa);
	liberar(programa);
	sem_post(&destruccionMensaje);
}

//----------------------------------FUNCIONES HILO RECEPTOR-----------------------------------

void escuchandoKernel() {
	pthread_detach(pthread_self());
	int estado = ACTIVADO;
	while(estado != DESACTIVADO) {
		sem_wait(&mutexControl);
		Mensaje* buffer = lRecv(kernel);
		sem_wait(&mutexMensaje);
		mensaje = buffer;
		sem_post(&mutexMensaje);
		Programa* programa = NULL;
		sem_wait(&mutexMensaje);
		switch(tipoOperacion()) {
		case NUEVO_MENSAJE:
		{
			int pid, len;
			memcpy(&pid, mensaje->data,sizeof(int));
			memcpy(&len, mensaje->data+sizeof(int), sizeof(int));
			char* msg = malloc(len);
			memcpy(msg, mensaje->data+sizeof(int)*2, len);
			mensaje->data = realloc(mensaje->data, len);
			memcpy(mensaje->data, msg, len);
			free(msg);
			programa = buscarProgramaPorPidNumerico(pid);
			sem_post(&programa->semaforo);

			break;
		}
		case NUEVO_PID: sem_post(&nuevoMensaje); break;
		case CERRAR_PROCESO:
			sem_wait(&mutexLista);
			programa = buscarProgramaPorPidNumerico(*(int*)mensaje->data);
			sem_post(&mutexLista);
			sem_post(&programa->semaforo);
			break;
		case ABORTAR_PROCESO: abortar(); estado =DESACTIVADO; break;
		case SIN_ESPACIO: sem_post(&nuevoMensaje); sem_post(&destruccionMensaje); break;
		case LIMITE_MULTIPROGRAMACION : mensajeMultiprogramacion(); sem_post(&destruccionMensaje); break;
		case FINALIZAR: estado = DESACTIVADO; sem_post(&destruccionMensaje) ;break;
		case ERROR: finalizarPorDesconexion(); estado = DESACTIVADO; sem_post(&destruccionMensaje); break;
		}
		sem_post(&mutexMensaje);
		sem_wait(&destruccionMensaje);
		destruirMensaje(buffer);
		sem_post(&mutexControl);
	}
	sem_post(&finalizacionHiloReceptor);
}


//-----------------------------------FUNCIONES AUXILIARES---------------------------------


void eliminarDeListaDeProgramas(Programa* programa) {
	int i = 0;
	sem_wait(&mutexLista);
	for(i=0; programa != (Programa*)list_get(listaDeProgramas, i); i++);
	list_remove(listaDeProgramas, i);
	sem_post(&mutexLista);
}

FILE* abrirArchivo(char* path) {
	FILE* archivo = fopen(path, "r");
	return archivo;
}

int archivoValido(FILE* archivo) {
	return archivo != NULL;
}

void cerrarArchivo(FILE* archivo) {
	fclose(archivo);
}

void iniciarSemaforo(sem_t* semaforo, unsigned int valor) {
	sem_init(semaforo, 0, valor);
}

void agregarAListaDeProgramas(Programa* programa) {
	sem_wait(&mutexLista);
	list_add(listaDeProgramas, programa);
	sem_post(&mutexLista);
}

int pidEnviadoPorKernel() {
	return *(int*)mensaje->data;
}

int tipoOperacion() {
	return mensaje->header.tipoOperacion;
}


void vaciarListaDeProgramas() {
	list_clean_and_destroy_elements(listaDeProgramas, liberar);
}

bool listaDeProgramasEstaVacia() {
	return list_is_empty(listaDeProgramas);
}

void enviarAlKernel(int* pid, int operacion) {
	lSend(kernel, pid, operacion, sizeof(int));
}

bool existeElPrograma(Programa* programa) {
	return programa != NULL;
}

int* obtenerPid(Programa* programa) {
	return &programa->pid;
}


Programa* buscarProgramaPorPid(char* pid) {

	bool buscarPorPid(void* unPrograma) {
			char* bufferPid = NULL;
			Programa* programa = (Programa*)unPrograma;
			bufferPid = string_itoa(programa->pid);
			int cumple = sonIguales(bufferPid, pid);
			free(bufferPid);
			return cumple;
	}

	Programa* programa =(Programa*)list_find(listaDeProgramas, buscarPorPid);
	int i;
	for(i=0; programa != (Programa*)list_get(listaDeProgramas, i); i++);
	if(programa != NULL) {
		return programa;
	}
	else {
		return NULL;
	}
}


Programa* buscarProgramaPorPidNumerico(int pid) {

	bool buscarPorPid(void* unPrograma) {
			Programa* programa = (Programa*)unPrograma;
			return programa->pid == pid;
	}

	Programa* programa =(Programa*)list_find(listaDeProgramas, buscarPorPid);
	int i;
	for(i=0; programa != (Programa*)list_get(listaDeProgramas, i); i++);
	if(programa != NULL) {
		return programa;
	}
	else {
		return NULL;
	}
}

void cancelarHilosPrograma() {
	int j;
	for(j=0; j<list_size(listaDeProgramas); j++) {
		pthread_cancel(((Programa*)(list_get(listaDeProgramas, j)))->hiloPrograma);
	}
}

void liberar(void* algo) {
	free(algo);
}


Instruccion obtenerInstruccion() {
	Instruccion instruccion;
	char* mensaje = leerCaracteresEntrantes();
	char* comando = obtenerComandoDe(mensaje);
	instruccion.comando = identificarComando(comando);
	if(instruccion.comando != ERROR) {
		if(elComandoLlevaParametros(comando)) {
			char* argumento = obtenerArgumentoDe(mensaje);
			strcpy(instruccion.argumento, argumento);
			if(!sonIguales(argumento, ""))
				free(argumento);
		}
	} else
		mensajeErrorComando();
	free(comando);
	free(mensaje);
	return instruccion;
}

int identificarComando(char* comando) {
	if(sonIguales(comando, C_RUN))
		return RUN;
	else if(sonIguales(comando, C_CLOSE))
		return CLOSE;
	else if(sonIguales(comando, C_EXIT))
		return EXIT;
	else if(sonIguales(comando, C_CLEAR))
		return CLEAR;
	else if(sonIguales(comando, C_LIST))
			return LIST;
	else if(sonIguales(comando, C_HELP))
			return HELP;
	else
		return ERROR;
}

void iniciarSemaforosDeControl() {
	iniciarSemaforo(&destruccionMensaje,0);
	iniciarSemaforo(&finalizacionHiloReceptor,0);
	iniciarSemaforo(&nuevoMensaje,0);
	iniciarSemaforo(&mutexLista, 1);
	iniciarSemaforo(&mutexMensaje, 1);
	iniciarSemaforo(&mutexTiempo, 1);
	iniciarSemaforo(&mutexControl, 1);
	iniciarSemaforo(&mutexOutput, 1);
}

void finalizarPorDesconexion(mensaje) {
	cancelarHilosPrograma();
	mensajeErrorConexion();
	puts("\nApreta enter para salir");
	desconectar();
	//sleep(1);
	//exit(EXIT_FAILURE);
}

void conectarConKernel() {
	kernel = getConnectedSocket2(config->ip_kernel, config->puerto_kernel, CONSOLA_ID);
	mensajeConsolaConectada();
}

void crearListaDeProgramas() {
	listaDeProgramas = list_create();
}

void crearHiloReceptor() {
	pthread_t receptor;
	pthread_create(&receptor, NULL, (void *) escuchandoKernel, NULL);
}

void destruirListaDeProgramas() {
	sem_wait(&mutexLista);
	list_destroy(listaDeProgramas);
	sem_post(&mutexLista);
}

int sonIguales(char* s1, char* s2) {
	if (strcmp(s1, s2) == 0)
		return 1;
	else
		return 0;
}

void imprimirMensaje(int pid, char* data) {
	sem_wait(&mutexOutput);
	printf("[PID: %i]: MENSAJE: %s\n", pid, data);
	sem_post(&mutexOutput);
}

void mensajeInicioPrograma(int pid) {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	printf("PID RECIBIDO: %i\n", pid);
	puts("ESPERANDO IMPRESIONES POR PANTALLA...");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&destruccionMensaje);
	sem_post(&mutexOutput);
}

void mensajeContenidoArchivo(char* texto) {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	printf("CONTENIDO DEL ARCHIVO: %s\n", texto);
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

bool operacionDelMensajeEs(int idOperacion) {
	return mensaje->header.tipoOperacion == idOperacion;
}

char* leerArchivo(FILE* archivo) {
	fseek(archivo, 0, SEEK_END);
	long posicion = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *texto = malloc(posicion + 1);
	fread(texto, posicion, 1, archivo);
	texto[posicion] = '\0';
	return texto;
}

void enviarArchivoAlKernel(FILE* archivo) {
	char* texto = leerArchivo(archivo);
	cerrarArchivo(archivo);
	//mensajeContenidoArchivo(texto);
	lSend(kernel, texto, 1, strlen(texto));
	free(texto);
}

void recorrerLista() {
	sem_wait(&mutexLista);
	sem_wait(&mutexOutput);
	int i;
	puts("------------------------------------------------------------");
	for(i=0;i<list_size(listaDeProgramas); i++)
		printf("EL PROCESO %i ESTA EJECUTANDOSE\n", ((Programa*)list_get(listaDeProgramas, i))->pid);
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
	sem_post(&mutexLista);
}

//-----------------------------------FUNCIONES TIEMPO-----------------------------------

time_t obtenerTiempo() {
	time_t tiempo = time(0);
	return tiempo;
}

char* mostrarTiempo(time_t tiempo) {
	struct tm *tlocal = localtime(&tiempo);
	char* fecha = malloc(MAX);
	strftime(fecha,MAX,"%d/%m/%y | %H:%M:%S", tlocal);
	return fecha;
}

//-----------------------------------FUNCIONES COMANDOS-----------------------------------

char* leerCaracteresEntrantes() {
	int i, caracterLeido;
	char* cadena = malloc(MAX);
	for(i = 0; (caracterLeido= getchar()) != '\n'; i++)
		cadena[i] = caracterLeido;
	cadena[i] = '\0';
	return cadena;
}

char* obtenerComandoDe(char* cadenaALeer) {
	int i;
	for(i=0; cadenaALeer[i] != ' ' && cadenaALeer[i] != '\0'; i++);
	char* cadena = string_substring_until(cadenaALeer, i);
	return cadena;
}

char* obtenerArgumentoDe(char* cadenaALeer) {
	char* comando = obtenerComandoDe(cadenaALeer);
 	int indice = strlen(comando);
 	free(comando);
	if(indice == strlen(cadenaALeer)) {
 		return "";
 	}
 	return string_substring_from(cadenaALeer, indice+1);
}

int elComandoLlevaParametros(char* comando) {
	return sonIguales(comando, C_RUN) || sonIguales(comando, C_CLOSE);
}

//-----------------------------------FUNCIONES ARCHIVO CONFIGURACION-----------------------------------

void leerArchivoDeConfiguracion() {
	config = configurate(PATH_CONFIG_FILE, leerArchivoConfig, keys);
}

configFile* leerArchivoConfig(t_config* configHandler) {
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->ip_kernel,config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->puerto_kernel,config_get_string_value(configHandler, "PUERTO_KERNEL"));
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}

void imprimirConfig(configFile* config) {
	sem_wait(&mutexOutput);
	puts("");
	puts("------------------------------------------------------------");
	puts("#PROCESO CONSOLA");
	printf("IP KERNEL: %s\n", config->ip_kernel);
	printf("PUERTO KERNEL: %s\n", config->puerto_kernel);
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

bool esUnArchivo(char* c) {
	return string_contains(c, ".");
}

//-----------------------------------MENSAJES CONSOLA-----------------------------------

void mensajeIngresar() {
	puts("INGRESE UN COMANDO:");
}

void mensajeErrorComando() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ERROR: COMANDO INVALIDO");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);

}

void mensajeConsolaConectada() {
	sem_wait(&mutexOutput);
	puts("CONSOLA CONECTADA");
	puts("------------------------------------------------------------");
	puts("PARA VER LOS COMANDOS INGRESE 'HELP'");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeConsolaDesconectada() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("CONSOLA DESCONECTADA");
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

void mensajeMultiprogramacion() {
	sem_wait(&mutexOutput);
	puts("-----------------------------------------------------------");
	puts("SE ALCANZO EL LIMITE DE MULTIPROGRAMACION, ESPERANDO PID...");
	puts("-----------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeErrorConexion() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ERROR: LA CONEXION FINALIZO");
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

void mensajeErrorArchivo() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ERROR: ARCHIVO O RUTA INVALIDA");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeErrorPid() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ERROR: IDENTIFICADOR DE PROCESO INEXISTENTE");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeDesconectar() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("TODOS LOS PROCESOS HAN SIDO DESCONECTADOS");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeNoHayProcesos() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("NO HAY PROCESOS EN EJECUCION");
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}


void informacionPrograma(Programa* programa) {
	sem_wait(&mutexTiempo);
	sem_wait(&mutexOutput);
	char* tiempoInicio = mostrarTiempo(programa->tiempoInicio);
	char* tiempoFinal = mostrarTiempo(obtenerTiempo());
	puts("------------------------------------------------------------");
	printf("ID PROCESO: %i\n", programa->pid);
	printf("FECHA DE INICIO: %s\n", tiempoInicio);
	printf("FECHA DE FINALIZACION: %s\n", tiempoFinal);
	printf("CANTIDAD DE IMPRESIONES %i\n", programa->impresiones);
	printf("TIEMPO DE EJECUCION: %f SEGUNDOS\n", difftime(obtenerTiempo(), programa->tiempoInicio));
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
	free(tiempoInicio);
	free(tiempoFinal);
	sem_post(&mutexTiempo);
}

void mensajeAyuda() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("RUN <RUTA_ARCHIVO> ---> INICIA UN PROCESO");
	puts("CLOSE <ID_PROCESO> ---> FINALIZA UN PROCESO");
	puts("CLEAR ----------------> LIMPIA LA PANTALLA");
	puts("LIST -----------------> MUESTRA LOS PROCESOS EN EJECUCION");
	puts("EXIT -----------------> DESCONECTA LA CONSOLA");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeEspera() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ESPERANDO RESPUESTA DEL KERNEL...");
	puts("------------------------------------------------------------");
	mensajeIngresar();
	sem_post(&mutexOutput);
}

void mensajeNoHayEspacio() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("ERROR: NO HAY ESPACIO DISPONIBLE EN MEMORIA");
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

void mensajeCerrandoProcesos() {
	sem_wait(&mutexOutput);
	puts("------------------------------------------------------------");
	puts("CERRANDO PROCESOS...");
	puts("------------------------------------------------------------");
	sem_post(&mutexOutput);
}

//--------------PARA QUE NO PUTEE VALGRIND------------------

int enviarHandShake2(int socket, int idPropia) {
	int* idProceso = malloc(sizeof(int));
	*idProceso = idPropia;
	lSend(socket, idProceso, HANDSHAKE,sizeof(int));
	free(idProceso);
	Mensaje* confirmacion = lRecv(socket);
	int conf = confirmacion->header.tipoOperacion != -1 && *((int*)confirmacion->data) != 0;
	destruirMensaje(confirmacion);
	return  conf;
}

int getConnectedSocket2(char* ip, char* port, int idPropia){
	int(*action)(int,const struct sockaddr*,socklen_t)=&connect;
	int socket = internalSocket(ip,port,action);
	if(!enviarHandShake2(socket, idPropia))
		errorIfEqual(0,0,"El servidor no admite conexiones para este proceso");
	return socket;
}
//--------------PARA QUE NO PUTEE VALGRIND------------------
