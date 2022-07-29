#include "headers/gestionMemoria.h"

int algoritmoDeReemplazo(int pid){
	if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
		return reubicarClockModificado(pid);
	}
	else if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK")){
		return reubicarClock(pid);
	}
	else {
		log_error(loggerMemoria,"[NAME ALGORITMO] EL NOMBRE DEL ALGORITMO EN CONFIG NO ES \"CLOCK\" NI \"CLOCK-M\". NO SE RECONOCE");
		return -5;
	}
}

void borrarDeRam(int nroFrame){
	pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
		void* frameVacio = malloc(tamanioDeCadaFrame);
		memcpy(memoriaPrincipal+(nroFrame*tamanioDeCadaFrame), frameVacio, tamanioDeCadaFrame);
	pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);
}

void reemplazarEnRam(void* page, int frameAReemplazar, int pid, int frameSwap, t_fila2N* pagVIEJA, int indice, t_fila2N* pagNUEVA){
	pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
		//si es clockM, averiguo si la pag fue modificada o no, para evitar un acceso a swap para actualizar la pagina en caso de no haberse modificado
		if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
			if(pagVIEJA->bitModificado == 1){ //es una pag modificada, hay que cargarla en swap
				void* aSwap = malloc(tamanioDeCadaFrame);
				memcpy(aSwap, memoriaPrincipal+(frameAReemplazar*tamanioDeCadaFrame), tamanioDeCadaFrame);

				pthread_mutex_lock(&mutexSwap);
					write_frame(pid, pagVIEJA->posicionSwap, aSwap);
				pthread_mutex_unlock(&mutexSwap);
			}
		}
		else{ //si NO es clockM, se actualiza igual en swap igual
			void* aSwap = malloc(tamanioDeCadaFrame);
			memcpy(aSwap, memoriaPrincipal+(frameAReemplazar*tamanioDeCadaFrame), tamanioDeCadaFrame);
			pthread_mutex_lock(&mutexSwap);
				write_frame(pid, pagVIEJA->posicionSwap, aSwap);
			pthread_mutex_unlock(&mutexSwap);
		}

		//reemplazo la page en ram
		memcpy(memoriaPrincipal+(frameAReemplazar*tamanioDeCadaFrame), page, tamanioDeCadaFrame);
	pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);

	//actualizo la tabla de paginas nueva
	pthread_mutex_lock(&mutexLista2N);
	pagNUEVA->bitPresencia = 1;
	pagNUEVA->bitDeUso = 1;
	pagNUEVA->bitModificado = 0;
	pagNUEVA->nroFrame = frameAReemplazar;
	pthread_mutex_unlock(&mutexLista2N);

	cambiarBitDePresencia(pagVIEJA->nroPagina);

	if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
		pthread_mutex_lock(&mutexLista2N);
		pagVIEJA->bitModificado = 0;
		pthread_mutex_unlock(&mutexLista2N);
	}

	//actualizo la tabla de frames
	t_filaFrame* marco;
	pthread_mutex_lock(&mutexListaFrames);
	for(int i=0; i<list_size(tablaDeFrames); i++){
		marco = list_get(tablaDeFrames, i);
		if(marco->nroFrame == frameAReemplazar){
			marco->estado = OCUPADO;
			marco->idProceso = pid;
			marco->nroPagina = pagNUEVA->nroPagina;
			//memcpy(memoriaPrincipal+(marco->nroFrame*tamanioDeCadaFrame),page,tamanioDeCadaFrame);
			break;
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
}











