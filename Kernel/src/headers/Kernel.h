#ifndef KERNEL_H_
#define KERNEL_H_


#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include "Utils.h"
#include <sys/time.h>

typedef struct {
	char* mensaje;
	int socket_consola;
}args_pcb;

typedef enum{
	NEW,
    READY,
    EXEC,
    BLOCKED,
    SUSP_BLOCKED,
    SUSP_READY,
	EXITED
}ESTADOS_PCB;

typedef struct {
	int id;
	t_list* instrucciones;
}instruccion_x_pcb;

struct timeval timeValBefore;
struct timeval timeValAfter;

void crearPCB(void*);
void iniciar_planif_largo_plazo();
void iniciar_planif_corto_plazo();
void iniciar_mensajes_cpu();
void iniciar_planif_io();

void consolaMultihilo();
void recibirMensajeCPU();

void planifLargoPlazo();
void planifMedianoPlazoSuspReady();
void planifCortoPlazo();
void planifBloqueados();

void hiloSuspendedor(PCB*);

int enviarMensajeMemoriaSwapReady(MSJ_KERNEL_MEMORIA_READY* mensaje);

PCB* algoritmo_SRT();
PCB* algoritmo_FIFO();

#endif
