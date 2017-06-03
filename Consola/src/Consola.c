/*
 ============================================================================
 Name        : Consola.c
 Author      : Dario Poma
 Version     : 1.0
 Copyright   : ---
 Description : Una hermosa consola...
 ============================================================================
 */

#include "Consola.h"

int main(void) {
	leerArchivoDeConfiguracion();
	conectarConKernel();
	atenderPedidos();
	finalizarProceso();
	return EXIT_SUCCESS;
}

void atenderInstrucciones() {
	Instruccion instruccion;
	instruccion.comando = 0;
	mensajeConsola(INGRESAR);
	while(instruccion.comando != EXIT) {
		instruccion = obtenerInstruccion();
		switch(instruccion.comando) {
			case RUN: iniciar(instruccion.argumento); break;
			case CLOSE: cerrar(instruccion.argumento); break;
			case DISCONNECT: desconectar(); break;
			case CLEAR: limpiar(); break;
			case LIST: mostrarLista(); break;
			case HELP: mensajeConsola(AYUDA); break;
			default: break;
		}
	}
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
		mensajeConsola(ERROR_COMANDO);
	free(comando);
	free(mensaje);
	return instruccion;
}

int identificarComando(char* comando) {
	if(sonIguales(comando, C_RUN))
		return RUN;
	else if(sonIguales(comando, C_CLOSE))
		return CLOSE;
	else if(sonIguales(comando, C_DISCONNECT))
		return DISCONNECT;
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

void mensajeConsola(int mensaje) {
	switch(mensaje) {
	case INGRESAR: mensajeIngresar(); break;
	case ERROR_COMANDO: mensajeComandoInvalido(); break;
	case CONSOLA_CONECTADA: mensajeConsolaConectada(); break;
	case ERROR_ARCHIVO: mensajeErrorArchivo(); break;
	case AYUDA: mensajeAyuda(); break;
	case ERROR_PID: mensajeErrorPid(); break;
	case CONSOLA_DESCONECTADA: mensajeConsolaDesconectada(); break;
	case NO_HAY_PROCESOS: mensajeNoHayProcesos(); break;
	case ERROR_CONEXION: mensajeErrorConexion(); break;
	case ESPERA: mensajeEspera(); break;
	}
}


//MENSAJES CONSOLA
void mensajeIngresar() {
	printf("Consola: ");
}

void mensajeComandoInvalido() {
	puts("-------------------------------");
	puts("ERROR: COMANDO INVALIDO");
	puts("-------------------------------");
	mensajeIngresar();
}

void mensajeConsolaConectada() {
	puts("CONSOLA CONECTADA");
	puts("--------------------------------------");
	puts("PARA VER LOS COMANDOS INGRESE 'HELP'");
	puts("--------------------------------------");
}

void mensajeConsolaDesconectada() {
	puts("-------------------------------");
	puts("CONSOLA DESCONECTADA");
	puts("-------------------------------");
}

void mensajeErrorConexion() {
	puts("-------------------------------");
	puts("ERROR: LA CONEXION FINALIZO");
	puts("-------------------------------");
}

void mensajeErrorArchivo() {
	puts("--------------------------------");
	puts("ERROR: ARCHIVO O RUTA INCORRECTA");
	puts("--------------------------------");
	mensajeIngresar();
}

void mensajeErrorPid() {
	puts("--------------------------------------");
	puts("ERROR: IDENTIFICADOR DE PROCESO INEXISTENTE");
	puts("--------------------------------------");
	mensajeIngresar();
}

void mensajeDesconectarTodo() {
	puts("-------------------------------------------");
	puts("TODOS LOS PROCESOS HAN SIDO DESCONECTADOS");
	puts("-------------------------------------------");
	mensajeIngresar();
}

void mensajeNoHayProcesos() {
	puts("----------------------------------");
	puts("NO HAY PROCESOS EJECUTANDOSE");
	puts("----------------------------------");
}


void informacionPrograma(Programa* programa) {
	char* tiempoInicio = mostrarTiempo(programa->tiempoInicio);
	char* tiempoFinal = mostrarTiempo(obtenerTiempo());
	puts("----------------------------------------");
	printf("ID PROCESO: %i\n", programa->pid);
	printf("FECHA DE INICIO: %s\n", tiempoInicio);
	printf("FECHA DE FINALIZACION: %s\n", tiempoFinal);
	printf("TIEMPO DE EJECUCION: %f SEGUNDOS\n", difftime(obtenerTiempo(), programa->tiempoInicio));
	puts("----------------------------------------");
	free(tiempoInicio);
	free(tiempoFinal);
}

void mensajeAyuda() {
	puts("------------------------------------------------------------");
	puts("RUN <RUTA_ARCHIVO> ---> INICIA UN PROCESO");
	puts("CLOSE <ID_PROCESO> ---> FINALIZA UN PROCESO");
	puts("DISCONNECT -----------> DESCONECTA TODOS LOS PROCESOS");
	puts("CLEAR ----------------> LIMPIA LA PANTALLA");
	puts("LIST -----------------> MUESTRA LOS PROCESOS EN EJECUCION");
	puts("EXIT -----------------> SALIR DEL PROGRAMA");
	puts("------------------------------------------------------------");
	mensajeIngresar();
}

void mensajeEspera() {
	puts("------------------------------------------------------------");
	puts("ESPERANDO RESPUESTA DEL KERNEL...");
	puts("------------------------------------------------------------");
}

void mensajeNoHayEspacio() {
	puts("ERROR: NO HAY ESPACIO DISPONIBLE EN MEMORIA");
	puts("------------------------------------------------------------");
	mensajeIngresar();
}


void iniciarEstructuraPrograma(char* path) {
	Programa* programa = malloc(sizeof(Programa));
	list_add(listaDeProgramas, programa);
	programa->tiempoInicio = obtenerTiempo();
	strcpy(programa->path, path);
	pthread_create(&(programa->hiloPrograma), NULL, (void *) hiloPrograma, programa);
}

void iniciar(char* path) {
	enviarArchivoAlKernel(path);
	sem_wait(&espera);
	if(respuesta->header.tipoOperacion != -2) {
		iniciarEstructuraPrograma(path);
	} else
		mensajeNoHayEspacio();
}


void enviarArchivoAlKernel(char* path) {
	FILE* archivo = fopen(path, "r");
	if(archivo != NULL) {
		char* texto = leerArchivo(archivo);
		fclose(archivo);
		puts("------------------------------------------");
		printf("CONTENIDO DEL ARCHIVO: %s\n", texto);
		puts("------------------------------------------");
		lSend(kernel, texto, 1, strlen(texto));
		free(texto);
	}
	else {
		mensajeConsola(ERROR_ARCHIVO);
	}
}

void hiloPrograma(Programa* programa) {
	pthread_detach(pthread_self());
	iniciarPrograma();
	esperarMensajes(programa);
}

void iniciarSemaforo(Programa* programa) {
	sem_init(&programa->semaforo,0,0);
}

void esperarMensajes(Programa* programa) {
	iniciarSemaforo(programa);
	int estado = 1;
	int i = 0;
	while(estado!=0) {
		sem_wait(&programa->semaforo);
		switch(respuesta->header.tipoOperacion) {
			case 1: imprimirMensajeKernel(respuesta->data); break;
			case 9:
				sem_wait(&espera);
				estado = 0;
				for(i=0; programa != (Programa*)list_get(listaDeProgramas, i); i++);
				list_remove(listaDeProgramas, i);
				free(programa);
				sem_post(&liberarMensaje);
				break;
			case -1: finalizarConsolaPorDesconexion(); break;
		}
	}
}

void escuchandoKernel() {
	pthread_detach(pthread_self());
	int estado = 1;
	sem_init(&espera,0,0);
	sem_init(&liberarPrograma, 0, 0);
	sem_init(&final, 0, 0);
	while(estado!=0) {
		Mensaje* mensaje = lRecv(kernel);
		respuesta = mensaje;
		Programa* programa = NULL;
		switch(respuesta->header.tipoOperacion) {
		case 1: printf("%s\n", (char*)mensaje->data); break;
		case 2: sem_post(&espera); break;
		case 9:
			programa = buscarProgramaPorPidNumerico(*(int*)respuesta->data);
			sem_post(&programa->semaforo);
			informacionPrograma(programa);
			sem_post(&espera);
			break;
		case 3: estado = 0;sem_post(&liberarMensaje) ;break;
		case -2: sem_post(&espera); break;
		}
		sem_wait(&liberarMensaje);
		destruirMensaje(mensaje);
	}
	sem_post(&final);
}

void iniciarPrograma() {
	memcpy(&(((Programa*)list_get(listaDeProgramas, list_size(listaDeProgramas)-1))->pid), respuesta->data, sizeof(int));
	printf("PID RECIBIDO: %i\n", *(int*)respuesta->data);
	puts("ESPERANDO IMPRESIONES POR PANTALLA...");
	puts("------------------------------------------------------------");
	sem_post(&liberarMensaje);
}


void imprimirMensajeKernel(char* data) {
	printf("Mensaje: %s\n", data);
}

void finalizarConsolaPorDesconexion(mensaje) {
	mensajeConsola(ERROR_CONEXION);
	exit(EXIT_FAILURE);
}

void noHayEspacio() {
	mensajeNoHayEspacio();
	pthread_exit(NULL);
}

void cerrar(char* pidPrograma) {
	Programa* programa = buscarProgramaPorPid(pidPrograma);

	if(programa == NULL)
		mensajeConsola(ERROR_PID);
	else {
		lSend(kernel, &programa->pid, 9, sizeof(int));
	}
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


void mostrarLista() {
	if(list_is_empty(listaDeProgramas))
		mensajeConsola(NO_HAY_PROCESOS);
	else
		recorrerLista();
	mensajeConsola(INGRESAR);
}

void recorrerLista() {
	int i;
	puts("---------------------------------------------");
	for(i=0;i<list_size(listaDeProgramas); i++)
		printf("EL PID DEL PROCESO N°%i ES: %i\n", i, ((Programa*)list_get(listaDeProgramas, i))->pid);
	puts("----------------------------------------------");
}

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

void leerArchivoDeConfiguracion() {
	config = configurate(PATH_CONFIG_FILE, leerArchivoConfig, keys);
}


//--------------PARA QUE NO PUTEE VALGRIND------------------
int enviarHandShake2(int socket, int idPropia)
{
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

void conectarConKernel() {
	kernel = getConnectedSocket2(config->ip_kernel, config->puerto_kernel, CONSOLA_ID);
	mensajeConsola(CONSOLA_CONECTADA);
}


void atenderPedidos() {
	listaDeProgramas = list_create();
	pthread_create(&escucha, NULL, (void *) escuchandoKernel, NULL);
	atenderInstrucciones();
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

void imprimirConfig(configFile* config) {
	puts("");
	puts("-------------------------------");
	puts("#PROCESO CONSOLA");
	printf("IP KERNEL: %s\n", config->ip_kernel);
	printf("PUERTO KERNEL: %s\n", config->puerto_kernel);
	puts("-------------------------------");
}

configFile* leerArchivoConfig(t_config* configHandler) {
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->ip_kernel,config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->puerto_kernel,config_get_string_value(configHandler, "PUERTO_KERNEL"));
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}

void desconectar() {

	void liberar(void* algo) {
		free(algo);
	}

	int j;

	for(j=0; j<list_size(listaDeProgramas); j++) {
		pthread_cancel(((Programa*)(list_get(listaDeProgramas, j)))->hiloPrograma);
	}
	int* i = malloc(sizeof(int));
	*i = 0;
	lSend(kernel, i, 2, sizeof(int));
	free(i);
	mensajeConsola(CONSOLA_DESCONECTADA);
	list_clean_and_destroy_elements(listaDeProgramas, liberar);
		mensajeConsola(INGRESAR);
}

void finalizarProceso() {
	desconectar();
	list_destroy(listaDeProgramas);
	int a = 0;
	lSend(kernel, &a, 3, sizeof(int));
	sem_wait(&final);
	free(config);
	close(kernel);
	sem_destroy(&espera);
	sem_destroy(&liberarMensaje);
	sem_destroy(&liberarPrograma);
	sem_destroy(&final);
}

void limpiar() {
	system("clear");
	mensajeConsola(INGRESAR);
}

int sonIguales(char* s1, char* s2) {
	if (strcmp(s1, s2) == 0)
		return 1;
	else
		return 0;
}
