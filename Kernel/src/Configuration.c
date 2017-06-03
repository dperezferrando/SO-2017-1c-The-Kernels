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
	pcb->indiceStack = deserializarIndiceDeStack(indiceStackSerializado);
	memcpy(puntero, &pcb->exitCode, sizeof(int));
	free(indiceStackSerializado.data);
	return pcb;

}

serializado serializarPCB(PCB* pcb) // A SER REEMPLAZADO POR LO DE NICO
{
	serializado pcbSerializado;
	serializado indiceStackSerializado = serializarIndiceDeStack(pcb->indiceStack);
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

serializado serializarIndiceDeStack(indStk* indiceStack)
{
	serializado indiceStackSerializado;
	int cantVars = list_size(indiceStack->variables);
	int cantArgs = list_size(indiceStack->argumentos);
	indiceStackSerializado.size = (cantVars+cantArgs)*sizeof(variable)+sizeof(variable)+sizeof(int)*3;
	indiceStackSerializado.data = malloc(indiceStackSerializado.size);
	char* puntero = indiceStackSerializado.data;
	memcpy(puntero, &indiceStack->posicionDeRetorno, sizeof(int));
	puntero += sizeof(int);
	memcpy(puntero, &indiceStack->variableDeRetorno, sizeof(variable));
	puntero += sizeof(variable);
	memcpy(puntero, &cantVars, sizeof(int));
	puntero += sizeof(int);
	void serializar(variable* unaVariable)
	{
		memcpy(puntero, unaVariable, sizeof(variable));
		puntero += sizeof(variable);
	}
	if(cantVars != 0)
		list_iterate(indiceStack->variables, serializar);
	memcpy(puntero, &cantArgs, sizeof(int));
	puntero += sizeof(int);
	if(cantArgs != 0)
		list_iterate(indiceStack->argumentos, serializar);
	return indiceStackSerializado;

}

indStk* deserializarIndiceDeStack(serializado indiceSerializado)
{
	indStk* indiceDeStack = malloc(sizeof(indStk));
	char* puntero = indiceSerializado.data;
	memcpy(&indiceDeStack->posicionDeRetorno, puntero, sizeof(int));
	puntero += sizeof(int);
	memcpy(&indiceDeStack->variableDeRetorno, puntero, sizeof(variable));
	puntero += sizeof(variable);
	int cantVars;
	memcpy(&cantVars, puntero, sizeof(int));
	puntero += sizeof(int);
	indiceDeStack->variables = list_create();
	indiceDeStack->argumentos = list_create();
	if(cantVars != 0)
	{
		int i;
		for(i = 0;i<cantVars;i++)
		{
			variable* var = malloc(sizeof(variable));
			memcpy(var, puntero, sizeof(variable));
			list_add(indiceDeStack->variables, var);
			puntero+= sizeof(variable);
		}
	}
	int cantArgs;
	memcpy(&cantArgs, puntero, sizeof(int));
	puntero += sizeof(int);
	if(cantArgs != 0)
	{
		int i;
		for(i = 0;i<cantArgs;i++)
		{
			variable* var = malloc(sizeof(variable));
			memcpy(var, puntero, sizeof(variable));
			list_add(indiceDeStack->argumentos, var);
			puntero += sizeof(variable);
		}
	}
	return indiceDeStack;

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

/*PCBSerializado serializarPCB (PCB* pcb){
	PCBSerializado pcbSerializado;
	pcbSerializado.size = sizeof(int)*5 + pcb->sizeIndiceCodigo;
	pcbSerializado.data = malloc(pcbSerializado.size);
	memcpy(pcbSerializado.data, &pcb->pid, sizeof(int));
	memcpy(pcbSerializado.data + 4, &pcb->programCounter, sizeof(int));
	memcpy(pcbSerializado.data + 8, &pcb->cantPaginasCodigo, sizeof(int));
	memcpy(pcbSerializado.data + 12, &pcb->exitCode, sizeof(int));
	memcpy(pcbSerializado.data + 16, &pcb->sizeIndiceCodigo, sizeof(int));
	memcpy(pcbSerializado.data + 20, pcb->indiceCodigo, pcb->sizeIndiceCodigo);

	//Falta:
	//indEtq indiceEtiqueta;
	//indStk indiceStack;
	return pcbSerializado;
}

PCB* deserializarPCB (char* buffer){
	PCB* pcb = malloc(sizeof(PCB));
	memcpy(&pcb->pid, buffer, sizeof(int));
	memcpy(&pcb->programCounter, buffer + 4, sizeof(int));
	memcpy(&pcb->cantPaginasCodigo, buffer + 8, sizeof(int));
	memcpy(&pcb->exitCode, buffer + 12, sizeof(int));
	memcpy(&pcb->sizeIndiceCodigo, buffer + 16, sizeof(int));
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	memcpy(pcb->indiceCodigo, buffer + 20, pcb->sizeIndiceCodigo);
	//Falta:
	//indEtq indiceEtiqueta;
	//indStk indiceStack;
	return pcb;
}

//SERIALIZACION BIBLICA
/*char *serializarPCB(t_pcb *pcb) {
	int g=0;
	//printf("Entre a serialziar\n");
	int size = 0;
	char *retorno, *retornotemp, *retornotempp;
	size += sizeof(t_pcb);
	size += pcb->sizeIndiceDeEtiquetas * sizeof(char);;
	size += pcb->sizeIndiceDeCodigo * 2 * sizeof(int);;
	int i, y;
	//size += pcb->sizeContextoActual * sizeof(t_contexto*);
	for (i = 0; i < pcb->sizeContextoActual; i++) {
		size += sizeof(t_contexto);
		int y;
		t_contexto * contexto;
		contexto = list_get(pcb->contextoActual, i);
		for (y = 0; y < contexto->sizeArgs; y++) {
			size += sizeof(t_direccion);
		}
		for (y = 0; y < contexto->sizeVars; y++) {
			size += sizeof(t_variable);
			size += sizeof(t_direccion);
		}
	}
	retorno = malloc(size);
	retornotemp = retorno;
	pcb->sizeTotal = size;
	memcpy(retornotemp, pcb, sizeof(t_pcb));
	retornotemp += sizeof(t_pcb);
	memcpy(retornotemp, pcb->indiceDeCodigo, pcb->sizeIndiceDeCodigo * 2 * sizeof(int));
	retornotemp += pcb->sizeIndiceDeCodigo * 2 * sizeof(int);
	memcpy(retornotemp, pcb->indiceDeEtiquetas, pcb->sizeIndiceDeEtiquetas * sizeof(char));
	retornotempp = retornotemp;
	retornotemp += pcb->sizeIndiceDeEtiquetas * sizeof(char);
	//memcpy(retornotemp, pcb->contextoActual, pcb->sizeContextoActual * sizeof(t_contexto*));
	//retornotemp += pcb->sizeContextoActual * sizeof(t_contexto*);
	//printf("ACA DENO ENTERARa\n");
	for (i = 0; i < pcb->sizeContextoActual; i++) {
		t_contexto *contexto;
		contexto = list_get(pcb->contextoActual, i);
		memcpy(retornotemp, contexto, sizeof(t_contexto));
		retornotemp += sizeof(t_contexto);
		for (y = 0; y < contexto->sizeArgs; y++) {
			t_direccion *dir;
			dir = list_get(contexto->args, y);
			memcpy(retornotemp, dir, sizeof(t_direccion));
			retornotemp += sizeof(t_direccion);
		}
		for (y = 0; y < contexto->sizeVars; y++) {
			t_variable *var;
			t_direccion *dir;
			var = list_get(contexto->vars, y);
			memcpy(retornotemp, var, sizeof(t_variable));
			retornotemp += sizeof(t_variable);
			t_direccion * diretemp = var->direccion;
			//	printf("ENCODING %d %d %d \n",diretemp->offset,diretemp->pagina,diretemp->size);
			memcpy(retornotemp,&diretemp->offset,4);
			memcpy(retornotemp+4,&diretemp->size,4);
			memcpy(retornotemp+8,&diretemp->pagina,4);
			//memcpy(retornotemp, diretemp, sizeof(t_direccion));
			//	printf("posicion %d \n",(retornotemp-retorno));
			retornotemp += sizeof(t_direccion);
			}
		}
	//printf("SIZE%d\n",size);
	return retorno;
}*/
