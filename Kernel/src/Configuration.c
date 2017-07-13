#include "Configuration.h"

int cantKeysALeer(char** keys)
{
	int i = 0;
	while(strcmp(keys[i], "NULL"))
		i++;
	return i;
}


bool archivoConfigCompleto(t_config* configHandler, char** keys)
{
	bool archivoValido = true;
	int i = 0;
	for(i = 0;i<cantKeysALeer(keys);i++)
	{
		if(!config_has_property(configHandler, keys[i]))
		{
			archivoValido = false;
			puts("ERROR: ARCHIVO DE CONFIGURACION INCOMPLETO");
			break;
		}
	}
	return archivoValido;
}

bool rutaCorrecta(t_config* configHandler)
{
	if(configHandler == NULL)
	{
		puts("ERROR: RUTA INCORRECTA PARA ARCHIVO DE CONFIGURACION");
		return false;
	}
	else return true;
}

bool archivoConfigValido(t_config* configHandler, char* keys[])
{
	return rutaCorrecta(configHandler) && archivoConfigCompleto(configHandler,keys);

}

void* configurate(char* ruta, void*(*handleConfigFile)(t_config*), char* keys[]) {
	t_config* configHandler = config_create(ruta);
	if(!archivoConfigValido(configHandler,keys))
		abort();
	return handleConfigFile(configHandler);

}

PCB* deserializarPCB(char* pcbSerializado) // A SER REEMPLAZADO POR LO DE NICO
{
	PCB* pcb = malloc(sizeof(PCB));
	char* puntero = pcbSerializado;
	memcpy(&pcb->pid, puntero, sizeof(int));
	puntero += sizeof(int);
	memcpy(&pcb->cantPaginasCodigo, puntero, sizeof(int));
	puntero += sizeof(int);
	memcpy(&pcb->programCounter, puntero, sizeof(int));
	puntero += sizeof(int);
	memcpy(&pcb->sizeIndiceCodigo, puntero, sizeof(int));
	puntero += sizeof(int);
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	memcpy(pcb->indiceCodigo, puntero, pcb->sizeIndiceCodigo);
	puntero += pcb->sizeIndiceCodigo;
	memcpy(&pcb->sizeIndiceEtiquetas, puntero, sizeof(int));
	puntero += sizeof(int);
	pcb->indiceEtiqueta = malloc(pcb->sizeIndiceEtiquetas);
	memcpy(pcb->indiceEtiqueta, puntero, pcb->sizeIndiceEtiquetas);
	puntero += pcb->sizeIndiceEtiquetas;
	memcpy(&pcb->nivelDelStack, puntero, sizeof(int));
	puntero += sizeof(int);
	serializado indiceStackSerializado;
	memcpy(&indiceStackSerializado.size, puntero, sizeof(int));
	puntero += sizeof(int);
	indiceStackSerializado.data = malloc(indiceStackSerializado.size);
	memcpy(indiceStackSerializado.data, puntero, indiceStackSerializado.size);
	puntero += indiceStackSerializado.size;
	pcb->indiceStack = deserializarIndiceDeStack(indiceStackSerializado, pcb->nivelDelStack);
	memcpy(puntero, &pcb->exitCode, sizeof(int));
	free(indiceStackSerializado.data);
	return pcb;

}

serializado serializarPCB(PCB* pcb)
{
	serializado pcbSerializado;
	serializado indiceStackSerializado = serializarIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
	pcbSerializado.size = sizeof(int)*8 + pcb->sizeIndiceCodigo + pcb->sizeIndiceEtiquetas + indiceStackSerializado.size;
	pcbSerializado.data = malloc(pcbSerializado.size);
	char* puntero = pcbSerializado.data;
	memcpy(puntero, &pcb->pid, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, &pcb->cantPaginasCodigo, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, &pcb->programCounter, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, &pcb->sizeIndiceCodigo, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, pcb->indiceCodigo, pcb->sizeIndiceCodigo);
	puntero += pcb->sizeIndiceCodigo;
	memcpy(puntero, &pcb->sizeIndiceEtiquetas, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, pcb->indiceEtiqueta, pcb->sizeIndiceEtiquetas);
	puntero += pcb->sizeIndiceEtiquetas;
	memcpy(puntero, &pcb->nivelDelStack, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, &indiceStackSerializado.size, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, indiceStackSerializado.data, indiceStackSerializado.size);
	puntero += indiceStackSerializado.size;
	memcpy(puntero, &pcb->exitCode, sizeof(int));
	free(indiceStackSerializado.data);
	return pcbSerializado;

}

