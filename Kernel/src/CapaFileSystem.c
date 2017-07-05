#include "CapaFileSystem.h"

tablaDeProceso* crearTablaDeProceso(int pid)
{
	tablaDeProceso* unaTabla = malloc(sizeof(tablaDeProceso));
	unaTabla->entradasTablaProceo = list_create();
	unaTabla->pid = pid;
	return unaTabla;
}

void crearEstructurasFSProceso(int pid)
{
	tablaDeProceso* unaTabla = crearTablaDeProceso(pid);
	list_add(tablasDeProcesosFS, unaTabla);
}

void deserializarInfoArchivo(char* data, int* pid, char** ruta, char** permisos)
{
	memcpy(&pid, data, sizeof(int));
	int tamanioRuta;
	memcpy(&tamanioRuta, data+sizeof(int), sizeof(int));
	*ruta = malloc(tamanioRuta+1); // agrego 1 byte para \0
	memcpy(*ruta, data+sizeof(int)+sizeof(int), tamanioRuta);
	*ruta[tamanioRuta] = '\0';
	int tamanioPermisos;
	memcpy(&tamanioPermisos, data+sizeof(int)+sizeof(int)+tamanioRuta, sizeof(int));
	*permisos = malloc(tamanioPermisos+1);
	memcpy(*permisos, data+sizeof(int)+sizeof(int)+tamanioRuta+sizeof(int), tamanioPermisos);
	*permisos[tamanioPermisos] = '\0';
}

int abrirArchivo(int pid, char* ruta, char* permisos)
{
	int existe;
	entradaTablaGlobalFS* entradaBuscada = buscarEnTablaGlobal(ruta);
	if(entradaBuscada == NULL)
	{
		entradaBuscada = agregarEntradaGlobal(ruta, permisos);
		if(entradaBuscada == NULL) // NO EXISTE EN EL FILESYSTEM EL ARCHIVO QUE SE QUIERE ABRIR
			return -1;
	}
	int fd = agregarEntradaTablaProceso(entradaBuscada, pid, permisos);
	return fd;

}

entradaTablaGlobalFS* buscarEnTablaGlobal(char* ruta)
{
	bool tieneLaMismaRuta(entradaTablaGlobalFS* entrada)
	{
		return !strcmp(entrada->ruta, ruta);
	}
	return list_find(tablaGlobalFS, tieneLaMismaRuta);
}

entradaTablaGlobalFS* agregarEntradaGlobal(char* ruta, char* permisos)
{
	if(!archivoValido(ruta) && !strstr(permisos, "c"))
		return NULL;
	else
	{
		if(strstr(permisos, "c"))
			lSend(conexionFS, ruta, 4, strlen(ruta));
		entradaTablaGlobalFS* entrada = malloc(sizeof(entradaTablaGlobalFS));
		entrada->instancias = 0;
		strcpy(entrada->ruta, ruta);
		list_add(tablaGlobalFS, entrada);
		return entrada;
	}
}

bool archivoValido(char* ruta)
{
	// CHEQUEAR CODIGOS DE OP CON EL FS
	lSend(conexionFS, ruta, 1, strlen(ruta));
	Mensaje* respuesta = lRecv(conexionFS);
	int op = respuesta->header.tipoOperacion;
	destruirMensaje(respuesta);
	return op == 104;
}

tablaDeProceso* encontrarTablaDelProceso(int pid)
{
	bool tienenMismoPid(tablaDeProceso* tabla)
	{
		return tabla->pid == pid;
	}
	return list_find(tablasDeProcesosFS, tienenMismoPid);
}

int agregarEntradaTablaProceso(entradaTablaGlobalFS* entradaGlobal, int pid, char* permisos)
{
	entradaTablaFSProceso* entrada = malloc(sizeof(entradaTablaFSProceso));
	entradaGlobal->instancias++;
	entrada->entradaGlobal = entradaGlobal;
	strcpy(entrada->flags, permisos);
	tablaDeProceso* tablaDelProceso = encontrarTablaDelProceso(pid);
	if(tablaDelProceso == NULL)
		puts("ESTO NO DEBERIA PASAR NUNCA, SI VES ESTE MENSAJE GET READY FOR SEG FAULT");
	list_add(tablaDelProceso->entradasTablaProceo, entrada);
	int fd = list_size(tablaDelProceso->entradasTablaProceo) + 2;
	return fd;
}

bool moverCursorArchivo(fileInfo info)
{
	entradaTablaFSProceso* entrada = buscarEnTablaDelProceso(info.pid, info.fd);
	if(entrada == NULL)
		return 0;
	entrada->cursor = info.cursor;
	return 1;
}

entradaTablaFSProceso* buscarEnTablaDelProceso(int pid, int fd)
{
	tablaDeProceso* tabla = encontrarTablaDelProceso(pid);
	entradaTablaFSProceso* entrada = list_get(tabla->entradasTablaProceo, fd-3);
	return entrada;
}

