#include "headers/utils.h"

int buscarBitDeUsoDePagina(int idPag2N){
	t_tabla2N* tabla2N;
	t_fila2N* pag2N;
	int pag2nBitDeUso = -1;
	pthread_mutex_lock(&mutexLista2N);
	for(int j=0; j<list_size(listaTodasTablas2N); j++){
		tabla2N = list_get(listaTodasTablas2N, j);
		for(int k=0; k<list_size(tabla2N->tablaPaginas2N); k++){
			pag2N = list_get(tabla2N->tablaPaginas2N, k);
			if(pag2N->nroPagina == idPag2N){
				pag2nBitDeUso = pag2N->bitDeUso;
				pthread_mutex_unlock(&mutexLista2N);
				log_debug(loggerMemoria, "ENCONTRE BIT DE USO = %d PARA LA PAG %d", pag2N->bitDeUso, idPag2N);
				return pag2nBitDeUso;
			}
		}
	}
	pthread_mutex_unlock(&mutexLista2N);
	log_error(loggerMemoria, "NO ENCONTRE BIT DE USO PARA LA PAG %d", idPag2N);
	return pag2nBitDeUso;
}

void cambiarBitDePresencia(int idPag2N){
			t_tabla2N* tabla2N;
			t_fila2N* pag2N;
			pthread_mutex_lock(&mutexLista2N);
			for(int j=0; j<list_size(listaTodasTablas2N); j++){
				tabla2N = list_get(listaTodasTablas2N, j);
				for(int k=0; k<list_size(tabla2N->tablaPaginas2N); k++){
					pag2N = list_get(tabla2N->tablaPaginas2N, k);
					if(pag2N->nroPagina == idPag2N){
						pag2N->bitPresencia = 0;
						pag2N->nroFrame = -1;
						pthread_mutex_unlock(&mutexLista2N);
						log_debug(loggerMemoria, "CAMBIE BIT DE PRESENCIA PARA LA PAG %d", idPag2N);
						return ;
					}
				}
			}
			pthread_mutex_unlock(&mutexLista2N);
		}

int buscarBitModificado(int idPag2N){
	t_tabla2N* tabla2N;
	t_fila2N* pag2N;
	int pag2nBitModificado = -1;

	pthread_mutex_lock(&mutexLista2N);
	for(int j=0; j<list_size(listaTodasTablas2N); j++){
		tabla2N = list_get(listaTodasTablas2N, j);
		for(int k=0; k<list_size(tabla2N->tablaPaginas2N); k++){
			pag2N = list_get(tabla2N->tablaPaginas2N, k);
			if(pag2N->nroPagina == idPag2N){
				pag2nBitModificado = pag2N->bitModificado;
				pthread_mutex_unlock(&mutexLista2N);
				log_debug(loggerMemoria, "ENCONTRE BIT DE MODIFICACION = %d PARA LA PAG %d", pag2N->bitModificado, idPag2N);
				return pag2nBitModificado;
			}
		}
	}
	pthread_mutex_unlock(&mutexLista2N);

	log_error(loggerMemoria, "NO ENCONTRE BIT DE MODIFICACION PARA LA PAG %d", idPag2N);
	return pag2nBitModificado;
}

void cambiarBitDeUsoDePagina(int idPag2N){
	t_tabla2N* tabla2N;
	t_fila2N* pag2N;
	pthread_mutex_lock(&mutexLista2N);
	for(int j=0; j<list_size(listaTodasTablas2N); j++){
		tabla2N = list_get(listaTodasTablas2N, j);
		for(int k=0; k<list_size(tabla2N->tablaPaginas2N); k++){
			pag2N = list_get(tabla2N->tablaPaginas2N, k);
			if(pag2N->nroPagina == idPag2N){
				pag2N->bitDeUso = 0;
				pthread_mutex_unlock(&mutexLista2N);
				log_debug(loggerMemoria, "CAMBIE BIT DE USO PARA LA PAG %d", idPag2N);
				return;
			}
		}
	}
	pthread_mutex_unlock(&mutexLista2N);
}

bool buscarProcesoEnTabla(int pid){
	pthread_mutex_lock(&mutexLista1N);
	t_tabla1N* tabla1Naux;
	for(int i = 0; i< list_size(listaTodasTablas1N); i++){
		tabla1Naux = list_get(listaTodasTablas1N, i);
		if(tabla1Naux->idProceso == pid){
			pthread_mutex_unlock(&mutexLista1N);
			return true;
		}
	}
	pthread_mutex_unlock(&mutexLista1N);
	return false;
}

