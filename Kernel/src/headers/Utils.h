/*
 * Utils.h
 *
 *  Created on: 19 abr. 2022
 *      Author: utnso
 */

#ifndef HEADERS_UTILS_H_
#define HEADERS_UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <commons/string.h>
#include <string.h>
#include <commons/config.h>
#include <commons/collections/list.h>
#include <commons/log.h>
#include <psicoLibrary.h>
#include "Kernel.h"

#define PATH_LOG_KERNEL "../logKernel.log"
#define PATH_CONFIG_KERNEL "../configKernel.config"

typedef struct {
	char* ipCPU;
	char* ipMemoria;
	char* puertoMemoria;
	char* puertoCPUDispatch;
	char* puertoCPUInterrupt;
	char* puertoEscucha;
	char* algoritmo;
	int estimacionInicial;
	double alfa;
	int gradoMultiprogramacion;
	int tiempoMaximoBloqueado;
}t_configKernel;

t_configKernel configKernel;
t_log* loggerKernel;
int ID_PROXIMO;

int socketMemoriaSwap;
int socketCpuDispatch;
int socketCpuInterrupt;

// LISTAS
t_list* LISTA_NEW;
t_list* LISTA_READY;
t_list* LISTA_EXEC;
t_list* LISTA_BLOCKED;
t_list* LISTA_BLOCKED_SUSPENDED;
t_list* LISTA_READY_SUSPENDED;
t_list* LISTA_EXIT;
t_list* COLA_BLOQUEO_IO;
t_list* LISTA_SOCKETS;
t_list* instrucciones_x_pcb;

// MUTEX
pthread_mutex_t mutex_creacion_ID;
pthread_mutex_t mutex_lista_new;
pthread_mutex_t mutex_lista_ready;
pthread_mutex_t mutex_lista_exec;
pthread_mutex_t mutex_lista_blocked;
pthread_mutex_t mutex_lista_blocked_suspended;
pthread_mutex_t mutex_lista_ready_suspended;
pthread_mutex_t mutex_lista_exit;
pthread_mutex_t mutex_lista_cola_io;
pthread_mutex_t mutex_lista_instrxpcb;

// SEMAFOROS
sem_t sem_planif_largo_plazo;
sem_t contador_multiprogramacion;
sem_t sem_ready;
sem_t sem_bloqueo;
sem_t sem_procesador;

// PTHREAD
pthread_t planificador_largo_plazo;
pthread_t planificador_corto_plazo;
pthread_t mensajes_cpu;
pthread_t planif_io;

void initKernel(char*);
void cerrarKernel();
void iniciarConfigYLog(char*);
t_configKernel extraerDatosConfig(t_config* archivoConfig);

int tamanioPCB(PCB* pcb);
void enviarInterruptCPU();

void pasar_a_new(PCB* pcb);
void pasar_a_ready(PCB* pcb);
void pasar_a_exec(PCB* pcb);
void pasar_a_exit(PCB* pcb);
void pasar_a_susp_ready(PCB*);
void pasar_a_susp_block(PCB* pcb);
void pasar_a_block(PCB*);


#endif /* HEADERS_UTILS_H_ */
