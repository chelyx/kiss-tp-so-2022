#include "headers/estructuras.h"

int crearTablasParaUnProceso(int pid, int tamanioProceso){
	int cantPaginasAUsar;
	if(tamanioProceso % configMemoriaSwap.tamanioPagina == 0){
		cantPaginasAUsar = tamanioProceso / configMemoriaSwap.tamanioPagina;
	} else {
		cantPaginasAUsar = (tamanioProceso / configMemoriaSwap.tamanioPagina) + 1;
	}
	int cantTablasSegundoNivel = calcularCantTablasSegundoNivelParaUnProceso(cantPaginasAUsar);

	t_tabla1N* tabla1N = malloc(sizeof(t_tabla1N));
	tabla1N->tablaPaginas1N = list_create();
	tabla1N->idProceso = pid;

	t_fila1N* fila1;
	t_tabla2N* tabla2N;
	t_fila2N* fila2;

	pthread_mutex_lock(&mutexIdTabla1N);
		idTabla1N++;
		tabla1N->idTabla = idTabla1N;
	pthread_mutex_unlock(&mutexIdTabla1N);
	int posicionswapcontador = 0;

	for(int k = 0; k < configMemoriaSwap.entradasPorTabla; k++){ //tabla 1N
		fila1 = malloc(sizeof(t_fila1N));
		fila1->nroPagina = k;

		if(k < cantTablasSegundoNivel){ //es una fila que tiene que tener asociada una tabla2N
			tabla2N = malloc(sizeof(t_tabla2N));
			tabla2N->idProceso = pid;
			tabla2N->tablaPaginas2N = list_create();
			fila1->nroTabla2N = k;

			for(int j = 0; j < configMemoriaSwap.entradasPorTabla; j++){ //tabla 2N
				fila2 = malloc(sizeof(t_fila2N));
				fila2->nroPagina =  (fila1->nroPagina * configMemoriaSwap.entradasPorTabla) + j;

				if((configMemoriaSwap.entradasPorTabla * k) + j < cantPaginasAUsar){ //PAG_A_USAR: es una entrada de 2N que se va a usar
					fila2->nroFrame = -1; //porque hasta que cpu no pida cargar la pagina, no le asigno frame
					fila2->bitPresencia = 0;
					fila2->bitDeUso = 0;
					fila2->bitModificado = 0;
					fila2->posicionSwap = posicionswapcontador;
					posicionswapcontador++;
					/*
					 * 	si entradasPorTabla = 3, entonces la posicionSwap va a ser 0, 1 y 2.
					 * 	No es lo mismo nroFrame que posicionSwap porque el nroFrame es desde 0 hasta cantFramesTotal
					 * 	y el posicionSwap va desde 0 hasya entradasPorTabla porque es 1 archivo por cada proceso.
					 */
					fila2->tipo = PAG_A_USAR;
				}
				else { //PAG_INUTIL: es una entrada de 2N que no se necesita porque el tamanio del proceso es mas chico
					fila2->nroFrame = -1;
					fila2->bitPresencia = -1;
					fila2->bitDeUso = -1;
					fila2->bitModificado = -1;
					fila2->posicionSwap = -1;
					fila2->posicionSwap = -1;
					fila2->tipo = PAG_INUTIL;
				}

				list_add(tabla2N->tablaPaginas2N, fila2);
			}

			pthread_mutex_lock(&mutexIdTabla2N);
				idTabla2N++;
				tabla2N->idTabla = idTabla2N;
			pthread_mutex_unlock(&mutexIdTabla2N);

			//agrega la tabla2N a la lista global de tablas2N en el indice igual a su id
			pthread_mutex_lock(&mutexLista2N);
				list_add(listaTodasTablas2N, tabla2N);
			pthread_mutex_unlock(&mutexLista2N);

			fila1->nroTabla2N = tabla2N->idTabla;
		}

		else { //es una fila que no tiene que tener asociada una tabla2N
			fila1->nroTabla2N = -1;
		}

		list_add(tabla1N->tablaPaginas1N, fila1);
	}



	pthread_mutex_lock(&mutexLista1N);
		list_add(listaTodasTablas1N, tabla1N);
	pthread_mutex_unlock(&mutexLista1N);

	pcb_mem* pcb = malloc(sizeof(pcb_mem));
	pcb->pid = pid;
	pcb->tamanioProceso = tamanioProceso;

	pthread_mutex_lock(&mutexListaPCB);
	list_add(listaPCB, pcb);
	pthread_mutex_unlock(&mutexListaPCB);
	return cantPaginasAUsar;
}

void iniciarEstructuras(void){
	memoriaPrincipal = malloc(configMemoriaSwap.tamanioMemoria);

	if (memoriaPrincipal == NULL) {
		log_error(loggerMemoria,"[MEMORIA PRINCIPAL] NO SE PUDO ALOCAR");
		exit(EXIT_FAILURE);
	}

	idTabla1N = -1;
	idTabla2N = -1;
	posicionClock = 0;

	tamanioDeCadaFrame = configMemoriaSwap.tamanioPagina;
	cantidadFramesTotal = (int)(configMemoriaSwap.tamanioMemoria / tamanioDeCadaFrame);

	//es lo mismo que el grado de multiprogramacion del kernel
	cantidadProcesosMaximosEnRam = (int)(cantidadFramesTotal / configMemoriaSwap.marcosPorProceso);

	iniciarFrames();
	iniciarCarpetaSwap();
}

void iniciarCarpetaSwap(){
	mkdir(configMemoriaSwap.pathSwap, S_IRWXU);
}

void iniciarFrames(void){

	for(int i = 0; i < cantidadFramesTotal; i++){
		t_filaFrame* frame;
		frame = malloc(sizeof(t_filaFrame));

		frame->estado = NO_OCUPADO;
		frame->idProceso = -1; //no tiene proceso asignado, por eso es negativo.
		frame->nroFrame = i;
		frame->nroPagina = -1; //no tiene nro de pagina asignada por eso es -1

		memcpy(memoriaPrincipal+(i*tamanioDeCadaFrame), string_repeat('*',tamanioDeCadaFrame), tamanioDeCadaFrame);

		list_add(tablaDeFrames, frame);
	}
}

void liberarFrame_Tabla(int nroFrame){
	t_filaFrame* frame;
	pthread_mutex_lock(&mutexListaFrames);
	for(int i=0; i<list_size(tablaDeFrames); i++){
		frame = list_get(tablaDeFrames, i);
		if(frame->nroFrame == nroFrame){
			frame->estado = NO_OCUPADO;
			frame->idProceso = -1;
			frame->nroPagina = -1;
			memcpy(memoriaPrincipal+(frame->nroFrame*tamanioDeCadaFrame),string_repeat('*',tamanioDeCadaFrame),tamanioDeCadaFrame);
		}
	}
	pthread_mutex_unlock(&mutexListaFrames);
}


