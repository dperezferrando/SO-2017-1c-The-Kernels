#include "Configuration.h"

typedef struct{
	char* SEM_IDS;
	int SEM_INIT;
}SEM_STRUCT;

typedef struct{
	char* SHARED_VARS[];
}SHARED_VARIABLES;

typedef struct {
	int PUERTO_PROG;
	int PUERTO_CPU;
	char* IP_MEMORIA;
	int PUERTO_MEMORIA;
	char* IP_FS;
	char PUERTO_FS;
	int QUANTUM;
	int QUANTUM_SLEEP;
	char* ALGORITMO;
	int GRADO_MULTIPROG;
	SHARED_VARIABLES SHARED_VARS;
	int STACK_SIZE;
	SEM_STRUCT SEM[];
} configFile;

bool configKernel(char*);
void handleConfigFile(t_config*);
configFile readConfigFile(t_config* configHandler);