char* leerArchivo(fileInfo info)
{
	entradaTablaFSProceso* entrada = buscarEnTablaDelProceso(info.pid, info.fd);
	if(!strstr(entrada->flags, "r"))
		return NULL;
	serializado pedidoLectura = serializarPedidoLectura(entrada->entradaGlobal->ruta, entrada->cursor, info.tamanio);
	lSend(conexionFS, pedidoLectura.data, 2, pedidoLectura.size);
	free(pedidoLectura.data);
	Mensaje* respuesta = lRecv(conexionFS);
	char* data = malloc(respuesta->header.tamanio);
	memcpy(data, respuesta->data, respuesta->header.tamanio);
	destruirMensaje(respuesta);
	return data;
}

bool escribirArchivo(fileInfo info, char* data)
{
	if(info.fd == 1)
		imprimirPorPantalla(info, data);
	else
	{
		entradaTablaFSProceso* entrada = buscarEnTablaDelProceso(info.pid, info.fd);
		if(!strstr(entrada->flags, "w"))
			return 0;
		serializado pedidoEscritura = serializarPedidoEscritura(entrada->entradaGlobal->ruta, entrada->cursor, info.tamanio, data);
		lSend(conexionFS, pedidoEscritura.data, 3, pedidoEscritura.size);
		free(pedidoEscritura.data);
	}
	return 1;
}

bool cerrarArchivo(int pid, int fd)
{
	tablaDeProceso* tabla = encontrarTablaDelProceso(pid);
	entradaTablaFSProceso* entrada = buscarEnTablaDelProceso(pid, fd);
	if(entrada == NULL)
		return 0;
	cerrarArchivoEnTablaGlobal(entrada->entradaGlobal);
	list_remove_and_destroy_element(tabla->entradasTablaProceo, fd-3, destruirEntradaTablaProceso);
	return 1;

}

void cerrarArchivoEnTablaGlobal(entradaTablaGlobalFS* entrada)
{
	entrada->instancias--;
	bool mismaRuta(entradaTablaGlobalFS* unaEntrada)
	{
		return !strcmp(entrada->ruta, unaEntrada->ruta);
	}
	if(entrada->instancias == 0)
	{
		list_remove_and_destroy_by_condition(tablaGlobalFS, mismaRuta, destruirEntradaGlobal);
	}
}

void destruirEntradaGlobal(entradaTablaGlobalFS* entrada)
{
	free(entrada->ruta);
}

void destruirEntradaTablaProceso(entradaTablaFSProceso* entrada)
{
	free(entrada->entradaGlobal);
}

void imprimirPorPantalla(fileInfo info, char* data)
{
	char* impresion = malloc(info.tamanio+1);
	strcpy(impresion, data);
	impresion[info.tamanio] = '\0';
	ProcessControl* pc= PIDFind(info.pid);
	lSend(pc->consola, impresion, 2, info.tamanio+1);
	free(impresion);

}

bool borrarArchivo(int pid, int fd)
{
	entradaTablaFSProceso* entrada = buscarEnTablaDelProceso(pid, fd);
	if(entrada->entradaGlobal->instancias > 1)
		return 0;
	lSend(conexionFS, entrada->entradaGlobal->ruta, 5, strlen(entrada->entradaGlobal->ruta));
	cerrarArchivo(pid, fd);
	return 1;
}

serializado serializarPedidoEscritura(char* ruta, int offset, int size, char* data)
{
	serializado pedido;
	int sizeRuta = strlen(ruta);
	pedido.size = sizeRuta+(sizeof(int)*3) + size;
	pedido.data = malloc(pedido.size);
	memcpy(pedido.data, &sizeRuta, sizeof(int));
	memcpy(pedido.data, ruta, sizeRuta);
	memcpy(pedido.data+sizeRuta+sizeof(int), &offset, sizeof(int));
	memcpy(pedido.data+sizeRuta+(sizeof(int)*2), &size, sizeof(int));
	memcpy(pedido.data+sizeRuta+(sizeof(int))*3, data, size);
	return pedido;

}

serializado serializarPedidoLectura(char* ruta, int offset, int size)
{
	serializado pedido;
	int sizeRuta = strlen(ruta);
	pedido.size = sizeRuta + (sizeof(int)*3);
	pedido.data = malloc(pedido.size);
	memcpy(pedido.data, &sizeRuta, sizeof(int));
	memcpy(pedido.data+sizeof(int), ruta, sizeRuta);
	memcpy(pedido.data+sizeof(int)+sizeRuta, &offset, sizeof(int));
	memcpy(pedido.data+(sizeof(int)*2)+sizeRuta, &size, sizeof(int));
	return pedido;

}
