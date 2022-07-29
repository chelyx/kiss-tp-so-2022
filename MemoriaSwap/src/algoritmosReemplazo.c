#include "headers/algoritmosReemplazo.h"

int reubicarClock(int pid){
	//tiene que retornar el numero del frame que tiene a la victima a reemplazar.

	/*
  		1. creo una variable global que sea por ej PUNTERO_CLOCK
		2. recorro toda la lista de frames desde la posicion en la que quedó el puntero, consultando si el frame de esa posicion tiene una pagina perteneciente al proceso que quiere cargar una nueva pagina
		   2.1 en caso positivo: evaluo lo del bit de uso (porque es reemplazo local)
				2.1.1 si bit de uso == 0 entonces reemplazo la pag de ese frame por la nueva e incremento el puntero y finaliza el algoritmo.
				2.1.2 si bit de uso == 1 entonces lo pongo en 0 e incremento el puntero
		   2.2 en caso negativo (es un frame que tiene una pagina de OTRO proceso), entonces lo dejo asi como esta e incremento el puntero.
		3. repito paso 2 hasta encontrar a la victima
	 */

	t_filaFrame* frame;
	int nroFrameQueTieneVictima = -1;
	int bitDeUsoTemp;
	pthread_mutex_lock(&mutexAlgoritmoReemplazo);
		if(posicionClock >= (cantidadFramesTotal-1)){ // se hace -1 xq arranca desde pos = 0
			posicionClock = 0;
		}
	pthread_mutex_unlock(&mutexAlgoritmoReemplazo);

	pthread_mutex_lock(&mutexListaFrames);
		while(nroFrameQueTieneVictima == -1){
			frame = list_get(tablaDeFrames, posicionClock);
			if(frame->idProceso == pid){
				bitDeUsoTemp = buscarBitDeUsoDePagina(frame->nroPagina);
				if(bitDeUsoTemp == 0){
					nroFrameQueTieneVictima = frame->nroFrame;
				}
				else { // bitDeUsoTemp == 1
					cambiarBitDeUsoDePagina(frame->nroPagina);
				}
			}
			incrementarPosicionClock();
		}
	pthread_mutex_unlock(&mutexListaFrames);

	return nroFrameQueTieneVictima;
}

int reubicarClockModificado(int pid){
	//tiene que retornar el numero del frame que tiene a la victima a reemplazar.

	int nroFrameQueTieneVictima = -1;
	pthread_mutex_lock(&mutexAlgoritmoReemplazo);
		if(posicionClock >= (cantidadFramesTotal-1)){ // se hace -1 xq arranca desde pos = 0
			posicionClock = 0;
		}
	pthread_mutex_unlock(&mutexAlgoritmoReemplazo);

	int vuelta = 1;  // 1, 2, 3 o 4.

	while(nroFrameQueTieneVictima == -1){
		if(vuelta == 1 || vuelta == 3){
			nroFrameQueTieneVictima = vuelta1y3(pid);
		}
		if(vuelta == 2 || vuelta == 4){
			nroFrameQueTieneVictima = vuelta2y4(pid);
		}

		if(nroFrameQueTieneVictima != -1){ //encontró el (0,0) o el (0,1) dependiendo la vuelta
			return nroFrameQueTieneVictima;
		}

		vuelta++;
	}

	//nunca va a llegar a este return pero sino el eclipse jode con que no puse return ;(
	return nroFrameQueTieneVictima;
}

void incrementarPosicionClock(void){
	pthread_mutex_lock(&mutexAlgoritmoReemplazo);
		if(posicionClock >= (cantidadFramesTotal-1)){
			posicionClock = 0;
		}
		else{
			posicionClock++;
		}
	pthread_mutex_unlock(&mutexAlgoritmoReemplazo);
}

int vuelta1y3(int pid){
	//PRIMERA y TERCERA VUELTA: busca el (0,0). NO MODIFICA EL BIT DE USO.
	t_filaFrame* frame;
	int bitDeUsoTemp;
	int bitModificadoTemp;
	pthread_mutex_lock(&mutexListaFrames);
	for(int i = 0; i < list_size(tablaDeFrames); i++){
		frame = list_get(tablaDeFrames, posicionClock);
		incrementarPosicionClock();
		if(frame->idProceso == pid){
			bitDeUsoTemp = buscarBitDeUsoDePagina(frame->nroPagina);
			bitModificadoTemp = buscarBitModificado(frame->nroPagina);
			if(bitDeUsoTemp == 0 && bitModificadoTemp == 0){
				//RETORNA EL NRO FRAME QUE TIENE A ESA VICTIMA
				pthread_mutex_unlock(&mutexListaFrames);
				return frame->nroFrame;
			}
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
	return -1;
}

int vuelta2y4(int pid){
	//SEGUNDA y CUARTA VUELTA: busca el (0,1). MODIFICA EL BIT DE USO.
	t_filaFrame* frame;
	int bitDeUsoTemp;
	int bitModificadoTemp;
	pthread_mutex_lock(&mutexListaFrames);
	for(int i = 0; i < list_size(tablaDeFrames); i++){
		frame = list_get(tablaDeFrames, posicionClock);
		incrementarPosicionClock();
		if(frame->idProceso == pid){
			bitDeUsoTemp = buscarBitDeUsoDePagina(frame->nroPagina);
			bitModificadoTemp = buscarBitModificado(frame->nroPagina);
			if(bitDeUsoTemp == 0 && bitModificadoTemp == 1){
				//RETORNA EL NRO FRAME QUE TIENE A ESA VICTIMA
				pthread_mutex_unlock(&mutexListaFrames);
				return frame->nroFrame;
			}
			else { // bitDeUsoTemp == 1
				cambiarBitDeUsoDePagina(frame->nroPagina);
			}
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
	return -1;
}






