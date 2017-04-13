/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>


#define CONFIG_FILE "cpu.conf"

typedef struct configFile{
	char* ip_Kernel;
	unsigned short int puerto_Kernel;
	char* ip_Memoria;
	unsigned short int puerto_Memoria;
} configFile;

configFile leer_archivo_configuracion(char ruta[]){

	configFile c;
	t_config* configHandler=config_create(ruta);
	c.puerto_Kernel= config_get_int_value(configHandler, "PUERTO KERNEL");
	c.puerto_Memoria= config_get_int_value(configHandler, "PUERTO MEMORIA");
	c.ip_Kernel=config_get_string_value(configHandler, "IP KERNEL");
	c.ip_Memoria=config_get_string_value(configHandler, "IP MEMORIA");
	config_destroy(configHandler);
	return c;
}

void imprimir(configFile c){
	puts("PROCESO CPU");
	printf("IP KERNEL: %s/n", c.ip_Kernel);
	printf("PUERTO KERNEL: %d/n", c.puerto_Kernel);
	printf("IP MEMORIA: %s/n", c.ip_Memoria);
	printf("PUERTO MEMORIA: %d/n", c.puerto_Memoria);

}

bool chequear_campos_archivo(t_config* handler){
	char* campos[4]={"IP_KERNEL", "PUERTO_KERNEL", "IP_MEMORIA", "PUERTO_MEMORIA"};
	bool archivo_correcto;
	int i;
	for (i =0; i<4; i++){
		if(config_has_property(handler, campos[i])){
			archivo_correcto=true;
		}
		else{
			archivo_correcto=false;
			puts("Error: Faltan campos en el archivo de configuracion");
			break;
		}
	}
	return archivo_correcto;

	}

bool chequear_ruta_archivo(t_config* handler){

	bool b= handler->path != NULL;
	if(!b){
		puts("Error: ruta invalida");
	}
	return b;
}

bool chequear_archivo_valido(t_config* handler){

	return chequear_campos_archivo(handler) && chequear_ruta_archivo(handler);

}

int main(int argc, char** argsv) {



	t_config* configHandler = config_create(argsv[1]);
		if(!chequear_archivo_valido(configHandler)){
			return EXIT_FAILURE;
		}
	configFile config = leer_archivo_configuracion(configHandler->path);
	imprimir(config);

	/*
	struct sockaddr_in direccionKernel;
	direccionKernel.sin_family=AF_INET;
	direccionKernel.sin_addr.s_addr=inet_addr(DIRECCION_IP_KERNEL);
	direccionKernel.sin_port=htons(PUERTO_KERNEL);

	struct sockaddr_in direcciónCliente;
	unsigned int len;
	int cliente = accept(servidor, (void*) &direcciónCliente, &len);

	printf("Recibí una conexión en %d!!\n", cliente);






	*/
	return EXIT_SUCCESS;


}




