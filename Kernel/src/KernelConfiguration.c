#import "KernelConfiguration.h"


bool configKernel(char* ruta){
	return configurate(ruta,&readConfigFile);
}

void handleConfigFile(t_config* configHandler){
	configFile config = readConfigFile(configHandler);
	imprimirConfig(config);
}

void imprimirConfig(configFile config)
{
	puts("--------PROCESO MEMORIA--------");
	printf("PUERTO PROGRAMA: %i | PUERTO CPU: %i \n", config.PUERTO_PROG, config.PUERTO_CPU);
	printf("IP MEMORIA: %i | PUERTO MEMORIA: %i \n", config.IP_MEMORIA, config.PUERTO_MEMORIA);
	printf("IP FILE SYSTEM: %i | PUERTO FILE SYSTEM: %i \n", config.IP_FS, config.PUERTO_FS);
	printf("QUANTUM: %i | QUANTUM SLEEP: %i | ALGORITMO: %i \n", config.QUANTUM, config.QUANTUM_SLEEP,config.ALGORITMO);
	printf("GRADO MULTIPGROG: %i | SEM IDS: %i | SEM INIT: %i \n", config.GRADO_MULTIPROG, config.SEM_IDS,config.SEM_INIT);
	printf("SHARED VARS: %i | STACK_SIZE: %i \n", config.SHARED_VARS, config.STACK_SIZE);
	puts("--------PROCESO MEMORIA--------");
}

/*
 *
configFile config = readConfigFile(configHandler);
imprimirConfig(config);
 */


configFile readConfigFile(t_config* configHandler)//la unica manera de generalizar esto es muy chota y seria incluyendo el tipo de dato ademas del nombre en un array
{
	configFile config;
	config.PUERTO_PROG = config_get_int_value(configHandler, "PUERTO_PROG");
	config.PUERTO_CPU = config_get_int_value(configHandler, "PUERTO_CPU");
	config.IP_MEMORIA = config_get_int_value(configHandler, "IP_MEMORIA");
	config.PUERTO_MEMORIA = config_get_int_value(configHandler, "PUERTO_MEMORIA");
	config.IP_FS = config_get_int_value(configHandler, "IP_FS");
	config.PUERTO_FS = config_get_int_value(configHandler, "PUERTO_FS");
	config.QUANTUM = config_get_int_value(configHandler, "QUANTUM");
	config.QUANTUM_SLEEP = config_get_int_value(configHandler, "QUANTUM_SLEEP");
	strcpy(config.ALGORITMO,config_get_string_value(configHandler, "ALGORITMO"));
	config.GRADO_MULTIPROG = config_get_int_value(configHandler, "GRADO_MULTIPROG");
	config.SEM_IDS= config_get_array_value(configHandler, "SEM_IDS");
	config.SEM_INIT= config_get_array_value(configHandler, "SEM_INIT");
	config.SHARED_VARS= config_get_array_value(configHandler, "SHARED_VARS");
	config_destroy(configHandler);
	return config;
}