void mostrarIndiceDeStack(indStk* indiceStack, int ultimoNivel)
{
	int i;
	for(i = 0;i<=ultimoNivel;i++)
	{
		printf("-------INDICE DE STACK NIVEL %i-------\n", i);
		printf("POS RETORONO: %i | VAR RETORNO %c: PAG: %i - OFFSET: %i - SIZE: %i\n", indiceStack[i].posicionDeRetorno, indiceStack[i].variableDeRetorno.identificador, indiceStack[i].variableDeRetorno.posicion.pagina, indiceStack[i].variableDeRetorno.posicion.offset, indiceStack[i].variableDeRetorno.posicion.size);
		puts("VARIABLES:");
		list_iterate(indiceStack[i].variables, mostrarVariable);
		puts("ARGUMENTOS");
		list_iterate(indiceStack[i].argumentos, mostrarVariable);
	}
}

void mostrarVariable(variable* var)
{
	printf("VARIABLE %c | PAG: %i | OFFSET %i | SIZE: %i\n", var->identificador, var->posicion.pagina, var->posicion.offset, var->posicion.size);
}

serializado serializarIndiceDeStack(indStk* indiceStack, int ultimoNivel)
{
	serializado indiceStackSerializado;
	indiceStackSerializado.size = sizeof(variable)+sizeof(int)*3;
	int i;
	for(i = 0;i<=ultimoNivel;i++)
	{
		int cantVars = list_size(indiceStack[i].variables);
		int cantArgs = list_size(indiceStack[i].argumentos);
		indiceStackSerializado.size += (cantVars+cantArgs)*sizeof(variable);
	}
	indiceStackSerializado.data = malloc(indiceStackSerializado.size);
	char* puntero = indiceStackSerializado.data;
	for(i = 0;i<=ultimoNivel;i++)
	{
		int cantVars = list_size(indiceStack[i].variables);
		int cantArgs = list_size(indiceStack[i].argumentos);
		memcpy(puntero, &indiceStack[i].posicionDeRetorno, sizeof(int));
		puntero += sizeof(int);
		memcpy(puntero, &indiceStack[i].variableDeRetorno, sizeof(variable));
		puntero += sizeof(variable);
		memcpy(puntero, &cantVars, sizeof(int));
		puntero += sizeof(int);
		void serializar(variable* unaVariable)
		{
			memcpy(puntero, unaVariable, sizeof(variable));
			puntero += sizeof(variable);
		}
		if(cantVars != 0)
			list_iterate(indiceStack[i].variables, serializar);
		memcpy(puntero, &cantArgs, sizeof(int));
		puntero += sizeof(int);
		if(cantArgs != 0)
			list_iterate(indiceStack[i].argumentos, serializar);
	}
	return indiceStackSerializado;

}

