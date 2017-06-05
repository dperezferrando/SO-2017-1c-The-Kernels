#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <pthread.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"

#define CONFIG_FILE "memoria.conf"
const char* keys[9] = {"PUERTO_KERNEL", "PUERTO_CPU", "MARCOS", "MARCO_SIZE", "ENTRADAS_CACHE", "CACHE_X_PROC", "REEMPLAZO_CACHE", "RETARDO_MEMORIA", "NULL"};

typedef struct {
	char puerto_kernel[5];
	char puerto_cpu[5];
	int marcos;
	int marco_size;
	int entradas_cache;
	int cache_x_proc;
	char reemplazo_cache[4];
	int retardo_memoria;
} configFile;

typedef struct entradaTabla {
	int pid;
	int frame;
	int pagina;
} entradaTabla;

void imprimirConfigFile(configFile*);
configFile* leerArchivoConfig(t_config*);
void conexion_kernel();
void conexion_cpu();
void esperar_cpus();
void levantarSockets();
void arrancarMemoria();
void mostrarTablaPaginas();
void crearEntradas(int, int);
int bestHashingAlgorithmInTheFuckingWorld(int, int);
char* solicitarBytes(int, int, int, int);
int escribirBytes(int,int,int,int,void*);
void inicializarPrograma(int,int,char*,int);
void escribirCodigoPrograma(int, char*, int);
int sePuedenAsignarPaginas(int, int);
void recibir_comandos();
entradaTabla* obtenerEntradaDe(int pid, int pagina);
void finalizarPrograma(int pid);