int calcularCantTablasSegundoNivelParaUnProceso(int cantPaginasAUsar) {
	int cantPaginasAUsarAux = cantPaginasAUsar;
	int cantTablasSegundoNivel = 0;
	while(cantPaginasAUsarAux >= configMemoriaSwap.entradasPorTabla){
		cantTablasSegundoNivel++;
		cantPaginasAUsarAux = cantPaginasAUsarAux - configMemoriaSwap.entradasPorTabla;
	}
	if(cantPaginasAUsarAux != 0) {
		cantTablasSegundoNivel++;
	}
	return cantTablasSegundoNivel;
}

int buscarNroTabla1N(int pid){
	pthread_mutex_lock(&mutexLista1N);
	t_tabla1N* tabla1Naux;
	for(int i = 0; i< list_size(listaTodasTablas1N); i++){
		tabla1Naux = list_get(listaTodasTablas1N, i);
		if(tabla1Naux->idProceso == pid){
			int nroTabla1N = tabla1Naux->idTabla;
			pthread_mutex_unlock(&mutexLista1N);
			return nroTabla1N;
		}
	}
	pthread_mutex_unlock(&mutexLista1N);
	return -1; //no encontro el nro de la tabla de 1N
}

t_tabla1N* buscarTabla1N(int idTabla){
	pthread_mutex_lock(&mutexLista1N);
	t_tabla1N* tabla1Naux;
	for(int i = 0; i< list_size(listaTodasTablas1N); i++){
		tabla1Naux = list_get(listaTodasTablas1N, i);
		if(tabla1Naux->idTabla == idTabla){
			pthread_mutex_unlock(&mutexLista1N);
			return tabla1Naux;
		}
	}
	pthread_mutex_unlock(&mutexLista1N);
	return NULL; //no encontro la tabla1N
}

int buscarIdTabla2NEnUnaLista(t_list* tablaPaginas1N, int pagina2N){
	t_fila1N* fila1;
	t_fila2N* fila2N;
	t_tabla2N* tabla2N;
	int nroTabla2N = -1;

	pthread_mutex_lock(&mutexLista1N);
	for(int i=0; i< list_size(tablaPaginas1N); i++){
		fila1 = list_get(tablaPaginas1N, i);
		pthread_mutex_lock(&mutexLista2N);
			for(int j=0; j < list_size(listaTodasTablas2N); j++){
				tabla2N = list_get(listaTodasTablas2N, j);
				if(fila1->nroTabla2N == tabla2N->idTabla) {
					for(int z = 0; z<list_size(tabla2N->tablaPaginas2N); z++){
						fila2N = list_get(tabla2N->tablaPaginas2N, z);
						if(fila2N->nroPagina == pagina2N){
							nroTabla2N = tabla2N->idTabla;
							pthread_mutex_unlock(&mutexLista2N);
							pthread_mutex_unlock(&mutexLista1N);
							return nroTabla2N;
						}
					}
				}
			}
		pthread_mutex_unlock(&mutexLista2N);
	}
	pthread_mutex_unlock(&mutexLista1N);
	return nroTabla2N; //= -1 no encontro el idTabla2 para esa lista de filas1N
}

t_tabla2N* buscarTabla2N(int idTabla){
	pthread_mutex_lock(&mutexLista2N);
	t_tabla2N* tabla2Naux;
	for(int i = 0; i< list_size(listaTodasTablas2N); i++){
		tabla2Naux = list_get(listaTodasTablas2N, i);
		if(tabla2Naux->idTabla == idTabla){
			pthread_mutex_unlock(&mutexLista2N);
			return tabla2Naux;
		}
	}
	pthread_mutex_unlock(&mutexLista2N);
	return NULL; //no encontro la tabla2N
}

int buscarFrameEnFilaEnListaConPagina(int pagina, t_list* tablaPaginas2Nivel){
	t_fila2N* fila2;
	int frameBuscado;
	for(int i=0; i<list_size(tablaPaginas2Nivel); i++){
		fila2 = list_get(tablaPaginas2Nivel, i);
		if(fila2->nroPagina == pagina){
			frameBuscado = fila2->nroFrame;
			break;
		}
	}
	return frameBuscado;
}

int hayFrameLibreParaElPid(int pid){
	pthread_mutex_lock(&mutexListaFrames);
	for(int i=0; i<list_size(tablaDeFrames); i++){
		if(     (((t_filaFrame*)list_get(tablaDeFrames, i))->idProceso == pid) &&
				(((t_filaFrame*)list_get(tablaDeFrames, i))->estado == NO_OCUPADO) &&
				(((t_filaFrame*)list_get(tablaDeFrames, i))->nroPagina == -1)){ //esto es si ya hay una página de ese proceso
			pthread_mutex_unlock(&mutexListaFrames);
			return ((t_filaFrame*)list_get(tablaDeFrames, i))->nroFrame; //ES UN FRAME LIBRE
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
	return -1; //NO HAY FRAMES  LIBRES PARA ESE PID

}



