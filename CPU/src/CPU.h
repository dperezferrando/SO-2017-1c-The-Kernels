
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <commons/config.h>
#include <pthread.h>
#include <parser/parser.h>
#include "../../ConfigLibrary/src/Configuration.c"
#include "../../SocketLibrary/src/SocketLibrary.c"
const char* keys[5]={"IP_KERNEL", "PUERTO_KERNEL", "IP_MEMORIA", "PUERTO_MEMORIA", "NULL"};

typedef struct configFile{
	char ip_Kernel[16];
	char puerto_Kernel[5];
	char ip_Memoria[16];
	char puerto_Memoria[5];
} configFile;

int kernel, memoria;

// FUNCIONES ANSISOP PARSER
t_puntero definirVariable(t_nombre_variable identificador){
	int enviarHandshake = enviarHandShake(memoria, CPU_ID);
	t_puntero posicionMemoria;
	Mensaje* m;
	//int recibirHandshake=recibirHandShake(memoria, MEMORIA_ID); no se si recibir aca el handshake
	if(enviarHandshake==0 /*|| recibirHandshake==0*/){
		puts("Error en el handshake");
	}
	else{
		lSend(memoria,(t_nombre_variable) identificador,0,strlen(identificador)+1);
	}
	m = lRecv(memoria);
	posicionMemoria=(t_puntero) m-> data;
	return posicionMemoria;

}
t_puntero obtenerPosicionVariable(t_nombre_variable identificador){
	int enviarHandshake =enviarHandShake(memoria, CPU_ID);
	t_puntero desplazamiento;
	Mensaje* m;

	//recibirHandshake=recibirHandShake(memoria, MEMORIA_ID); no se si recibir aca el handshake
	if(enviarHandshake==0 /*|| recibirHandshake==0*/){
		puts("Error en el handshake");
	}
	else{
		lSend(memoria,(t_nombre_variable) identificador,0,strlen(identificador)+1);
		}

	m=lRecv(memoria);
	desplazamiento=(t_puntero) m->data;

	if(desplazamiento==-1){
		puts("Error");
	}

	return desplazamiento;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){//falta ersolver
	int enviarHandshake=enviarHandShake(kernel,CPU_ID);
	t_valor_variable valorAsignado=valor;


	if(enviarHandshake==0){
		puts("Error en el handshake");
	}
	else{
		lSend(kernel, (t_valor_variable) valorAsignado,0,sizeof(t_valor_variable));
	}

	return valorAsignado;

}




void imprimir(configFile*);
configFile* leer_archivo_configuracion(t_config*);
void conexion_memoria();
void conexion_kernel();
