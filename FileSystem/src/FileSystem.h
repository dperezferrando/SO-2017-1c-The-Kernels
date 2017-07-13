#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <ctype.h>
#include <math.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <math.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"


int kernel;
int conexion;
int flagC;
int flagR;
int flagW;
int cantBloq;
int tamBloq;
char* bitarray;
t_bitarray* Ebitarray;
#define CONFIG_FILE "filesystem.conf"
char* ruta_Conf;
char* ruta_Meta;
char* ruta_BitM;
char* ruta_CMeta;
char* ruta_Blqs;
char* ruta_Arch;
int tamRuta_Conf;
int tamRuta_Meta;
int tamRuta_BitM;
int tamRuta_CMeta;
int tamRuta_Blqs;
int tamRuta_Arch;
const char* keys[4] = {"PUERTO", "PUNTO_MONTAJE" ,"IP_PROPIA", "NULL"};
const char* metaKeys[4]={"TAMANIO_BLOQUES","CANTIDAD_BLOQUES","MAGIC_NUMBER","NULL"};
const char* archKeys[3] = {"TAMANIO", "BLOQUES" , "NULL"};

typedef struct {
	char* puerto;
	char* punto_montaje;
	char ip_propia[16];
} configFile;

typedef struct {
	int tamanio_Bloques;
	int cantidad_Bloques;
	char magic_Number[7];
} metadata;

typedef struct {
	int arrayDeBloques[5192];
} bitMap;

typedef struct {
	int tamanio;
	int* bloques;
} archivo;

typedef struct {
	int tamanio;
	int* bloques;
} bloques;

typedef struct {
	char data[65];
} data;

void imprimirConfigFile(configFile*);
configFile* leerArchivoConfig(t_config*);
metadata* leerArchivoMetadata(t_config*);
archivo* leerArch(t_config*);
void esperarOperacion();
int validarArchivo(char*);
int crearArchivo(char*);
char* leerArchivo(char*,int,int);
int escribirArchivo(char*,int,int,char*);
int borrarArchivo(char*);
int enviarTamanioArchivo(int);
int levantarBitmap(void);
int levantarBloquesEnUso(void);
void destruirBitmap(void);
void retornoDePath(char*);
bloques* getbloques(char*);
int addbloque(char*);
void quitarUltimoBloque(char*);
int getTamanio(char*);
int cambiarTamanio(char*,int);
char *rutabloque(int);
char* IntToString(int);
char* agregarBarraCero(char* data, int tamanio);




