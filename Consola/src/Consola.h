//------------------------------BIBLIOTECAS------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/txt.h>
#include <commons/temporal.h>
#include <commons/collections/list.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
#include  <signal.h>

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

//Conexion consola-kernel
#define SIN_ESPACIO -2
#define LIMITE_MULTIPROGRAMACION -3
#define NUEVO_MENSAJE 1
#define NUEVO_PID 2
#define ABORTAR_PROCESO 3
#define FINALIZAR 4
#define CERRAR_PROCESO 9


//Otros
#define MAX 1000
#define ERROR -1
#define DESACTIVADO 0
#define ACTIVADO 1

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
	int impresiones;
	char path[MAX];
	time_t tiempoInicio;
	pthread_t hiloPrograma;
	sem_t semaforo;
} Programa;

//------------------------------VARIABLES GLOBALES------------------------------

int kernel; //Socket del kernel
configFile* config; //Para levantar el configFile
t_list* listaDeProgramas; //Lista de los programas
const char* keys[3] = { "IP_KERNEL", "PUERTO_KERNEL", "NULL" }; //Campos del archivo de configuracion
int nada = 0; //Cuando no tengo nada que enviarle al kernel
Mensaje* mensaje = NULL; //Para que pueden acceder al mensaje mis hermosos hilos
int morir = 0;
//------------------------------SEMAFOROS------------------------------

sem_t nuevoMensaje;
sem_t finalizacionHiloReceptor;
sem_t destruccionMensaje;
sem_t mutexLista;
sem_t mutexMensaje;
sem_t mutexControl;
sem_t mutexTiempo;
sem_t mutexOutput;

//------------------------------FUNCIONES------------------------------

void handleExit(int sig);
void leerArchivoDeConfiguracion();
void conectarConKernel();
void atenderPedidos();
void finalizarProceso();
void atenderInstrucciones();
Instruccion obtenerInstruccion();
void iniciar(char*);
void cerrar(char*);
void desconectar();
void limpiar();
char* leerCaracteresEntrantes();
char* obtenerComandoDe(char*);
char* obtenerArgumentoDe(char*);
int identificarComando(char*);
int elComandoLlevaParametros(char*);
void conexionKernel(Programa*);
char* leerArchivo(FILE*);
void recibirRespuesta();
configFile* leerArchivoConfig(t_config*);
void imprimirConfig(configFile*);
int sonIguales(char*, char*);
Programa* buscarProgramaPorPid(char*);
bool buscarPorPid(void*, char*);
void mensajeConsola();
void mostrarLista();
void mensajeAyuda();
void mensajeIngresar();
void mensajeConsolaConectada();
void mensajeErrorComando();
void mensajeConsolaDesconectada();
void mensajeErrorArchivo();
void mensajeErrorPid();
void mensajeNoHayProcesos();
void mensajeErrorConexion();
void mensajeEspera();
time_t obtenerTiempo();
char* mostrarTiempo(time_t);
void recorrerLista();
void enviarArchivoAlKernel(FILE*);
void imprimirMensajeKernel(char*);
void hiloPrograma(Programa*);
void finalizarConsolaPorDesconexion();
void mensajeNoHayEspacio();
void esperarMensajes(Programa*);
void iniciarPrograma();
void noHayEspacio();
Programa* buscarProgramaPorPidNumerico(int pid);
void informacionPrograma(Programa* programa);
void crearHiloPrograma();
void mensajeInicioPrograma(int pid);
void iniciarSemaforo(sem_t* semaforo, unsigned int valor);
void cerrarPrograma(Programa*);
void imprimirMensaje(int pid, char* data);
int getConnectedSocket2(char* ip, char* port, int idPropia);
void finalizarPorDesconexion();
int archivoValido(FILE* archivo);
FILE* abrirArchivo(char* path);
void esperar(sem_t* semaforo);
void activar(sem_t* semaforo);
void cancelarHilosPrograma();
int* obtenerPid(Programa* programa);
bool existeElPrograma(Programa* programa);
void vaciarListaDeProgramas();
void enviarAlKernel(int* pid, int operacion);
void liberar(void* algo);
bool listaDeProgramasEstaVacia();
bool operacionDelMensajeEs(int idOperacion);
int pidEnviadoPorKernel();
void agregarAListaDeProgramas(Programa* programa);
void eliminarDeListaDeProgramas(Programa* programa);
int tipoOperacion();
void crearListaDeProgramas();
void crearHiloReceptor();
void destruirListaDeProgramas();
void iniciarSemaforosDeControl();
void desconectarPrograma(Programa* programa);
void mensajeCerrandoProcesos();
void realizarDesconexion();
void informarDesconexion();
bool esUnArchivo(char*);
void mensajeMultiprogramacion();
