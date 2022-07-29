#ifndef TLB_H_
#define TLB_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <commons/collections/list.h>
#include <commons/config.h>
#include <commons/temporal.h>
#include "utils.h"

typedef struct{
	int nroPagina;
	int nroFrame;
} entrada_tlb;

typedef struct tlb{
	t_list* entradas;
	int size; //cantidad de entradas
	char* algoritmo;
} tlb;

//extern tlb* TLB;

tlb* TLB;
int TLBEnable;
pthread_mutex_t mutexTLB;


void iniciar_TLB();
void actualizar_TLB(int nroPagina,int nroFrame);
int buscar_en_TLB(int nroPagina);
void limpiar_entrada_TLB(int nroPagina, int pid);
void limpiar_entradas_TLB();
void reemplazo_fifo(int nroPagina, int nroFrame);
void reemplazo_lru(int nroPagina, int nroFrame);
void cerrar_TLB();
void destruir_entrada(void* entry);


#endif
