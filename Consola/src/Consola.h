//------------------------------BIBLIOTECAS------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"

//------------------------------CONSTANTES------------------------------

//Archivo de configuracion
#define CONFIG_FILE "consola.conf"
#define PATH_CONFIG_FILE "/home/utnso/Escritorio/tp-2017-1c-The-Kernels/Consola/Debug/consola.conf"

//Cadena identificadora de cada comando
#define C_RUN "run"
#define C_CLOSE "close"
#define C_DISCONNECT "disconnect"
#define C_CLEAR "clear"
#define C_LIST "list"
#define C_HELP "help"
#define C_EXIT "exit"

//Identificador de cada comando
#define RUN 1
#define CLOSE 2
#define DISCONNECT 3
#define CLEAR 4
#define LIST 5
#define HELP 6
#define EXIT 7

//Mensajes de la consola
#define INGRESAR 11
#define CONSOLA_CONECTADA 12
#define SALIR 13
#define ERROR_COMANDO 14
#define ERROR_ARCHIVO 15
#define ERROR_PID 16
#define AYUDA 17
#define CONSOLA_DESCONECTADA 18
#define NO_HAY_PROCESOS 19
#define ERROR_CONEXION 20
#define ESPERA 21

//Otros
#define MAX 1000
#define ERROR -1

//------------------------------ESTRUCTURAS------------------------------

typedef struct {
	char ip_kernel[16];
	char puerto_kernel[5];
} configFile;

//Instruccion = Comando + Argumento
typedef struct {
	int comando;
	char argumento[MAX];
} Instruccion;

typedef struct {
	int pid;
	FILE* archivo;
	time_t tiempoInicio;
	pthread_t hiloPrograma;
} Programa;

//------------------------------VARIABLES GLOBALES------------------------------

int kernel; //Socket del kernel
configFile* config; //Para levantar el configFile
t_list* listaDeProgramas; //Lista de los programas
const char* keys[3] = { "IP_KERNEL", "PUERTO_KERNEL", "NULL" }; //Campos del archivo de configuracion
int gg;

//------------------------------FUNCIONES------------------------------

void leerArchivoDeConfiguracion(); //Levanta el archivo de configuracion y lo muestra en pantalla
void conectarConKernel(); //Conecta el socket y muestra un lindo mensajito, mira como esta esa delegacion papa
void atenderPedidos(); //Atiende todo lo que tiren
void finalizarProceso(); //Libera recursos y desconecta el socket
void atenderInstrucciones(); //Da inicio a la rutina del comando ingresado, el corazon del equipo
Instruccion obtenerInstruccion(); //Obtiene la instruccion ingresada
void iniciar(char*); //Inicia un programa creando su respectivo hilo
void cerrar(char*); //Finaliza el programa
void desconectar(); //Desconecta la consola matando a todos los programas
void limpiar(); //Limpia la pantalla
char* leerCaracteresEntrantes(); //Lee los caracteres de la consola con el mistico getchar, gracias solá
char* obtenerComandoDe(char*); //Obtiene el comando de la instruccion
char* obtenerArgumentoDe(char*); //Obtiene el parametro de la instruccion
int identificarComando(char*); //Identifica el comando ingresado y devuelve su id
int elComandoLlevaParametros(char*); //Verifica si el comando recibe argumentos
void conexionKernel(Programa*); //Envia el archivo al kernel y espera sus respuestas
char* leerArchivo(FILE*); //Lee el archivo magicamente y lo mete en una cadena
void recibirRespuesta(); //Recibe mensajes del kernel
configFile* leerArchivoConfig(t_config*); //Lee el archivo de configuracion
void imprimirConfig(configFile*); //Imprime el archivo de configuracion
int sonIguales(char*, char*); //Compara dos cadenas, no me gusta el strcmp no es expresivo
Programa* buscarProgramaPorPid(char*); //Busca el programa con el pid pasado
bool buscarPorPid(void*, char*); //Para el list_find
void mensajeConsola(); //Guarda todos los mensajes de la consola
void mostrarLista(); //Muestra los procesos en ejecucion
void mensajeAyuda(); //Muestra los comandos
void mensajeIngresar(); //Mensaje
void mensajeConsolaConectada(); //Mensaje
void mensajeComandoInvalido(); //Mensaje
void mensajeConsolaDesconectada(); //Mensaje
void mensajeErrorArchivo(); //Mensaje
void mensajeErrorPid(); //Mensaje
void mensajeNoHayProcesos(); //Mensaje
void mensajeErrorConexion(); //Mensaje
void mensajeEspera(); //Mensaje
time_t obtenerTiempo(); //Obtiene el tiempo
char* mostrarTiempo(time_t); //Muestra el tiempo
