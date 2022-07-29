#ifndef MEMORIASWAP_H_
#define MEMORIASWAP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdint.h>
#include <netdb.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdbool.h>
#include <math.h>
#include <stdbool.h>

//headers de commons
#include <commons/string.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>

//mis headers
#include <psicoLibrary.h>

#include "algoritmosReemplazo.h"
#include "peticiones.h"
#include "estructuras.h"
#include "gestionMemoria.h"
#include "utils.h"
#include "Swap.h"

#define PATH_LOG_MEMORIA "../logMemoria.log"
#define PATH_LOG_SWAP "../logSwap.log"
#define PATH_CONFIG_MEMORIASWAP "../configMemoriaSwap.config"

typedef struct {
	char* puertoEscucha;
	int tamanioMemoria;
	int tamanioPagina;
	int entradasPorTabla;
	int retardoMemoria;
	char* algoritmoReemplazo;
	int marcosPorProceso;
	int retardoSwap;
	char* pathSwap;
}t_configMemoriaSwap;

t_log* loggerMemoria;
t_log* loggerSwap;
t_configMemoriaSwap configMemoriaSwap;

pthread_mutex_t mutexSwap;
pthread_mutex_t mutexIdTabla1N;
pthread_mutex_t mutexIdTabla2N;
pthread_mutex_t mutexLista1N;
pthread_mutex_t mutexLista2N;
pthread_mutex_t mutexListaFrames;
pthread_mutex_t mutexVoidMemoriaPrincipal;
pthread_mutex_t mutexAlgoritmoReemplazo;
pthread_mutex_t mutexListaPCB;

int conexion;
int socketAceptado;

void iniciarConfigYLog(char*);
void extraerDatosConfig(t_config* archivoConfig);
void iniciarSemaforos(void);
void iniciarListas(void);
void iniciarModuloMultihilo(void);
void cerrarMemoria(void);

#endif
