#ifndef HEADERS_UTILS_H_
#define HEADERS_UTILS_H_

#include <commons/config.h>
#include <psicoLibrary.h>
#include <commons/log.h>
#include <commons/string.h>
#include <semaphore.h>
#include "tlb.h"

#define PATH_CONFIG_CPU "../configCPU.config"
#define PATH_LOG_CPU "../logCPU.log"

typedef struct {
	int entradasTLB;
	char* reemplazoTLB;
	int retardoNOOP;
	char* ipMemoria;
	char* puertoMemoria;
	char* puertoEscuchaDispatch;
	char* puertoEscuchaInterrupt;
	int cantidadEntradasPorTabla;
	int tamanioPagina;
}t_configCPU;

t_configCPU configCPU;
t_log* loggerCPU;
sem_t sem_exec;

int socketMemoria;

t_configCPU extraerDatosConfig(t_config* archivoConfig);
void recibir_config_memoria();
void initCPU(char*);
void recibir_config_memoria();
#endif /* HEADERS_UTILS_H_ */
