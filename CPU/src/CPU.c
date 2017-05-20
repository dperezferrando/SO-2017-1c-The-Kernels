/*
 ============================================================================
 Name        : CPU.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */
#include "CPU.h"

AnSISOP_funciones primitivas = {
		.AnSISOP_definirVariable		= definirVariable,
		.AnSISOP_obtenerPosicionVariable= obtenerPosicionVariable,
		.AnSISOP_asignarValorCompartida				= asignarValorCompartida
};

	//por ahora sincronice todo con flags cabeza despues definimos si ponemos semaforos o que onda
	PCB pcb;
	int Pc= pcb.programCounter;
	int continuarPcb = 0;
	int actualizar =0;
	int endm=0;
	int endk=0;
	int terminar =0;
	int necesitoPcb=1;

int main(int argc, char** argsv) {
	// COSAS DEL KERNEL COMENTADAS PORQUE ESTA RIP

	configFile* config = configurate("/home/utnso/Escritorio/tp-2017-1c-The-Kernels/CPU/Debug/CPU.conf", leer_archivo_configuracion, keys);
	kernel = getConnectedSocket(config->ip_Kernel, config->puerto_Kernel, CPU_ID);
	memoria = getConnectedSocket(config->ip_Memoria, config->puerto_Memoria, CPU_ID);
	pthread_t conexionKernel, conexionMemoria;
	pthread_create(&conexionKernel, NULL, conexion_kernel, NULL);
	pthread_create(&conexionMemoria, NULL, conexion_memoria, NULL);
	pthread_join(conexionMemoria, NULL);
	pthread_join(&conexionKernel, NULL);
	free(config);
	close(kernel);
	close(memoria);
	return EXIT_SUCCESS;


}

void conexion_kernel(){

						//--------//
	// CODIGO DE TESTEO; NO TIENE QUE VER CON EL ENUNCIADO
	puts("KERNEL CONECTADO - ENVIANDO MENSAJE <3");
	lSend(kernel, "Hola soy un test", 1, sizeof(char)*17); // TESTING
						//--------//

	int conexion = lAccept(kernel, KERNEL_ID);
	while(endk==0){
		if(necesitoPcb==1){
			Mensaje* info = lRecv(conexion); //LOCKEA PARA RECIVIR
			switch(info->header.tipoOperacion){
				case -1:{
					puts("MEMORY IS DEAD");
					exit(EXIT_FAILURE);
					break;
				}
				case 1:{
					char* mensaje;
					strcpy(mensaje,info->data);
					printf("PCB RECIBIDO: %s\n", mensaje);
					pcb = deserializacion(mensaje);
					destruirMensaje(info);
					continuarPcb=1; //habilita hilo memoria
					necesitoPcb=0;
				}
			}
		}
		if (terminar==1){
			pcb.programCounter = Pc;
			lSend(kernel, "Fin Ejecucion", 1, sizeof(char)*14);
			char* mensaje;
			mensaje= serializar();
			//lSend(kernel, ("",mensaje), 1, sizeof(char)*);
			endk=1;
		}
	}
}

void conexion_memoria(){

	int conexion = lAccept(memoria, MEMORIA_ID);
	while(endm==0){
		if(continuarPcb==1){
			//busco posiciones
			int offset = pcb.indiceCodigo.offset[Pc];
			int longitud = pcb.indiceCodigo.longitud[Pc];
			//mando mensaje
			int c1 = contardigitoscabeza(offset);
			int c2 = contardigitoscabeza(longitud);
			lSend(memoria,("%d%d", offset, longitud), 1, sizeof(char)*(c1+c2+1));
			//espero sentencia
			puts("Esperando sentencia de Memoria");
			Mensaje* info = lRecv(conexion); //LOCKEA PARA RECIVIR
			switch(info->header.tipoOperacion){
				case -1:{
					puts("MEMORY IS DEAD");
					exit(EXIT_FAILURE);
					break;
				}
				case 1:{
					char* sentencia;
					strcpy(sentencia,info->data);
					printf("SENTENCIA RECIBIDA: %s\n", sentencia);
					destruirMensaje(info);
					analizadorlinea(sentencia); //mando sentencia al analisador (No se como funciona)
					Pc++; //aumento Pc, antes de morir debe actualizar pc en pcb y enviar a kernel
					continuarPcb=0; //no se espera mas nada (Este CP)
				}
			}
		}
		if (actualizar==1){
			actualizarvalores();
			endm=1;
		}
	}

	// CODIGO DE TESTEO; NO TIENE QUE VER CON EL ENUNCIADO. SE MANDAN MENSAJES A MEMORIA. "SALIR" PARA TERMINAR
	/*puts("HILI");

	char mensaje[10];
	scanf("%s", mensaje);
	while(strcmp(mensaje, "SALIR"))
	{
		lSend(memoria, mensaje, 1, sizeof(char)*10);
		scanf("%s", mensaje);
	}
	lSend(memoria, NULL, 2, 0);*/

}

configFile* leer_archivo_configuracion(t_config* configHandler){

	configFile* config = malloc(sizeof(configFile));
	strcpy(config->puerto_Kernel, config_get_string_value(configHandler, "PUERTO_KERNEL"));
	strcpy(config->puerto_Memoria, config_get_string_value(configHandler, "PUERTO_MEMORIA"));
	strcpy(config->ip_Kernel, config_get_string_value(configHandler, "IP_KERNEL"));
	strcpy(config->ip_Memoria, config_get_string_value(configHandler, "IP_MEMORIA"));
	config_destroy(configHandler);
	imprimir(config);
	return config;
}

void imprimir(configFile* c){
	puts("--------PROCESO CPU--------");
	printf("IP KERNEL: %s\n", c->ip_Kernel);
	printf("PUERTO KERNEL: %s\n", c->puerto_Kernel);
	printf("IP MEMORIA: %s\n", c->ip_Memoria);
	printf("PUERTO MEMORIA: %s\n", c->puerto_Memoria);
	puts("--------PROCESO CPU--------");

}

int contardigitoscabeza(int var){ //se llama cabeza porque supongo que hay alguna common para hacer esto, pero no la encontre
	int c = 1;
	while(var/10>0){
		c++;
		var = var / 10;
	}
	return c;
}

void actualizarvalores(){ //INCOMPLETA
	//lSend(memoria, MENSAJEVALORESDELSTACK, 1, sizeof(char)*(c1+c2));
	terminar=1;
}
