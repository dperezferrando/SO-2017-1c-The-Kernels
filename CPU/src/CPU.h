
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <pthread.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
#include <signal.h>
#include <unistd.h>


//DEFINES DE MEMORIA

#define DESREFERENCIAR 100
#define LEER 2
#define ESCRIBIR 3

// ESTADOS DEL PROCESO
#define OK 0
#define TERMINO 1 // TERMINO EJECUCION NORMAL (TODAS LAS INSTRUCCIONES)
#define EXPULSADO 2 // FIN DE Q
#define BLOQUEADO 3 // SE BLOQUEO
#define STKOF 4 // HIZO ALGO QUE NO DEBIA (EJ: STACK OVERFLOW)
#define ABORTADO 5
#define KILLED 6

//DEFINES DE KERNEL

#define OBTENERCOMPARTIDA 200
#define ASIGNARCOMPARTIDA 201
#define WAIT 202
#define SIGNAL 203
#define RESERVAR_MEMORIA_HEAP 204
#define LIBERAR_PUNTERO 205
#define ABRIR_ARCHIVO 206
#define BORRAR_ARCHIVO 207
#define CERRAR_ARCHIVO 208
#define MOVER_CURSOR_ARCHIVO 209
#define ESCRIBIR_ARCHIVO 210
#define LEER_ARCHIVO 211

/*
#define ACCESO_ARCHIVO_INEXISTENTE -2
#define LECTURA_ARCHIVO_SIN_PERMISO -3
#define ESCRITURA_ARCHIVO_SIN_PERMISO -4
#define INTENTAR_BORRAR_ARCHIVO_EN_USO -10
#define ARCHIVO_NULO -11
#define NO_DEFINIDO -20*/

typedef struct configFile{
	char ip_Kernel[16];
	char puerto_Kernel[5];
	char ip_Memoria[16];
	char puerto_Memoria[5];
	char log[100];
} configFile;

typedef struct __attribute__ ((packed)) pedidoAperturaArchivo{

	t_direccion_archivo dir;
	t_banderas flags;

}pedidoAperturaArchivo;


// GLOBALES
const char* keys[6]={"IP_KERNEL", "PUERTO_KERNEL", "IP_MEMORIA", "PUERTO_MEMORIA", "LOG", "NULL"};
PCB* pcb;
int kernel, memoria;
int tamanioPagina;
configFile* config;
int quantum; // si = 0 => ES FIFO
int stackSize;
int estado;
t_log*  logFile;
int toBeKilled;
int quantumSleep;
// ANSISOP
t_puntero definirVariable(t_nombre_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida, t_valor_variable);
void asignar(t_puntero direccionVariable, t_valor_variable valor);
void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero dondeRetornar);
void llamarSinRetorno(t_nombre_etiqueta etiqueta);
t_valor_variable dereferenciar(t_puntero posicion);
void finalizar(void);
void irAlLabel(t_nombre_etiqueta nombre);
void retornar(t_valor_variable valorDeRetorno);
void signalSem(char*);
void wait(char*);
t_puntero reservar(t_valor_variable);
void liberar (t_puntero);
t_descriptor_archivo abrir(t_direccion_archivo, t_banderas);
void borrar(t_descriptor_archivo);
void cerrar(t_descriptor_archivo);
void moverCursor(t_descriptor_archivo, t_valor_variable);
void escribir(t_descriptor_archivo, void*, t_valor_variable);
void leer(t_descriptor_archivo, t_puntero, t_valor_variable);
t_valor_variable obtenerValorCompartida(t_nombre_compartida nombre);


AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_asignarValorCompartida	= asignarValorCompartida,
		.AnSISOP_obtenerValorCompartida = obtenerValorCompartida,
		.AnSISOP_asignar = asignar,
		.AnSISOP_llamarConRetorno = llamarConRetorno,
		.AnSISOP_llamarSinRetorno = llamarSinRetorno,
		.AnSISOP_finalizar = finalizar,
		.AnSISOP_irAlLabel = irAlLabel,
		.AnSISOP_retornar = retornar,
		.AnSISOP_dereferenciar = dereferenciar
};

AnSISOP_kernel primitivas_kernel = {
		.AnSISOP_wait = wait,
		.AnSISOP_signal = signalSem,
		.AnSISOP_reservar = reservar,
		.AnSISOP_liberar = liberar,
		.AnSISOP_abrir = abrir,
		.AnSISOP_borrar = borrar,
		.AnSISOP_cerrar = cerrar,
		.AnSISOP_moverCursor = moverCursor,
		.AnSISOP_escribir = escribir,
		.AnSISOP_leer = leer
};


posicionEnMemoria obtenerPosicionMemoria(int, int, int);
void iniciarConexiones(void);
int esperarPCB(void);
void informarAMemoriaDelPIDActual(void);
char* conseguirDatosDeLaMemoria(PCB*, int);
t_puntero definirVariable(t_nombre_variable);
t_puntero obtenerPosicionVariable(t_nombre_variable);
t_valor_variable asignarValorCompartida(t_nombre_compartida, t_valor_variable);
void imprimir(configFile*);
configFile* leer_archivo_configuracion(t_config*);
void conexion_memoria();
void conexion_kernel();
char* serializarPosicionEnMemoria(int, int, int);
char* pedirInstruccionAMemoria(PCB* pcb, int tamanioPagina);
posicionEnMemoria calcularPosicion(int);
indStk* crearIndiceDeStack();
variable* obtenerUltimaVariable(t_list* listaVariables);
t_puntero convertirADireccionReal(posicionEnMemoria unaPosicion);
int enviarPedidoEscrituraMemoria(posicionEnMemoria posicion, char* valor);
char* enviarPedidoLecturaMemoria(posicionEnMemoria posicion);
posicionEnMemoria convertirADireccionLogica(t_puntero posicionReal);
posicionEnMemoria generarPosicionEnBaseAUltimaVariableDe(t_list*);
char* leerEnMemoria(posicionEnMemoria);
void recibirInformacion();
serializado serializarPedido(int num);
void sig_handler(int signo);
void levantarLog();
void escribirEnMemoria(posicionEnMemoria posicion, char* valor);
