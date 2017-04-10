#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <commons/config.h>
#define CONFIG_FILE "Kernel.conf"

bool rutaCorrecta(t_config* configHandler);
bool archivoConfigValido(t_config* configHandler,char* []);
bool archivoConfigCompleto(t_config* configHandler, char* []);
bool configurate(char* ,void(t_config*), char* []);
