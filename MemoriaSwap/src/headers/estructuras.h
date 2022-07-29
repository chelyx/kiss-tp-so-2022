#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include "MemoriaSwap.h"

typedef enum {
	//ESTO DEFINE SI LA PAG 2N VA A SER UNA PAG QUE SE CARGARA EN RAM Y SWAP,
	//O SI VA A SER UNA PAG INUTIL QUE SOLO SE CREA PORQUE HAY UN LIMITE
	//DETERMINADO DE ENTRADAS A CREAR POR TABLA.
	PAG_A_USAR,
	PAG_INUTIL,
}t_tipoPagina;

typedef struct {
	int nroPagina;
	int nroTabla2N;  //es el idTabla de t_tabla2N
}t_fila1N;

typedef struct {
	int nroPagina;
	int nroFrame;
	int bitPresencia; //1: esta en ram. 0: no esta en ram
	int bitDeUso;
	int bitModificado; //0: no esta modificado. 1: esta modificado
	int posicionSwap;
	t_tipoPagina tipo;
}t_fila2N;

typedef struct {
	int idTabla;
	int idProceso;
	t_list* tablaPaginas2N; //lista de t_fila2N
}t_tabla2N;

typedef struct {
	int idTabla;
	int idProceso;
	t_list* tablaPaginas1N; //lista de t_fila1N
}t_tabla1N;

typedef enum {
	NO_OCUPADO = 0,
	OCUPADO = 1,
}t_estadoFrame;

typedef struct {
	int idProceso;
	int nroPagina;
	int nroFrame;
	t_estadoFrame estado;
	//void* frameVoid;
}t_filaFrame;

typedef struct {
	int pid;
	int tamanioProceso;
}pcb_mem;

t_list* tablaDeFrames;				 //lista de t_filaFrame
t_list* listaTodasTablas1N;			 //lista de t_tabla1N
t_list* listaTodasTablas2N;			 //lista de t_tabla2N
t_list* listaPCB;						// pcb_mem

void* memoriaPrincipal; //Espacio de usuario
int posicionClock;

int idTabla1N;
int idTabla2N;

int cantidadFramesTotal;
int tamanioDeCadaFrame;
int cantidadProcesosMaximosEnRam;


int crearTablasParaUnProceso(int pid, int tamanioProceso);
void iniciarEstructuras(void);
void iniciarFrames(void);
void iniciarCarpetaSwap();
void liberarFrame_Tabla(int nroFrame);
t_tabla1N* buscarTabla1N(int idTabla);
t_tabla2N* buscarTabla2N(int idTabla);
void reemplazarEnRam(void* page, int frameAReemplazar, int pid, int frameSwap, t_fila2N* pag2N, int indice, t_fila2N* pagNUEVA);
void freeFrame(t_filaFrame* frame);
void free2N(t_tabla2N* tabla2N);
void free1N(t_tabla1N* tabla1N);
void cambiarBitDePresencia(int idPag2N);


#endif
