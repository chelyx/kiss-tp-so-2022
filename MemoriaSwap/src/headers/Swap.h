#ifndef HEADERS_SWAP_H_
#define HEADERS_SWAP_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "MemoriaSwap.h"
#include <sys/types.h>
#include <psicoLibrary.h>

typedef enum {
	WRITE_OK,
	WRITE_ERROR,
	FILE_CREATED,
}t_resultadoWriteSwap;

void* read_in_swap(int pid, int numero_frame,int desplazamiento);
t_resultadoWriteSwap write_frame(int pid, int numero_frame, void* bytes);
t_resultadoWriteSwap write_pid_completo(int pid, int tamanio, void* bytes);
t_resultadoWriteSwap copy(int pid, int numero_frame, void* bytes, int desplazamiento);
void delete_in_swap(int pid);
t_resultadoWriteSwap write_pid_completo_lista(int pid, int tamanio, t_list* bytesEnLista);

#endif /* HEADERS_SWAP_H_ */
