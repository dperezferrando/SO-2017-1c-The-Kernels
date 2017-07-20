#include "KernelConfiguration.h"


/*void handleConfigFile(t_config* configHandler){
	configFile* config = readConfigFile(configHandler);

}*/

void imprimirConfig(configFile* config)
{	puts("--------PROCESO KERNEL--------");
	printf("PUERTO PROGRAMA: %s | PUERTO CPU: %s \n", config->PUERTO_PROG, config->PUERTO_CPU);
	printf("IP MEMORIA: %s | PUERTO MEMORIA: %s \n", config->IP_MEMORIA, config->PUERTO_MEMORIA);
	printf("IP FILE SYSTEM: %s | PUERTO FILE SYSTEM: %s \n", config->IP_FS, config->PUERTO_FS);
	printf("QUANTUM: %i | QUANTUM SLEEP: %i | ALGORITMO: %s \n", config->QUANTUM, config->QUANTUM_SLEEP,config->ALGORITMO);
	printf("GRADO MULTIPGROG: %i\n", config->GRADO_MULTIPROG);
	mostrarSemaforos(config);
	mostrarVarsGlobales(config);
	printf("STACK_SIZE: %i\n", config->STACK_SIZE);
	printf("IP PROPIA: %s\n", config->IP_PROPIA);
	puts("--------PROCESO KERNEL--------");
}

void mostrarSemaforos(configFile* config)
{
	int i = 0;
	printf("SEMAFOROS:");
	while(config->SEM_IDS[i] != NULL)
	{
		printf(" %s [%s] |", config->SEM_IDS[i], config->SEM_INIT[i]);
		i++;
	}
	puts("");
}

void mostrarVarsGlobales(configFile* config)
{
	int i = 0;
	printf("VARS GLOBALES:");
	while(config->SHARED_VARS[i] != NULL)
	{
		printf(" %s |", config->SHARED_VARS[i]);
		i++;
	}
	puts("");
}


void destruirArray(char** arr)
{
	int i = 0;
	while(arr[i] != NULL)
	{
		free(arr[i]);
		i++;
	}
	free(arr);
}

void destruirConfig(configFile* config)
{
	destruirArray(config->SEM_IDS);
	destruirArray(config->SHARED_VARS);
	destruirArray(config->SEM_INIT);
	free(config);
}

/*
 *
configFile config = readConfigFile(configHandler);
imprimirConfig(config);
 */



configFile* readConfigFile(t_config* configHandler)//la unica manera de generalizar esto es muy chota y seria incluyendo el tipo de dato ademas del nombre en un array
{
	configFile* config = malloc(sizeof(configFile));
	strcpy(config->PUERTO_PROG, config_get_string_value(configHandler, "PUERTO_PROG"));
	strcpy(config->PUERTO_CPU, config_get_string_value(configHandler, "PUERTO_CPU"));
	strcpy(config->IP_MEMORIA, config_get_string_value(configHandler, "IP_MEMORIA"));
	strcpy(config->PUERTO_MEMORIA, config_get_string_value(configHandler, "PUERTO_MEMORIA"));
	strcpy(config->IP_FS, config_get_string_value(configHandler, "IP_FS"));
	strcpy(config->PUERTO_FS, config_get_string_value(configHandler, "PUERTO_FS"));
	config->QUANTUM = config_get_int_value(configHandler, "QUANTUM");
	config->QUANTUM_SLEEP = config_get_int_value(configHandler, "QUANTUM_SLEEP");
	strcpy(config->ALGORITMO,config_get_string_value(configHandler, "ALGORITMO"));
	config->GRADO_MULTIPROG = config_get_int_value(configHandler, "GRADO_MULTIPROG");
	config->SEM_IDS= config_get_array_value(configHandler, "SEM_IDS");
	config->SEM_INIT= config_get_array_value(configHandler, "SEM_INIT");
	config->SHARED_VARS= config_get_array_value(configHandler, "SHARED_VARS");
	config->STACK_SIZE=config_get_int_value(configHandler, "STACK_SIZE");
	strcpy(config->IP_PROPIA, config_get_string_value(configHandler, "IP_PROPIA"));
	strcpy(config->log, config_get_string_value(configHandler, "LOG"));
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}
