
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>
#include <parser/parser.h>
#include <parser/metadata_program.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"

//DEFINES DE MEMORIA

#define DESREFERENCIAR 100
#define LEER 2
#define ESCRIBIR 3

//DEFINES DE KERNEL

#define OBTENERCOMPARTIDA 200
#define ASIGNARCOMPARTIDA 201
#define WAIT 202
#define SIGNAL 203
#define RESERVAR_MEMORIA_HEAP 204
#define LIBERAR_PUNTERO 205

typedef struct configFile{
	char ip_Kernel[16];
	char puerto_Kernel[5];
	char ip_Memoria[16];
	char puerto_Memoria[5];
} configFile;


// GLOBALES
const char* keys[5]={"IP_KERNEL", "PUERTO_KERNEL", "IP_MEMORIA", "PUERTO_MEMORIA", "NULL"};
PCB* pcb;
int kernel, memoria;
int tamanioPagina;
configFile* config;
int finaliza = 0;
int quantum; // si = 0 => ES FIFO

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
void signal(char*);
void wait(char*);

AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable = definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_asignarValorCompartida	= asignarValorCompartida,
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
		.AnSISOP_signal = signal

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
void escribirEnMemoria(posicionEnMemoria posicion, t_valor_variable valor);
char* enviarPedidoLecturaMemoria(posicionEnMemoria posicion);
posicionEnMemoria convertirADireccionLogica(t_puntero posicionReal);
posicionEnMemoria generarPosicionEnBaseAUltimaVariableDe(t_list*);
char* leerEnMemoria(posicionEnMemoria);
void enviarPedidoEscrituraMemoria(posicionEnMemoria, t_valor_variable);


