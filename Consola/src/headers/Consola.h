#ifndef CONSOLA_H_
#define CONSOLA_H_

#include <stdio.h>
#include <stdlib.h>
#include <commons/string.h>
#include <string.h>
#include <commons/config.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <psicoLibrary.h>

#define PATH_CONFIG_CONSOLA "../cfg/configConsola.config"
#define PATH_LOG_CONSOLA "../logConsola.log"


t_log* LOGGER;

void liberarListaIns();
char* crearMensaje(char*, char*);

#endif
