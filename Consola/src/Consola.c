/*
 ============================================================================
 Name        : Consola.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/collections/list.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
#define CONFIG_FILE "consola.conf"
#define C_INICIAR "run"
#define C_FINALIZAR "close"
#define C_DESCONECTAR "exit"
#define C_LIMPIAR "clear"
#define INICIAR 1
#define FINALIZAR 2
#define DESCONECTAR 3
#define LIMPIAR 4
#define ERROR -1

int kernel; //Socket
int contadorPrograma;

const char* keys[3] = { "IP_KERNEL", "PUERTO_KERNEL", "NULL" };

typedef struct {
	char ip_kernel[16];
	char puerto_kernel[5];

} configFile;


typedef struct {
	int id;
	char* path;
} Comando;

void imprimirConfig(configFile* config) {
	puts("-------------------------------");
	puts("#PROCESO CONSOLA");
	printf("IP KERNEL: %s\n", config->ip_kernel);
	printf("PUERTO KERNEL: %s\n", config->puerto_kernel);
	puts("-------------------------------");
}

configFile* leerArchivoConfig(t_config* configHandler) {
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->ip_kernel,
			config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->puerto_kernel,
			config_get_string_value(configHandler, "PUERTO_KERNEL"));
	config_destroy(configHandler);
	imprimirConfig(config);

	return config;
}

int sonIguales(char* s1, char* s2) {
	if (strcmp(s1, s2) == 0)
		return 1;
	else
		return 0;
}


char* leerArchivo(FILE *archivo) {
	fseek(archivo, 0, SEEK_END);
	long fsize = ftell(archivo);
	fseek(archivo, 0, SEEK_SET);
	char *script = malloc(fsize + 1);
	fread(script, fsize, 1, archivo);
	script[fsize] = '\0';
	return script;
}

void iniciarPrograma(FILE* archivo) {
	char* texto = leerArchivo(archivo);
	contadorPrograma++;
	printf("Hilo %i creado\n", contadorPrograma);
	printf("Aca esta el texto: %s\n", texto);
	lSend(kernel, texto, 1, strlen(texto));
	puts("archivo enviado, esperando respuesta...");
	Mensaje* mensaje = lRecv(kernel);
	switch(mensaje->header.tipoOperacion)
	{
		case -1:
			puts("La conexion murio");
			break;
		case 1:
			printf("Mensaje: %s\n", mensaje->data);
			break;
	}


}




void finalizar() {

}
void limpiar() {
}
void desconectar() {
}




int identificarComando(char* comando) {
	if (sonIguales(comando, C_INICIAR))
		return INICIAR;
	else if (sonIguales(comando, C_FINALIZAR))
		return FINALIZAR;
	else if (sonIguales(comando, C_DESCONECTAR))
		return DESCONECTAR;
	else if (sonIguales(comando, C_LIMPIAR))
		return LIMPIAR;
	else
		return ERROR;
}

void verificarComando(int idComando) {
	if (idComando > 0)
		puts("Comando reconocido");
	else
		puts("Comando inexistente");
}

Comando controlarComando() {
	Comando comando;
	int caracter, i;
	//No hay argumento
	int flagArg = 0;
	char mensaje[1000];
	char instruccion[10];
	//Con enter se envia el comando
	for (i = 0; (caracter = getchar()) != '\n'; i++) {
		switch (caracter) {
		//Con espacio separo la instruccion del argumento
		case ' ': {
			mensaje[i] = '\0';
			//Me guardo la instruccion
			strcpy(instruccion, mensaje);
			//Para que empiece a guardar el path en 'mensaje'
			i = -1;
			//Hay argumento
			flagArg = 1;
		}
			break;
		default:
			//Voy guardando los caracteres
			mensaje[i] = caracter;
		}
	}
	//Por si alguien se olvido de poner el path (no apreto espacio)
	if (flagArg == 0) {
		mensaje[i] = '\0';
		strcpy(instruccion, mensaje);
	}

	mensaje[i] = '\0';
	//Me guardo el tipo de comando
	comando.id = identificarComando(instruccion);
	//Me guardo el path
	comando.path = mensaje;
	//Verifico que exista el comando
	verificarComando(comando.id);
	//TESTING
	printf("El path es: %s\n", comando.path);
	return comando;
}


void iniciar(char* path) {
	FILE* archivo = fopen(path, "r");
	pthread_t programa;
	pthread_create(&programa, NULL, (void *) iniciarPrograma, archivo);
}

void atenderMensajes() {
	//Para que entre al while
	int instruccion = 0;
	//Finaliza con el comando 'exit'
	while (instruccion != DESCONECTAR) {
		Comando comando;
		puts("-------------------------------");
		printf("Ingrese un comando: ");
		//Me devuelve el tipo de comando y el argumento (path)
		comando = controlarComando();
		//Verifico el tipo de comando
		switch(comando.id) {
			case INICIAR: iniciar(comando.path); break;
			case FINALIZAR: finalizar(); break;
			case LIMPIAR: limpiar(); break;
			case DESCONECTAR: desconectar(); break;
		}
	}
	puts("-------------------------------");
	printf("Consola desconectada.\n");
	printf("\n");

}



int main(void) {
	configFile* config;
	config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Consola/Debug/consola.conf",leerArchivoConfig, keys);
	kernel = getConnectedSocket(config->ip_kernel, config->puerto_kernel,CONSOLA_ID);
	printf("Consola conectada.\n");
	pthread_t usuario;
	pthread_create(&usuario, NULL, (void *) atenderMensajes, NULL);
	pthread_join(usuario, NULL);
	free(config);
	close(kernel);
	return 0;
}

