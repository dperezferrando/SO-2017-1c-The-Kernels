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
	memcpy(&pcb->pid, pcbSerializado, sizeof(int));
	memcpy(&pcb->cantPaginasCodigo, pcbSerializado + sizeof(int), sizeof(int));
	memcpy(&pcb->programCounter, pcbSerializado + (sizeof(int)*2), sizeof(int));
	memcpy(&pcb->sizeIndiceCodigo, pcbSerializado + (sizeof(int)*3), sizeof(int));
	pcb->indiceCodigo = malloc(pcb->sizeIndiceCodigo);
	memcpy(pcb->indiceCodigo, pcbSerializado + (sizeof(int)*4), pcb->sizeIndiceCodigo);
	return pcb;

}

PCBSerializado serializarPCB(PCB* pcb) // A SER REEMPLAZADO POR LO DE NICO
{
	PCBSerializado pcbSerializado;
	pcbSerializado.size = sizeof(int)*4 + pcb->sizeIndiceCodigo;
	pcbSerializado.data = malloc(pcbSerializado.size);
	memcpy(pcbSerializado.data, &pcb->pid, sizeof(int));
	memcpy(pcbSerializado.data + sizeof(int), &pcb->cantPaginasCodigo, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*2), &pcb->programCounter, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*3), &pcb->sizeIndiceCodigo, sizeof(int));
	memcpy(pcbSerializado.data + (sizeof(int)*4), pcb->indiceCodigo, pcb->sizeIndiceCodigo);
	return pcbSerializado;

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
