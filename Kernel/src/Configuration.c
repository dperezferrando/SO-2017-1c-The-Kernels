#import "Configuration.h"

bool archivoConfigCompleto(t_config* configHandler, char* keys[])
{
	bool archivoValido = true;
	int i = 0;
	for(i = 0;i<config_keys_amount(configHandler);i++)
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

bool configurate(char* ruta,void(*handleConfigFile)(t_config*), char* keys[]) {
	t_config* configHandler = config_create(ruta);
	if(!archivoConfigValido(configHandler,keys))
		return EXIT_FAILURE;
	handleConfigFile(configHandler);
	return EXIT_SUCCESS;
}
