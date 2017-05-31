#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"

int kernel;
#define CONFIG_FILE "filesystem.conf"
const char* keys[3] = {"PUERTO", "PUNTO_MONTAJE" , "NULL"};
const char* metaKeys[4]={"TAMANIO_BLOQUES","CANTIDAD_BLOQUES","MAGIC_NUMBER","NULL"};
const char* archKeys[3] = {"TAMANIO", "BLOQUES" , "NULL"};

typedef struct {
	char puerto[5];
	char punto_montaje [15];
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

