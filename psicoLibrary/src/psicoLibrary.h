#ifndef SRC_PSICOLIBRARY_H_
#define SRC_PSICOLIBRARY_H_

#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <commons/string.h>
#include <commons/collections/list.h>

typedef struct {
	int nroMarco;
	int desplazamiento;
}t_direccionFisica;

typedef struct {
	uint32_t pid;
	uint32_t tamanioProceso;
}MSJ_KERNEL_MEMORIA_READY;

typedef struct {
	int cantEntradasPorTabla;
	int tamanioPagina;
}MSJ_MEMORIA_CPU_INIT;

typedef struct {
	int idTablaPrimerNivel;
	int pagina;
}MSJ_MEMORIA_CPU_ACCESO_1ERPASO;

typedef struct {
	int idTablaSegundoNivel;
	int pagina;
}MSJ_MEMORIA_CPU_ACCESO_2DOPASO;

typedef struct {
	int nroMarco;
	int desplazamiento;
	int valorAEscribir;
	int pid;
}MSJ_MEMORIA_CPU_WRITE;

typedef struct {
	int nroMarcoOrigen;
	int desplazamientoOrigen;
	int nroMarcoDestino;
	int desplazamientoDestino;
	int pid;
}MSJ_MEMORIA_CPU_COPY;

typedef struct {
	int nroMarco;
	int desplazamiento;
	int pid;
}MSJ_MEMORIA_CPU_READ;

typedef struct {
	int numero;
}MSJ_INT;

typedef struct {
	char* cadena;
}MSJ_STRING;

typedef enum {
	INSTRUCCIONES,    				//entre consola-kernel
	DISPATCH_PCB,     				//entre kernel-cpu
	BLOCK_PCB,						//entre kernel-cpu
	INTERRUPT_INTERRUPCION,			//entre kernel-cpu
	EXIT_PCB,						//entre kernel-cpu
	PASAR_A_READY,					//entre kernel-memoria
	SUSPENDER,						//entre kernel-memoria
	PASAR_A_EXIT,					//entre kernel-memoria
	CONFIG_DIR_LOG_A_FISICA,   	 	//entre cpu-memoria: ESTO ES PARA PASARLE LA CONFIGURACION DE LAS DIRECCIONES, ES EN EL INIT DE LA CPU
	TRADUCCION_DIR_PRIMER_PASO,		//entre cpu-memoria
	TRADUCCION_DIR_SEGUNDO_PASO,	//entre cpu-memoria
	ACCESO_MEMORIA_READ,			//entre cpu-memoria
	ACCESO_MEMORIA_WRITE,			//entre cpu-memoria
	ACCESO_MEMORIA_COPY,			//entre cpu-memoria
	HANDSHAKE_INICIAL,
}t_tipoMensaje;

typedef enum {
	CONSOLA,
	KERNEL,
	CPU,
	MEMORIA_SWAP,
}t_enviadoPor;

typedef struct {
	t_tipoMensaje tipoMensaje;
	int tamanioMensaje;
	t_enviadoPor cliente;
} t_infoMensaje;

typedef struct {
	t_infoMensaje header;
	void* mensaje;
}t_paquete;

typedef struct {
    uint32_t size; // Tamaño del payload
    void* stream; // Payload
} t_buffer;

typedef struct {
	uint8_t codigo_operacion;
    t_buffer* buffer;
} t_pqte;

typedef enum{
	NO_OP,
   IO,
   READ,
   WRITE,
   COPY,
   EXIT
}OPERACION_ID;

typedef struct  {
	OPERACION_ID identificador;
	uint32_t parametro1;
	uint32_t parametro2;
}instruccionConsola;

typedef struct {
	uint32_t id;
	uint32_t tamanio;
	char* instrucciones;
	uint32_t ins_length;
	uint32_t programCounter;
	uint32_t tablaPag; // definir con memoria
	double estimacion_actual;
	double real_anterior;
	double ejecutados_total;
}PCB;

int iniciarCliente(char *ip, char* puerto);
void enviarMensaje(int socket, t_enviadoPor unModulo, void* mensaje, int tamanioMensaje, t_tipoMensaje tipoMensaje);
void enviarPaquete(int socket, t_paquete* paquete);
void recibirMensaje(int socket, t_paquete* paquete);
int recibirDatos(void* paquete, int socket, uint32_t cantARecibir);
int iniciarServidor(char* puerto) ;

void serializarPCB(int socket, PCB* pcb, t_tipoMensaje tipoMensaje);
//void enviarInterruptCPU(int socket, uint32_t pcbId, t_tipoMensaje tipoMensaje);
void crearPaquete(t_buffer* buffer, t_tipoMensaje op, int unSocket);
t_pqte* recibirPaquete(int socket);
PCB* deserializoPCB(t_buffer* buffer);
t_list* leerPseudocodigo(char*);

#endif
