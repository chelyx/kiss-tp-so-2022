#ifndef CPU_H_
#define CPU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <semaphore.h>
#include <pthread.h>
#include <psicoLibrary.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include "utils.h"
#include "tlb.h"
#include "MMU.h"

#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

#include <commons/config.h>
#include <commons/collections/list.h>


bool interrupciones;
pthread_t hiloDispatch;
pthread_t hiloInterrupt;

pthread_mutex_t mutex_interrupciones;

int socketMemoria;
int socketAceptadoDispatch;
int socketAceptadoInterrupt;

void iniciarDispatch();

void iniciarInterrupt();
void cicloInstruccion();
void iniciar_dispatch();
void iniciar_interrupt();
t_direccionFisica* calcularDireccionFisica(uint32_t id_tabla_de_paginas_1er_nivel,uint32_t direccionLogica);
void cerrarCPU();
#endif
