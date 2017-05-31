#include "FileSystem.h"


int main(int argc, char** argsv) {
	configFile* config;
	metadata* metad;
	config = configurate("/home/utnso/tp-2017-1c-The-Kernels/mnt/filesystem.conf", leerArchivoConfig, keys);
	metad = configurate("/home/utnso/tp-2017-1c-The-Kernels/mnt/FS_SADICA/Metadata/Metadata.bin", leerArchivoMetadata, metaKeys);
	kernel = getBindedSocket("127.0.0.1", config->puerto);
	puts("ESPERANDO AL KERNEL");
	int conexion = lAccept(kernel, KERNEL_ID);
	esperarOperacion();
	close(kernel);
	free(config);
	free(metad);
	return EXIT_SUCCESS;
}

void esperarOperacion()
{
	puts("Esperando Operacion");
	Mensaje* mensaje = lRecv(kernel);
	puts("Operacion recibida");
	int sizePath;
	char* path;
	int offset;
	int  size;
	int sizeBuffer;
	char* buffer;
	switch(mensaje->header.tipoOperacion){
		case -1:
			puts("MURIO EL KERNEL /FF");
			exit(EXIT_FAILURE);
			break;
		case 0:{
			//Op.validar
			memcpy(sizePath, mensaje->data, sizeof(int));
			memcpy(path, mensaje->data, sizePath);
			validarArchivo(path);
			break;
		}
		case 1:{
			//Op.crear
			memcpy(sizePath, mensaje->data, sizeof(int));
			memcpy(path, mensaje->data + sizeof(int), sizePath);
			crearArchivo(path);
			break;
		}
		case 2:{
			//Op.leer
			memcpy(sizePath, mensaje->data, sizeof(int));
			memcpy(path, mensaje->data + sizeof(int), sizePath);
			memcpy(offset, mensaje->data+ sizeof(int) + sizeof(sizePath), sizeof(int));
			memcpy(size, mensaje->data + sizeof(int)*2 + sizeof(sizePath), sizeof(int));
			leerArchivo(path,offset,size);
			break;
		}
		case 3:{
			//Op.escribir
			memcpy(sizePath, mensaje->data, sizeof(int));
			memcpy(path, mensaje->data + sizeof(int), sizePath);
			memcpy(offset, mensaje->data+ sizeof(int) + sizeof(sizePath), sizeof(int));
			memcpy(size, mensaje->data + sizeof(int)*2 + sizeof(sizePath), sizeof(int));
			memcpy(sizeBuffer, mensaje->data + sizeof(int)*3 + sizeof(sizePath), sizeof(int));
			memcpy(buffer, mensaje->data + sizeof(int)*4 + sizeof(sizePath), sizeBuffer);
			escribirArchivo(path,offset,size,buffer);
			break;
		}
		case 4:{
			//Op.borrar
			memcpy(sizePath, mensaje->data, sizeof(int));
			memcpy(path, mensaje->data + sizeof(int), sizePath);
			borrarArchivo(path);
			break;
		}
	}
	destruirMensaje(mensaje);
}

validarArchivo(char* path){
	archivo* arch;
	arch = configurate(path, leerArch, archKeys);
	enviarTamanioArchivo(arch->tamanio);
	free(arch);
}

enviarTamanioArchivo(int tamanio){
	lSend(kernel, "archivo existente", 1, sizeof(char)*17);
	lSend(kernel,  tamanio, 1, sizeof(int));
}

crearArchivo(char* path){

}

char* leerArchivo(char* path, int offset, int size){
	char* buffer;
	return buffer;
}

escribirArchivo(char* path, int offset, int size, char* buffer){

}

borrarArchivo(char* path){

}

configFile* leerArchivoConfig(t_config* configHandler)
{
	configFile* config= malloc(sizeof(configFile));
	strcpy(config->puerto, config_get_string_value(configHandler, "PUERTO"));
	strcpy(config->punto_montaje,config_get_string_value(configHandler, "PUNTO_MONTAJE"));
	config_destroy(configHandler);
	imprimirConfig(config);
	return config;
}

metadata* leerArchivoMetadata(t_config* configHandler)
{
	metadata* metad= malloc(sizeof(metadata));
	metad->tamanio_Bloques = config_get_int_value(configHandler, "TAMANIO_BLOQUES");
	metad->cantidad_Bloques = config_get_int_value(configHandler, "CANTIDAD_BLOQUES");
	strcpy(metad->magic_Number,config_get_string_value(configHandler, "MAGIC_NUMBER"));
	config_destroy(configHandler);
	imprimirMetadata(metad);
	return metad;
}

archivo* leerArch(t_config* configHandler)
{
	archivo* arch= malloc(sizeof(archivo));
	arch->tamanio = config_get_int_value(configHandler, "TAMANIO");
	arch->bloques = config_get_array_value(configHandler, "BLOQUES");
	config_destroy(configHandler);
	return arch;
}

void imprimirConfig(configFile* config)
{
	puts("--------PROCESO FILESYSTEM--------");
	printf("ESCUCHANDO EN PUERTO: %s | PUNTO_MONTAJE %s\n", config->puerto, config->punto_montaje);
	puts("--------PROCESO FILESYSTEM--------\n");
}

void imprimirMetadata(metadata* metad)
{
	puts("--------FILESYSTEM METADATA--------");
	printf("TAMAÑO DE BLOQUES: %d | CANTIDAD DE BLOQUES: %d | MAGIC NUMBER: %s\n", metad->tamanio_Bloques, metad->cantidad_Bloques, metad->magic_Number);
	puts("--------FILESYSTEM METADATA--------");
}
