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
