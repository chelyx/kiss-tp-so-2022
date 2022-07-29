#include "headers/Swap.h"

void* read_in_swap(int pid, int numero_frame,int desplazamiento){
	log_debug(loggerSwap, "[INIT - READ_SWAP] PID: %d FRAME: %d OFFSET: %d", pid, numero_frame, desplazamiento);

	bool _criterio_remocion_lista(void* elemento) {
		return (((pcb_mem*)elemento)->pid == pid);
	}

	pthread_mutex_lock(&mutexListaPCB);
	int tamanioProceso = ((pcb_mem*)	list_find(listaPCB, _criterio_remocion_lista))->tamanioProceso;
	pthread_mutex_unlock(&mutexListaPCB);

    char* path = string_new();
    string_append(&path, configMemoriaSwap.pathSwap);
    int tamanio_pagina = configMemoriaSwap.tamanioPagina;
    int tamanio_swap = tamanioProceso;
    int posicion = tamanio_pagina*numero_frame + desplazamiento;
    int posicion_final = posicion + tamanioDeCadaFrame -desplazamiento;
    char* new_pid = string_new();
    string_append(&new_pid, string_itoa(pid));
    string_append(&path, "/");
    string_append(&path, new_pid ); //se puede abstraer
    string_append(&path, ".swap");
    int fd = open(path,O_RDWR, S_IRUSR| S_IWUSR);
    char* maped = mmap(NULL,tamanio_swap, PROT_WRITE, MAP_SHARED,fd,0);
    char* bytes = malloc(tamanioDeCadaFrame-desplazamiento);
    for(int i = 0 ; i < tamanioDeCadaFrame-desplazamiento ; i++ ){
        bytes[i] = maped[posicion];
        posicion++;
        if(posicion == posicion_final){
        	break;
        }
    }
    int error = munmap(maped,tamanio_swap);
    close(fd);
    if(error != 0){
        free(bytes);
        free(path);
    	log_error(loggerSwap, "[READ_SWAP] NPID: %d FRAME: %d OFFSET: %d", pid, numero_frame, desplazamiento);
        return "ERROR";
    }
    else{
        free(path);
        char* bytesSubstring = string_substring(bytes, 0, tamanioDeCadaFrame-desplazamiento);
    	log_debug(loggerSwap, "[FIN - READ_SWAP] PID: %d FRAME: %d OFFSET: %d", pid, numero_frame, desplazamiento);
        return bytesSubstring;
    }
}

t_resultadoWriteSwap write_frame(int pid, int numero_frame, void* bytes){
	log_debug(loggerSwap, "[INIT - WRITE_FRAME_SWAP] PID: %d FRAME: %d", pid, numero_frame);

	bool _criterio_remocion_lista(void* elemento) {
		return (((pcb_mem*)elemento)->pid == pid);
	}

	pthread_mutex_lock(&mutexListaPCB);
	int tamanioProceso = ((pcb_mem*)	list_find(listaPCB, _criterio_remocion_lista))->tamanioProceso;
	pthread_mutex_unlock(&mutexListaPCB);

	char* bytes_char = string_new();
	string_append(&bytes_char, (char *)bytes);
	char* path = string_new();
	//int tamanio_pagina = 64;
	//int tamanio_swap = tamanio_pagina * 4;
	string_append(&path,configMemoriaSwap.pathSwap);
	int tamanio_pagina = configMemoriaSwap.tamanioPagina;
	int tamanio_swap = tamanioProceso;
	int posicion = tamanio_pagina*numero_frame;
	int posicion_final = posicion + tamanio_pagina;
	char* new_pid = string_new();
	string_append(&new_pid, string_itoa(pid));
	string_append(&path, "/");
	string_append(&path, new_pid ); //se puede abstraer
	string_append(&path, ".swap");
	int fd = open(path,O_RDWR, S_IRUSR| S_IWUSR);

	char* maped = mmap(NULL,tamanio_swap, PROT_WRITE, MAP_SHARED,fd,0);
	for(int i = 0 ; i < tamanio_swap ; i++ ){
		maped[posicion] = bytes_char[i];
		posicion++;
		if(posicion == posicion_final){
			break;
		}
	}
	int error = munmap(maped,tamanio_swap);
	if(error != 0){
		free(bytes);
		free(path);
		log_error(loggerSwap, "[WRITE_FRAME_SWAP] MAPPING FAILED PID: %d FRAME: %d", pid, numero_frame);
		return WRITE_ERROR;
	}
	else{
		free(path);
		log_debug(loggerSwap, "[FIN - WRITE_FRAME_SWAP] PID: %d FRAME: %d", pid, numero_frame);
		return WRITE_OK;
	}
}

t_resultadoWriteSwap write_pid_completo(int pid, int tamanio, void* bytes){
	log_debug(loggerSwap, "[INIT - WRITE_NEW_PID_SWAP] PID: %d, tamanio %i", pid, tamanio);
	char* bytes_char = string_new();
	string_append(&bytes_char, (char *)bytes);
	char* path = string_new();
	string_append(&path,configMemoriaSwap.pathSwap);
	//int tamanio_pagina = configMemoriaSwap.tamanioPagina;
	int tamanio_swap = tamanio;
	int posicion_final = tamanio;
	char* new_pid = string_new();
	string_append(&new_pid, string_itoa(pid));
	string_append(&path, "/");
	string_append(&path, new_pid ); //se puede abstraer
	string_append(&path, ".swap");
	FILE *f = fopen(path,"w");
	ftruncate(fileno(f),tamanio_swap);

	fclose(f);

	int fd = open(path,O_RDWR, S_IRUSR| S_IWUSR);
	char* maped = mmap(NULL,tamanio_swap, PROT_WRITE, MAP_SHARED,fd,0);

	for(int posicion = 0 ; posicion < tamanio_swap ; posicion++ ){
		maped[posicion] = bytes_char[posicion];
		if(posicion == posicion_final){
			break;
		}
	}
	int error = munmap(maped,tamanio_swap);
	if(error != 0){
		free(bytes);
		free(path);
		free(new_pid);
		log_error(loggerSwap, "[WRITE_NEW_PID_SWAP] MAPPING FAILED PID: %d", pid);
		return WRITE_ERROR;
	}
	else{
		free(path);
		free(new_pid);
		log_debug(loggerSwap, "[FIN - WRITE_NEW_PID_SWAP] PID: %d", pid);
		return WRITE_OK;
	}
}