indStk* deserializarIndiceDeStack(serializado indiceSerializado, int ultimoNivel)
{
	indStk* indiceDeStack = malloc(sizeof(indStk)*(ultimoNivel+1));
	char* puntero = indiceSerializado.data;
	int i;
	for(i = 0;i<=ultimoNivel;i++)
	{
		memcpy(&indiceDeStack[i].posicionDeRetorno, puntero, sizeof(int));
		puntero += sizeof(int);
		memcpy(&indiceDeStack[i].variableDeRetorno, puntero, sizeof(variable));
		puntero += sizeof(variable);
		int cantVars;
		memcpy(&cantVars, puntero, sizeof(int));
		puntero += sizeof(int);
		indiceDeStack[i].variables = list_create();
		indiceDeStack[i].argumentos = list_create();
		if(cantVars != 0)
		{
			int j;
			for(j = 0;j<cantVars;j++)
			{
				variable* var = malloc(sizeof(variable));
				memcpy(var, puntero, sizeof(variable));
				list_add(indiceDeStack[i].variables, var);
				puntero+= sizeof(variable);
			}
		}
		int cantArgs;
		memcpy(&cantArgs, puntero, sizeof(int));
		puntero += sizeof(int);
		if(cantArgs != 0)
		{
			int j;
			for(j = 0;j<cantArgs;j++)
			{
				variable* var = malloc(sizeof(variable));
				memcpy(var, puntero, sizeof(variable));
				list_add(indiceDeStack[i].argumentos, var);
				puntero += sizeof(variable);
			}
		}
	}
	return indiceDeStack;

}

indStk* crearIndiceDeStack()
{
	indStk* indice = malloc(sizeof(indStk));
	indice->argumentos = list_create();
	indice->variables = list_create();
	indice->posicionDeRetorno = 0;
	indice->variableDeRetorno.posicion.offset = -1;
	indice->variableDeRetorno.posicion.pagina = -1;
	indice->variableDeRetorno.posicion.size = -1;
	indice->variableDeRetorno.identificador = -1;
	return indice;
}

void destruirIndiceDeStack(indStk* indiceStack, int ultimoNivel)
{
	int i;
	for(i = 0;i<=ultimoNivel;i++)
	{
		list_destroy_and_destroy_elements(indiceStack[i].argumentos, free);
		list_destroy_and_destroy_elements(indiceStack[i].variables, free);

	}
	free(indiceStack);
}

void destruirPCB(PCB* pcb)
{
	free(pcb->indiceCodigo);
	free(pcb->indiceEtiqueta);
	destruirIndiceDeStack(pcb->indiceStack, pcb->nivelDelStack);
	free(pcb);
}


serializado serializarOpFS(int tipoOperacion){ //Con parametros globales, No se si esta mejor pasar todos los parametros e ignorar algunos o hacer multiples funciones
	serializado opFSSerializada;
	int sizePathOpFS=sizeof(pathOpFS);
	int sizeBufferOpFS=sizeof(bufferOpFS);
	switch(tipoOperacion){
		case 0||1||4:{
			opFSSerializada.size = sizeof(int) + sizePathOpFS;
			opFSSerializada.data = malloc(opFSSerializada.size);
			break;
		}
		case 2:{
			opFSSerializada.size = sizeof(int)*3 + sizePathOpFS;
			opFSSerializada.data = malloc(opFSSerializada.size);
			break;
		}
		case 3:{
			opFSSerializada.size = sizeof(int)*4 + sizePathOpFS + sizeBufferOpFS;
			opFSSerializada.data = malloc(opFSSerializada.size);
			break;
		}
	}
	if (tipoOperacion>=0 || tipoOperacion<=4){
		sizePathOpFS=sizeof(pathOpFS);
		memcpy(opFSSerializada.data, sizePathOpFS, sizeof(int));
		memcpy(opFSSerializada.data + sizeof(int), pathOpFS, sizePathOpFS);
		if(tipoOperacion==2 || tipoOperacion==3){
			memcpy(opFSSerializada.data + sizeof(int) + sizePathOpFS, offsetOpFS, sizeof(int));
			memcpy(opFSSerializada.data + sizeof(int)*2 + sizePathOpFS, sizeOpFS, sizeof(int));
			if(tipoOperacion==3){
				memcpy(opFSSerializada.data + sizeof(int)*3 + sizePathOpFS, sizeBufferOpFS, sizeof(int));
				memcpy(opFSSerializada.data + sizeof(int)*3 + sizePathOpFS + sizeBufferOpFS, bufferOpFS, sizeBufferOpFS);
			}
		}
	}
	return opFSSerializada;
}

