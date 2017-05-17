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
	char mensaje[MAX];
	char comando[MAX];
	strcpy(mensaje, leerCaracteresEntrantes());
	strcpy(comando, obtenerComandoDe(mensaje));
	instruccion.comando = identificarComando(comando);
	if(instruccion.comando != ERROR) {
		if(elComandoLlevaParametros(comando))
			strcpy(instruccion.argumento, obtenerArgumentoDe(mensaje));
	} else
		mensajeConsola(ERROR_COMANDO);
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
	puts("----------------------------------------");
	printf("ID PROCESO: %i\n", programa->pid);
	printf("FECHA DE INICIO: %s\n", mostrarTiempo(programa->tiempoInicio));
	printf("FECHA DE FINALIZACION: %s\n", mostrarTiempo(obtenerTiempo()));
	printf("TIEMPO DE EJECUCION: %f SEGUNDOS\n", difftime(obtenerTiempo(), programa->tiempoInicio));
	puts("----------------------------------------");
	mensajeIngresar();

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


//FUNCIONES NO DECLARATIVAS

//DELEGAR
void recibirRespuesta(Programa* programa) {
	int estado = 1;
	while(estado != 0) {
		//NO SE QUE PORQUE DESPUES DEL RECV EL PUNTERO CAMBIA MAGICAMENTE
		//printf("%p", programa);
		Mensaje* mensaje = lRecv(kernel);
		//printf("%p", programa);
		//printf("RECUPERO EL PUNTERO %p, ", (Programa*)list_get(listaDeProgramas, list_size(listaDeProgramas)-1));
		switch(mensaje->header.tipoOperacion) {
			case -1:
				mensajeConsola(ERROR_CONEXION);
				exit(EXIT_FAILURE);
				break;
			case 1:
				printf("Mensaje: %s\n", mensaje->data);
				break;
			case 2:
				memcpy(&(((Programa*)list_get(listaDeProgramas, list_size(listaDeProgramas)-1))->pid), mensaje->data, sizeof(int));
				printf("PID RECIBIDO: %i\n", ((Programa*)list_get(listaDeProgramas, list_size(listaDeProgramas)-1))->pid);
				puts("ESPERANDO IMPRESIONES POR PANTALLA...");
				puts("------------------------------------------");
				break;

		}
		destruirMensaje(mensaje);
	}
}


void iniciar(char* path) {
	FILE* archivo = fopen(path, "r");
	if(archivo != NULL) {
		Programa* programa = malloc(sizeof(Programa));
		programa->tiempoInicio = obtenerTiempo();
		programa->archivo = archivo;
		pthread_create(&(programa->hiloPrograma), NULL, (void *) conexionKernel, programa);
		list_add(listaDeProgramas, programa);
	}
	else
		mensajeConsola(ERROR_ARCHIVO);
}


void cerrar(char* pidPrograma) {

	Programa* programa = buscarProgramaPorPid(pidPrograma);
	if(programa == NULL)
		mensajeConsola(ERROR_PID);
	else {
		/*lSend(kernel, &programa->pid, 2, sizeof(int));
		mensajeEspera();
		Mensaje* mensaje = lRecv(kernel);
		switch(mensaje->header.tipoOperacion) {
			case -1:
				mensajeConsola(ERROR_CONEXION);
				exit(EXIT_FAILURE);
				break;
			case 1:
				break;
		}
		destruirMensaje(mensaje);
		*/
		//ESTO IRIA EN CASE 1
		informacionPrograma(programa);
		free(programa);
	}
}

void desconectar() {
	/*
	int i = -1;
	lSend(kernel, &i, 2, sizeof(int));
	mensajeEspera();
	Mensaje* mensaje = lRecv(kernel);
	switch(mensaje->header.tipoOperacion) {
		case -1:
			mensajeConsola(ERROR_CONEXION);
			exit(EXIT_FAILURE);
			break;
		case 1:
			//printf("Mensaje: %s\n", mensaje->data);

			break;
	}
	destruirMensaje(mensaje);
	*/
	//ESTO IRIA EN CASE 1
	mensajeConsola(CONSOLA_DESCONECTADA);
	mensajeConsola(INGRESAR);
}


Programa* buscarProgramaPorPid(char* pid) {

	bool buscarPorPid(void* unPrograma) {
			Programa* programa = (Programa*)unPrograma;
			return sonIguales(string_itoa(programa->pid), pid);
	}

	Programa* programa =(Programa*)list_find(listaDeProgramas, buscarPorPid);
	int i;
	for(i=0; programa != (Programa*)list_get(listaDeProgramas, i); i++);
	if(programa != NULL) {
		list_remove(listaDeProgramas, i);
		return programa;
	}
	else
		return NULL;
}


void conexionKernel(Programa* programa) {
	char* texto = leerArchivo(programa->archivo);
	puts("------------------------------------------");
	printf("CONTENIDO DEL ARCHIVO: %s\n", texto);
	lSend(kernel, texto, 1, strlen(texto));
	recibirRespuesta(programa);
}


void mostrarLista() {
	int i;
	if(list_is_empty(listaDeProgramas))
		mensajeConsola(NO_HAY_PROCESOS);
	else {
		puts("---------------------------------------------");
		for(i=0;i<list_size(listaDeProgramas); i++)
			printf("EL PID DEL PROCESO N°%i ES: %i\n", i, ((Programa*)list_get(listaDeProgramas, i))->pid);
		puts("----------------------------------------------");
	}
	mensajeConsola(INGRESAR);
}

time_t obtenerTiempo() {
	time_t tiempo = time(0);
	return tiempo;
}

char* mostrarTiempo(time_t tiempo) {
	struct tm *tlocal = localtime(&tiempo);
	char fecha[MAX];
	strftime(fecha,MAX,"%d/%m/%y | %H:%M:%S", tlocal);
	return fecha;
}

char* leerCaracteresEntrantes() {
	int i, caracterLeido;
	char cadena[MAX];
	for(i = 0; (caracterLeido= getchar()) != '\n'; i++)
		cadena[i] = caracterLeido;
	cadena[i] = '\0';
	return cadena;
}

char* obtenerComandoDe(char* cadenaALeer) {
	int i;
	char cadena[MAX];
	for(i=0; cadenaALeer[i] != ' ' && cadenaALeer[i] != '\0'; i++);
	strcpy(cadena, string_substring_until(cadenaALeer, i));
	return cadena;
}

char* obtenerArgumentoDe(char* cadenaALeer) {
	char comando[MAX];
	strcpy(comando, obtenerComandoDe(cadenaALeer));
 	int indice = strlen(comando);
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

void conectarConKernel() {
	kernel = getConnectedSocket(config->ip_kernel, config->puerto_kernel, CONSOLA_ID);
	mensajeConsola(CONSOLA_CONECTADA);
}

void atenderPedidos() {
	listaDeProgramas = list_create();
	pthread_t usuario;
	pthread_create(&usuario, NULL, (void *) atenderInstrucciones, NULL);
	pthread_join(usuario, NULL);
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


//LA DECLARATIVIDAD NO SE MANCHA

void finalizarProceso() {
	free(config);
	close(kernel);
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