t_resultadoWriteSwap copy(int pid, int numero_frame, void* bytes, int desplazamiento){
	log_debug(loggerSwap, "[INIT - COPY_SWAP] PID: %d FRAME: %d", pid, numero_frame);
	bool _criterio_remocion_lista(void* elemento) {
		return (((pcb_mem*)elemento)->pid == pid);
	}

	pthread_mutex_lock(&mutexListaPCB);
	int tamanioProceso = ((pcb_mem*)	list_find(listaPCB, _criterio_remocion_lista))->tamanioProceso;
	pthread_mutex_unlock(&mutexListaPCB);
	char* bytes_char = string_new();
	string_append(&bytes_char, (char *)bytes);
	char* path = configMemoriaSwap.pathSwap;
	int tamanio_pagina = configMemoriaSwap.tamanioPagina;
	int tamanio_swap = tamanioProceso;
	int posicion = tamanio_pagina*numero_frame;
	int posicion_final = posicion + desplazamiento;
	char* new_pid = string_new();
	string_append(&new_pid, string_itoa(pid));
	string_append(&path, "/home/utnso/swap/");
	string_append(&path, new_pid ); //se puede abstraer
	string_append(&path, ".swap");
	int fd = open(path,O_RDWR, S_IRUSR| S_IWUSR);

	char* maped = mmap(NULL,tamanio_swap, PROT_WRITE, MAP_SHARED,fd,0);
	for(int i = 0 ; i < tamanio_swap ; i++ ){
		maped[posicion] = bytes_char[i];
		posicion++;
		if(posicion == posicion_final){
			break;
		}
	}
	int error = munmap(maped,tamanio_swap);
	if(error != 0){
		free(bytes);
		free(path);
		log_error(loggerSwap, "[COPY_SWAP] MAPPING FAILED PID: %d FRAME: %d", pid, numero_frame);
		return WRITE_ERROR;
	}
	else{
		free(path);
		log_debug(loggerSwap, "[FIN - COPY_SWAP] PID: %d FRAME: %d", pid, numero_frame);
		return WRITE_OK;
	}
}

void delete_in_swap(int pid){
	log_debug(loggerSwap, "[INIT - DELETE_PID_SWAP] PID: %d", pid);
	char* path = string_new();
    string_append(&path, string_itoa(pid) ); //se puede abstraer
    string_append(&path, ".swap");
	if (remove(path) == 0){
		log_debug(loggerSwap, "[FIN - DELETE_PID_SWAP] PID: %d", pid);
	}
    else{
    	log_error(loggerSwap, "[DELETE_PID_SWAP] PID: %d", pid);
    }
}

t_resultadoWriteSwap write_pid_completo_lista(int pid, int tamanio, t_list* bytesEnLista){
	log_debug(loggerSwap, "[INIT - WRITE_NEW_PID_SWAP] PID: %d", pid);
	char* bytes_char = string_new();
	for(int indice=0; indice<list_size(bytesEnLista); indice++){
	    string_append(&bytes_char, string_substring((char*)list_get(bytesEnLista, indice), 0, configMemoriaSwap.tamanioPagina));
	}

	bool _criterio_remocion_lista(void* elemento) {
		return (((pcb_mem*)elemento)->pid == pid);
	}

	pthread_mutex_lock(&mutexListaPCB);
	int tamanioProceso = ((pcb_mem*)	list_find(listaPCB, _criterio_remocion_lista))->tamanioProceso;
	pthread_mutex_unlock(&mutexListaPCB);

	char* path = string_new();
	string_append(&path,configMemoriaSwap.pathSwap);
	int tamanio_swap = tamanioProceso;
	int posicion_final = tamanio;
	char* new_pid = string_new();
	string_append(&new_pid, string_itoa(pid));
	string_append(&path, "/");
	string_append(&path, new_pid ); //se puede abstraer
	string_append(&path, ".swap");
	FILE *f = fopen(path,"w");
	ftruncate(fileno(f),tamanio_swap);

	fclose(f);

	int fd = open(path,O_RDWR, S_IRUSR| S_IWUSR);
	char* maped = mmap(NULL,tamanio_swap, PROT_WRITE, MAP_SHARED,fd,0);

	for(int posicion = 0 ; posicion < tamanio_swap ; posicion++ ){
		maped[posicion] = bytes_char[posicion];
		if(posicion == posicion_final){
			break;
		}
	}
	int error = munmap(maped,tamanio_swap);
	if(error != 0){
		free(path);
		free(new_pid);
		free(bytes_char);
		log_error(loggerSwap, "[WRITE_NEW_PID_SWAP] MAPPING FAILED PID: %d", pid);
		return WRITE_ERROR;
	}
	else{
		free(path);
		free(new_pid);
		free(bytes_char);
		log_debug(loggerSwap, "[FIN - WRITE_NEW_PID_SWAP] PID: %d", pid);
		return WRITE_OK;
	}
}

