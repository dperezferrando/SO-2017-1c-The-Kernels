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

char* serializacionPcb (PCB pcb){
	int numeroInstrucciones;//Este parametro viene de algun lado

	void *buffer = malloc(16+8*numeroInstrucciones);
	memcpy(buffer, &(pcb.pid), 4);
	memcpy(buffer + 4, &(pcb.programCounter), 4);
	memcpy(buffer + 8, &(pcb.cantPaginasCodigo), 4);
	memcpy(buffer + 12, &(numeroInstrucciones), 4);
	for(int i=0;i<=numeroInstrucciones;i++){
		memcpy(buffer + 12 + 4*(i+1), &(pcb.indiceCodigo.offset[i]), 4);
		memcpy(buffer + 16 + 4*(i+1), &(pcb.indiceCodigo.longitud[i]), 4);
	}
	memcpy(buffer + 12 + 8*numeroInstrucciones , &(pcb.exitCode), 4);
	//Falta:
	//indEtq indiceEtiqueta;
	//indStk indiceStack;
	return buffer;
}

PCB deserializacionPcb (char* buffer){
	PCB pcb;
	memcpy(&(pcb->pid), buffer, 4);
	memcpy(&(pcb->programCounter), buffer + 4, 4);
	memcpy(&(pcb->cantPaginasCodigo), buffer + 8, 4);
	int n1 = memcpy(buffer + 12, 4);
	for(int i=0;i<=n1;i++){
		memcpy(&(pcb->indiceCodigo->offset[i]), buffer + 12 + 4*(i+1), 4);
		memcpy(&(pcb->indiceCodigo->longitud[i]),buffer + 16 + 4*(i+1), 4);
	}
	//Falta:
	//indEtq indiceEtiqueta;
	//indStk indiceStack;
	memcpy(&(pcb->exitCode), buffer + 12 + 8*n1 , 4);
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
