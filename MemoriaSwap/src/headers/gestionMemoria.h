#ifndef GESTIONMEMORIA_H_
#define GESTIONMEMORIA_H_

#include "MemoriaSwap.h"

int asignarFrame(int pid);
int algoritmoDeReemplazo(int pid);
void borrarDeRam(int nroFrame);
//void reemplazarEnRam(void* page, int frame, int pid, int frameSwap, t_tabla2N* tabla2, int indice);

#endif
