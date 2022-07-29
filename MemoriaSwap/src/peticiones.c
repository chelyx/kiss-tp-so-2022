#include "headers/peticiones.h"

void conexionCPU(void* socketAceptadoVoid){
	int socketAceptado = (int)socketAceptadoVoid;
	t_paquete paquete;

	int pid;
	int pagina;
	int idTabla1Nivel;
	int idTabla2Nivel;
	int valorAEscribir;
	t_direccionFisica* dirFisicaOrigen;
	t_direccionFisica* dirFisicaDestino;
	t_direccionFisica* dirFisica;

	MSJ_MEMORIA_CPU_ACCESO_1ERPASO* infoMemoriaCpu1erPaso;
	MSJ_MEMORIA_CPU_ACCESO_2DOPASO* infoMemoriaCpu2doPaso ;
	MSJ_MEMORIA_CPU_WRITE* infoMemoriaCpuWrite;
	MSJ_MEMORIA_CPU_COPY* infoMemoriaCpuCopy;
	MSJ_MEMORIA_CPU_READ* infoMemoriaCpuRead;

	while(1){
		dirFisicaOrigen = malloc(sizeof(t_direccionFisica));
		dirFisicaDestino = malloc(sizeof(t_direccionFisica));
		dirFisica = malloc(sizeof(t_direccionFisica));
		infoMemoriaCpu1erPaso = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_1ERPASO));
		infoMemoriaCpu2doPaso = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_2DOPASO));
		infoMemoriaCpuWrite = malloc(sizeof(MSJ_MEMORIA_CPU_WRITE));
		infoMemoriaCpuCopy = malloc(sizeof(MSJ_MEMORIA_CPU_COPY));
		infoMemoriaCpuRead = malloc(sizeof(MSJ_MEMORIA_CPU_READ));

		recibirMensaje(socketAceptado, &paquete);

		switch(paquete.header.tipoMensaje) {
			case CONFIG_DIR_LOG_A_FISICA:
				configurarDireccionesCPU(socketAceptado);
				break;
			case TRADUCCION_DIR_PRIMER_PASO:
				infoMemoriaCpu1erPaso = paquete.mensaje;
				idTabla1Nivel = infoMemoriaCpu1erPaso->idTablaPrimerNivel;
				pagina = infoMemoriaCpu1erPaso->pagina;
				primerPasoTraduccionDireccion(idTabla1Nivel, pagina, socketAceptado);
				break;
			case TRADUCCION_DIR_SEGUNDO_PASO:
				infoMemoriaCpu2doPaso = paquete.mensaje;
				idTabla2Nivel = infoMemoriaCpu2doPaso->idTablaSegundoNivel;
				pagina = infoMemoriaCpu2doPaso->pagina;
				segundoPasoTraduccionDireccion(idTabla2Nivel, pagina, socketAceptado);
				break;
			case ACCESO_MEMORIA_READ:
				infoMemoriaCpuRead = paquete.mensaje;
				dirFisica->nroMarco = infoMemoriaCpuRead->nroMarco;
				dirFisica->desplazamiento = infoMemoriaCpuRead->desplazamiento;
				pid = infoMemoriaCpuRead->pid;
				accesoMemoriaRead(dirFisica, pid, socketAceptado);
				break;
			case ACCESO_MEMORIA_WRITE:
				infoMemoriaCpuWrite = paquete.mensaje;
				dirFisica->nroMarco = infoMemoriaCpuWrite->nroMarco;
				dirFisica->desplazamiento = infoMemoriaCpuWrite->desplazamiento;
				valorAEscribir = infoMemoriaCpuWrite->valorAEscribir;
				pid = infoMemoriaCpuWrite->pid;
				accesoMemoriaWrite(dirFisica, valorAEscribir, pid, socketAceptado);
				break;
			case ACCESO_MEMORIA_COPY:
				infoMemoriaCpuCopy = paquete.mensaje;
				dirFisicaDestino->nroMarco = infoMemoriaCpuCopy->nroMarcoDestino;
				dirFisicaDestino->desplazamiento = infoMemoriaCpuCopy->desplazamientoDestino;
				dirFisicaOrigen->nroMarco = infoMemoriaCpuCopy->nroMarcoOrigen;
				dirFisicaOrigen->desplazamiento = infoMemoriaCpuCopy->desplazamientoOrigen;
				pid = infoMemoriaCpuCopy->pid;
				accesoMemoriaCopy(dirFisicaDestino, dirFisicaOrigen, pid, socketAceptado);
				break;
			default:
				log_error(loggerMemoria, "[TIPO DE MENSAJE] NO SE RECONOCE");
				break;
		}


		free(dirFisicaOrigen);
		free(dirFisicaDestino);
		free(dirFisica);
		free(infoMemoriaCpu1erPaso);
		free(infoMemoriaCpu2doPaso);
		free(infoMemoriaCpuWrite);
		free(infoMemoriaCpuCopy);
		free(infoMemoriaCpuRead);

		}
}

void conexionKernel(void* socketAceptadoVoid){
	int socketAceptado = (int)socketAceptadoVoid;
	t_paquete paquete;

	int pid;
	int tamanioProceso;
	MSJ_KERNEL_MEMORIA_READY* infoKernelMemoriaReady;
	MSJ_INT* mensajeEntero;

	while(1){
		mensajeEntero = malloc(sizeof(MSJ_INT));
		infoKernelMemoriaReady = malloc(sizeof(MSJ_KERNEL_MEMORIA_READY));

		recibirMensaje(socketAceptado, &paquete);
		switch(paquete.header.tipoMensaje) {
			case PASAR_A_READY:
				infoKernelMemoriaReady = paquete.mensaje;
				pid = infoKernelMemoriaReady->pid;
				tamanioProceso = infoKernelMemoriaReady->tamanioProceso;
				pasarAReady(pid, tamanioProceso, socketAceptado);
				break;
			case SUSPENDER:
				mensajeEntero = paquete.mensaje;
				pid = mensajeEntero->numero;
				suspender(pid);
				break;
			case PASAR_A_EXIT:
				mensajeEntero = paquete.mensaje;
				pid = mensajeEntero->numero;
				pasarAExit(pid);
				break;
			default:
				log_error(loggerMemoria, "[TIPO DE MENSAJE] NO SE RECONOCE");
				break;
		}

		free(infoKernelMemoriaReady);
		free(mensajeEntero);
	}
}

void pasarAReady(int pid, int tamanioProceso, int socketAceptado){
	log_debug(loggerMemoria,"[INIT - PASAR_A_READY] PID: %d", pid);

	//verifico si las tablas de ese pid ya existen porque vuevle a ready de suspendido o bloqueado
	t_tabla1N* tablaPrimerN;
	pthread_mutex_lock(&mutexLista1N);
		for(int indice= 0; indice < list_size(listaTodasTablas1N); indice++){
			tablaPrimerN = list_get(listaTodasTablas1N, indice);
			if(tablaPrimerN->idProceso == pid){
				//le asigno los frames vacios
				pthread_mutex_lock(&mutexListaFrames);
					t_filaFrame* frameLibre;
					int cantFrameAsignado = 0;
					for(int j=0; j<list_size(tablaDeFrames) && cantFrameAsignado < configMemoriaSwap.marcosPorProceso; j++){
						frameLibre = list_get(tablaDeFrames, j);

						if(frameLibre->estado == NO_OCUPADO && frameLibre->idProceso == -1 && frameLibre->nroPagina == -1){
							//log_error(loggerMemoria, "frame-> %s", (char*)frameLibre->frameVoid);
							frameLibre->idProceso = pid;
							log_info(loggerMemoria, "asigne frame a pid #%d. frameLibre->idProceso=%d",
									j, ((t_filaFrame*)list_get(tablaDeFrames,j))->idProceso);
							cantFrameAsignado++;
						}
					}
				pthread_mutex_unlock(&mutexListaFrames);


				char* cadena = string_new();
				string_append(&cadena, "OK");
				enviarMensaje(socketAceptado, MEMORIA_SWAP, cadena, strlen("OK")+1, PASAR_A_READY);
				log_debug(loggerMemoria, "[FIN - DEVUELTO A READY]  PID: %d --> ya existen sus tablas, no es proceso nuevo", pid);
				pthread_mutex_unlock(&mutexLista1N);
				free(cadena);
				return;
			}
		}
	pthread_mutex_unlock(&mutexLista1N);


	//en caso de que no existan las tablas porque es un pid nuevo:
	log_debug(loggerMemoria, "[PASAR A READY]  PID: %d --> ES UN PROCESO NUEVO", pid);
    int cantPags2N = crearTablasParaUnProceso(pid, tamanioProceso);

    //creo las pags vacias para guardar en swap. no las guardo en ram por carga a demanda

    void* paginasASwap = malloc(cantPags2N * configMemoriaSwap.tamanioPagina);
    void* pag2N;
    for(int i=0; i<cantPags2N; i++){
    	pag2N = malloc(configMemoriaSwap.tamanioPagina);
    	memcpy(pag2N,string_repeat('*',configMemoriaSwap.tamanioPagina),configMemoriaSwap.tamanioPagina);
    	memcpy(paginasASwap+(i*configMemoriaSwap.tamanioPagina), pag2N, configMemoriaSwap.tamanioPagina);
    }

    //le asigno los frames vacios
    pthread_mutex_lock(&mutexListaFrames);
    	t_filaFrame* frameLibre;
    	int cantFrameAsignado = 0;
		for(int j=0; j<list_size(tablaDeFrames) && cantFrameAsignado < configMemoriaSwap.marcosPorProceso; j++){
			frameLibre = list_get(tablaDeFrames, j);

			if(frameLibre->estado == NO_OCUPADO && frameLibre->idProceso == -1 && frameLibre->nroPagina == -1){
				frameLibre->idProceso = pid;
				log_warning(loggerMemoria, "asigne frame a pid #%d. frameLibre->idProceso=%d",
						j, ((t_filaFrame*)list_get(tablaDeFrames,j))->idProceso);
				cantFrameAsignado++;
			}
		}
	pthread_mutex_unlock(&mutexListaFrames);

	pthread_mutex_lock(&mutexSwap);
		log_debug(loggerMemoria, "resultado write pid completo: %d",write_pid_completo(pid, cantPags2N * configMemoriaSwap.tamanioPagina, paginasASwap));
    pthread_mutex_unlock(&mutexSwap);

    usleep(configMemoriaSwap.retardoSwap * 1000);

    int nroTablad1N = buscarNroTabla1N(pid);
	MSJ_INT* msjINT = malloc(sizeof(MSJ_INT));
    msjINT->numero = nroTablad1N;
    enviarMensaje(socketAceptado, MEMORIA_SWAP, msjINT, sizeof(MSJ_INT), PASAR_A_READY);
    free(msjINT);
	log_debug(loggerMemoria,"[FIN - PASAR_A_READY] PID: %d NRO TABLA ENVIADO AL KERNEL", pid);
}

void suspender(int pid){
	log_debug(loggerMemoria,"[INIT - SUSPENDER] PID: %d", pid);

	t_tabla2N* tablaSegundoNivel;
	t_fila2N* pag2N;
	int cantPagASwapear;
	t_list* listaFramesALiberar = list_create();
	void* aSuspender;
	pthread_mutex_lock(&mutexLista2N);
		cantPagASwapear = 0;
		for(int i=0; i<list_size(listaTodasTablas2N); i++){
			tablaSegundoNivel = list_get(listaTodasTablas2N, i);
			if(tablaSegundoNivel->idProceso == pid){    //es una tabla2N de ese pid
				for(int j=0; j<list_size(tablaSegundoNivel->tablaPaginas2N); j++){ //para cada pag2N... etc
					pag2N = list_get(tablaSegundoNivel->tablaPaginas2N, j);
					if(pag2N->tipo == PAG_A_USAR && pag2N->bitPresencia == 1){
						pag2N->bitPresencia = 0;
						pag2N->bitDeUso = 0;
						pag2N->bitModificado = 0;

						log_debug(loggerMemoria, "[A SWAPPEAR] nroPagina %d", pag2N->nroPagina);

						pthread_mutex_lock(&mutexListaFrames);
						bool tieneNroPagina(void* frame) {
							return (((t_filaFrame*)frame)->nroPagina == pag2N->nroPagina);
						}
						t_filaFrame* frameEncontrado = list_find(tablaDeFrames, tieneNroPagina);
						pthread_mutex_unlock(&mutexListaFrames);

						if(frameEncontrado == NULL){
							log_error(loggerMemoria, "frameEncontrado es NULL");
						}
						else {
							aSuspender = malloc(tamanioDeCadaFrame); //lo inicializo con el maximo de memoria que podria suspender
							pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
							memcpy(aSuspender,
									memoriaPrincipal+(frameEncontrado->nroFrame*tamanioDeCadaFrame),
									tamanioDeCadaFrame);
							pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);

							pthread_mutex_lock(&mutexSwap);
							log_debug(loggerMemoria, "pos swap PAG2N %d", pag2N->posicionSwap);
							write_frame(pid, pag2N->posicionSwap, aSuspender);
							pthread_mutex_unlock(&mutexSwap);
							log_debug(loggerMemoria, "ENCONTRE EL FRAME PARA LA PAG2N %d", pag2N->nroPagina);

							list_add(listaFramesALiberar, frameEncontrado);
							pag2N->nroFrame = -1;
						}

						cantPagASwapear++;
					}
				}
			}
		}

	pthread_mutex_unlock(&mutexLista2N);

    for(int lpm=0; lpm<list_size(listaFramesALiberar); lpm++){
    	liberarFrame_Tabla(((t_filaFrame*)list_get(listaFramesALiberar, lpm))->nroFrame);
    }

    usleep(configMemoriaSwap.retardoSwap * 1000);

    log_debug(loggerMemoria,"[FIN - SUSPENDER] PID: %d" , pid);
}

void pasarAExit(int pid){
	log_debug(loggerMemoria,"[INIT - PASAR_A_EXIT] PID: %d", pid);
	t_tabla2N* tablaSegundoNivel;
	t_fila2N* pag2N;

	pthread_mutex_lock(&mutexLista2N);
		for(int i=0; i<list_size(listaTodasTablas2N); i++){
			tablaSegundoNivel = list_get(listaTodasTablas2N, i);
			if(tablaSegundoNivel->idProceso == pid){    //es una tabla2N de ese pid
				for(int j=0; j<list_size(tablaSegundoNivel->tablaPaginas2N); j++){ //para cada pag2N... etc
					pag2N = list_get(tablaSegundoNivel->tablaPaginas2N, j);
					if(pag2N->tipo == PAG_A_USAR){
						pag2N->bitPresencia = 0;
						pag2N->bitDeUso = 0;
						pag2N->bitModificado = 0;
						liberarFrame_Tabla(pag2N->nroFrame);
						pag2N->nroFrame = -1;
					}
				}
			}
		}
	pthread_mutex_unlock(&mutexLista2N);

	pthread_mutex_lock(&mutexSwap);
    	delete_in_swap(pid);
    pthread_mutex_unlock(&mutexSwap);

    usleep(configMemoriaSwap.retardoSwap * 1000);

	log_debug(loggerMemoria,"[FIN - PASAR_A_EXIT] PID: %d" , pid);
}

void configurarDireccionesCPU(int socketAceptado){
	//enviar ENTRADAS_POR_TABLA y TAM_PAGINA
	log_debug(loggerMemoria,"[INIT - CONFIG_DIR_LOG_A_FISICA]");

	MSJ_MEMORIA_CPU_INIT* infoAcpu = malloc(sizeof(MSJ_MEMORIA_CPU_INIT));
	infoAcpu->cantEntradasPorTabla = configMemoriaSwap.entradasPorTabla;
	infoAcpu->tamanioPagina = configMemoriaSwap.tamanioPagina;

	usleep(configMemoriaSwap.retardoMemoria * 1000);
	enviarMensaje(socketAceptado, MEMORIA_SWAP, infoAcpu, sizeof(MSJ_MEMORIA_CPU_INIT), CONFIG_DIR_LOG_A_FISICA);
	free(infoAcpu);

	log_debug(loggerMemoria,"[FIN - CONFIG_DIR_LOG_A_FISICA] INFO DE CANT ENTRADAS POR TABLA Y TAMANIO PAGINA ENVIADO A CPU");
}

void primerPasoTraduccionDireccion(int idTabla1Nivel, int pagina2N, int socketAceptado){
	//CPU PREGUNTA CUAL ES EL ID DE LA TABLA DE 2N DONDE ESTA LA PAG 2N EN ESA TABLA1N
	log_debug(loggerMemoria,"[INIT - TRADUCCION_DIR_PRIMER_PASO] ID_TABLA1N: %d NRO_PAGINA_2N: %d", idTabla1Nivel, pagina2N);

	pthread_mutex_lock(&mutexLista1N);
		t_tabla1N* tabla1Naux;
		for(int i = 0; i< list_size(listaTodasTablas1N); i++){
			tabla1Naux = list_get(listaTodasTablas1N, i);
			if(tabla1Naux->idTabla == idTabla1Nivel){
				pthread_mutex_unlock(&mutexLista1N);
				break;
			}
		}
	pthread_mutex_unlock(&mutexLista1N);
	//LA TABLA DE 1N ES LA QUE ESTA EN LA POSICION i

	if(tabla1Naux == NULL) {
		log_error(loggerMemoria,"tabla1Naux es NULL");
	}

	MSJ_INT* idTabla2NSolicitada = malloc(sizeof(MSJ_INT));

	idTabla2NSolicitada->numero = buscarIdTabla2NEnUnaLista(tabla1Naux->tablaPaginas1N, pagina2N);

	log_debug(loggerMemoria, "idTabla2NSolicitada->numero %i", idTabla2NSolicitada->numero);

	usleep(configMemoriaSwap.retardoMemoria * 1000);
	enviarMensaje(socketAceptado, MEMORIA_SWAP, idTabla2NSolicitada, sizeof(MSJ_INT), TRADUCCION_DIR_PRIMER_PASO);
	free(idTabla2NSolicitada);
	log_debug(loggerMemoria,"[FIN - TRADUCCION_DIR_PRIMER_PASO] ID TABLA 2DO NIVEL SOBRE PAGINA: %d DE TABLA 1ER NIVEL: %d ENVIADA A LA CPU",
				pagina2N, idTabla1Nivel);
}

void segundoPasoTraduccionDireccion(int idTabla2Nivel, int pagina, int socketAceptado){
	//CPU PREGUNTA CUAL ES EL FRAME DONDE ESTA LA PAGINA DE ESA TABLA DE 2N
	log_debug(loggerMemoria,"[INIT - TRADUCCION_DIR_SEGUNDO_PASO] ID_TABLA2N: %d NRO_PAGINA: %d",
				idTabla2Nivel, pagina);

	int frameBuscado;
	t_fila2N* pag2N;
	t_tabla2N* tabla2N;
	int indice;
	bool corte = false;
	//busco la pagina que piden
	pthread_mutex_lock(&mutexLista2N);
		for(int i=0; i<list_size(listaTodasTablas2N)&& !corte; i++){
			tabla2N = list_get(listaTodasTablas2N, i);
			if(tabla2N->idTabla == idTabla2Nivel){
				for(int j=0; j<list_size(tabla2N->tablaPaginas2N) && !corte; j++){
					pag2N = list_get(tabla2N->tablaPaginas2N, j);
					if(pag2N->nroPagina == pagina){
						log_debug(loggerMemoria, "BUSCO PAGINA %d", pag2N->nroPagina);
						indice = j;
						corte = true;
					}
				}
			}
		}
	pthread_mutex_unlock(&mutexLista2N);

	//analizo si esta en ram
	if(pag2N->bitPresencia == 1 && pag2N->nroFrame!=-1){ //la pag esta cargada en ram
		pthread_mutex_lock(&mutexLista2N);
			pag2N->bitDeUso = 1;
			if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
				pag2N->bitModificado = 0;
			}
		pthread_mutex_unlock(&mutexLista2N);
		frameBuscado = pag2N->nroFrame;
		log_debug(loggerMemoria,"[TRADUCCION_DIR_SEGUNDO_PASO] LA PAGINA ESTA EN RAM");
	}
	else{ //la pag no esta en ram. hay que acceder a swap.
		//1) verificar si hay frames disponibles para ese proceso
		void* aReemplazar = malloc(configMemoriaSwap.tamanioPagina);
		int frameAReemplazar = hayFrameLibreParaElPid(tabla2N->idProceso); //-1: no hay. !=-1:nroFrame a reemplazar

		if(frameAReemplazar != -1){
			log_debug(loggerMemoria,"[TRADUCCION_DIR_SEGUNDO_PASO] HAY FRAME LIBRE PARA EL PID %d", tabla2N->idProceso);
			//2) si hay 1 frame disponible: pedir a swap la pagina, actualizar la pag a reemplazar en swap y cargar la nueva

			bool criterio(void* elemento) {
				return (((t_filaFrame*)elemento)->nroFrame == frameAReemplazar);
			}
			pthread_mutex_lock(&mutexListaFrames);
			t_filaFrame* frameAReemplazarStruct = list_find(tablaDeFrames,criterio);
			pthread_mutex_unlock(&mutexListaFrames);


			//busco la pagina vieja que piden
			t_fila2N* pagVIEJA;
			t_tabla2N* tabla2Ntmp;
			bool corte2 = false;
			pthread_mutex_lock(&mutexLista2N);
				for(int i=0; i<list_size(listaTodasTablas2N)&& !corte2; i++){
					tabla2Ntmp = list_get(listaTodasTablas2N, i);
					for(int j=0; j<list_size(tabla2Ntmp->tablaPaginas2N) && !corte2; j++){
						pagVIEJA = list_get(tabla2Ntmp->tablaPaginas2N, j);
						if(pagVIEJA->nroPagina == frameAReemplazarStruct->nroPagina){
							log_debug(loggerMemoria, "PAGINA VIEJA ES %d", pagVIEJA->nroPagina);
							corte2 = true;
						}
					}
				}
			pthread_mutex_unlock(&mutexLista2N);


			void* loQueTraigoDeSwap = malloc(configMemoriaSwap.tamanioPagina);
			log_debug(loggerMemoria, "posicion swap %d", pag2N->posicionSwap);
			pthread_mutex_lock(&mutexSwap);
			memcpy(loQueTraigoDeSwap,
					string_substring((char*)read_in_swap(tabla2N->idProceso, pag2N->posicionSwap, 0), 0, configMemoriaSwap.tamanioPagina),
					configMemoriaSwap.tamanioPagina);
			pthread_mutex_unlock(&mutexSwap);

			if(string_is_empty((char*)loQueTraigoDeSwap)){
				memcpy(aReemplazar, string_repeat('*',tamanioDeCadaFrame), tamanioDeCadaFrame);
			}
			else{
				memcpy(aReemplazar, loQueTraigoDeSwap, tamanioDeCadaFrame);
			}
			usleep(configMemoriaSwap.retardoSwap * 1000);
			reemplazarEnRam(aReemplazar, frameAReemplazar, tabla2N->idProceso, pagVIEJA->posicionSwap, pagVIEJA, indice, pag2N);
			frameBuscado = frameAReemplazar;
		}
		else{
			log_debug(loggerMemoria,"[INIT - ALGORITMO DE REEMPLAZO]");
			//3) si no hay frames: arrancar algoritmo de reemplazo para buscar la victima
			//4) reemplazar victima por pagina nueva.
			frameAReemplazar = algoritmoDeReemplazo(tabla2N->idProceso);

			if(frameAReemplazar == -5){
				log_error(loggerMemoria, "[ALGORITMO DE REEMPLAZO] No se reconoció el nombre del algoritmo extraido de config");
				exit(EXIT_FAILURE);
			}

				//busco la pagina que piden para cambiar su bit de presencia

		bool criterioFrame(void* elemento) {
			return (((t_filaFrame*)elemento)->nroFrame == frameAReemplazar);
		}
		pthread_mutex_lock(&mutexListaFrames);
		t_filaFrame* frameAReemplazarStruct = list_find(tablaDeFrames,criterioFrame);
		pthread_mutex_unlock(&mutexListaFrames);
			
		log_debug(loggerMemoria, "frameAreemplazar %i", frameAReemplazarStruct->nroPagina);
		log_debug(loggerMemoria, "posciicon %d", pag2N->posicionSwap);
		log_debug(loggerMemoria, "datos pag nro: %d", pag2N->nroPagina);


		//busco la pagina vieja que piden
		t_fila2N* pagVIEJA;
		t_tabla2N* tabla2Ntmp;
		bool corte2 = false;
		pthread_mutex_lock(&mutexLista2N);
			for(int i=0; i<list_size(listaTodasTablas2N)&& !corte2; i++){
				tabla2Ntmp = list_get(listaTodasTablas2N, i);
				for(int j=0; j<list_size(tabla2Ntmp->tablaPaginas2N) && !corte2; j++){
					pagVIEJA = list_get(tabla2Ntmp->tablaPaginas2N, j);
					if(pagVIEJA->nroPagina == frameAReemplazarStruct->nroPagina){
						log_debug(loggerMemoria, "PAGINA VIEJA ES %d", pagVIEJA->nroPagina);
						corte2 = true;
					}
				}
			}
		pthread_mutex_unlock(&mutexLista2N);


		log_debug(loggerMemoria, "posicion swap %d", pag2N->posicionSwap);
			pthread_mutex_lock(&mutexSwap);
				memcpy(aReemplazar,
						string_substring((char*)read_in_swap(tabla2N->idProceso, pag2N->posicionSwap, 0), 0, configMemoriaSwap.tamanioPagina),
						tamanioDeCadaFrame);
			pthread_mutex_unlock(&mutexSwap);

			usleep(configMemoriaSwap.retardoSwap * 1000);
			reemplazarEnRam(aReemplazar, frameAReemplazar, tabla2N->idProceso, pag2N->posicionSwap, pagVIEJA, indice, pag2N);
			frameBuscado = frameAReemplazar;

			log_debug(loggerMemoria,"[FIN - ALGORITMO DE REEMPLAZO]");
		}
	}

	usleep(configMemoriaSwap.retardoMemoria * 1000);

	MSJ_INT* mensaje = malloc(sizeof(MSJ_INT));
	mensaje->numero = frameBuscado;

	enviarMensaje(socketAceptado, MEMORIA_SWAP, mensaje, sizeof(MSJ_INT), TRADUCCION_DIR_SEGUNDO_PASO);
	free(mensaje);
	log_debug(loggerMemoria,"[FIN - TRADUCCION_DIR_SEGUNDO_PASO] FRAME BUSCADO = %d ,DE LA PAGINA: %d DE TABLA 2DO NIVEL: %d ENVIADO A CPU",
				frameBuscado, pagina, idTabla2Nivel);
}

void accesoMemoriaRead(t_direccionFisica* dirFisica, int pid, int socketAceptado){
	log_debug(loggerMemoria,"[INIT - ACCESO_MEMORIA_READ] DIR_FISICA: %d%d",
				dirFisica->nroMarco, dirFisica->desplazamiento);

	int nroFrame = dirFisica->nroMarco;
	int offset = dirFisica->desplazamiento;
	void* aLeer = malloc(tamanioDeCadaFrame-offset);
	int valorLeido;
	MSJ_STRING* mensajeError;

	//valido que el offset sea valido
	if(offset>tamanioDeCadaFrame){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		mensajeError = malloc(sizeof(MSJ_STRING));
		string_append(&mensajeError->cadena, "ERROR_DESPLAZAMIENTO");
		enviarMensaje(socketAceptado, MEMORIA_SWAP, mensajeError, sizeof(MSJ_STRING), ACCESO_MEMORIA_READ);
		free(mensajeError);
		log_error(loggerMemoria,"[ACCESO_MEMORIA_READ] OFFSET MAYOR AL TAMANIO DEL FRAME.  DIR_FISICA: %d%d",
					dirFisica->nroMarco, dirFisica->desplazamiento);
		return;
	}

	//valido que el nro frame sea valido
	if(nroFrame>cantidadFramesTotal){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		mensajeError = malloc(sizeof(MSJ_STRING));
		string_append(&mensajeError->cadena, "ERROR_NRO_FRAME");
		enviarMensaje(socketAceptado, MEMORIA_SWAP, mensajeError, sizeof(MSJ_STRING), ACCESO_MEMORIA_READ);
		free(mensajeError);
		log_error(loggerMemoria,"[ACCESO_MEMORIA_READ] NRO DE FRAME INEXISTENTE.  DIR_FISICA: %d%d",
					dirFisica->nroMarco, dirFisica->desplazamiento);
		return;
	}

	pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
		memcpy(aLeer, memoriaPrincipal+(nroFrame*tamanioDeCadaFrame)+offset, tamanioDeCadaFrame-offset);
	pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);
	char** cosa2 = string_array_new();
	cosa2 = string_split((char*)aLeer, "*");
	char* leidoStringArray = string_new();
	int size =string_array_size(cosa2) -1;
	for(int i = 0; i < size; i++){
		string_append(&leidoStringArray, cosa2[i]);
	}
	valorLeido = atoi(leidoStringArray);

	log_debug(loggerMemoria, "Valor Leido: %s", leidoStringArray);

	usleep(configMemoriaSwap.retardoMemoria * 1000);

	//////////////////////////////////////////
	t_tabla2N* tabla2N;
	t_fila2N* pag2N;
	bool update = false;
	pthread_mutex_lock(&mutexLista2N);
		for(int i=0; i<list_size(listaTodasTablas2N) && !update; i++){
			tabla2N = list_get(listaTodasTablas2N, i);
			for(int j=0; j<list_size(tabla2N->tablaPaginas2N) && !update; j++){
				pag2N = list_get(tabla2N->tablaPaginas2N, j);
				if(pag2N->nroFrame == nroFrame){
					pag2N->bitDeUso = 1;
					update = true;
				}
			}
		}
	pthread_mutex_unlock(&mutexLista2N);

	////////////////////////////////////////////
	MSJ_INT* mensajeRead = malloc(sizeof(MSJ_INT));
	mensajeRead->numero = valorLeido;
	enviarMensaje(socketAceptado, MEMORIA_SWAP, mensajeRead, sizeof(MSJ_INT), ACCESO_MEMORIA_READ);
	free(leidoStringArray);
	free(cosa2);
	free(mensajeRead);

	log_debug(loggerMemoria,"[FIN - ACCESO_MEMORIA_READ] DIR_FISICA: frame%d offset%d",
				dirFisica->nroMarco, dirFisica->desplazamiento);
}

void accesoMemoriaWrite(t_direccionFisica* dirFisica, int valorAEscribir, int pid, int socketAceptado){
	log_debug(loggerMemoria,"[INIT - ACCESO_MEMORIA_WRITE] DIR_FISICA: nroMarco %d offset %d, VALOR: %d",
				dirFisica->nroMarco, dirFisica->desplazamiento, valorAEscribir);

	int nroFrame = dirFisica->nroMarco;
	int offset = dirFisica->desplazamiento;


	//valido que el offset sea valido
	if(offset>tamanioDeCadaFrame){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		char* mensajeError = string_new();
		string_append(&mensajeError, "ERROR_DESPLAZAMIENTO");
		enviarMensaje(socketAceptado, MEMORIA_SWAP, mensajeError, strlen(mensajeError)+1, ACCESO_MEMORIA_WRITE);
		free(mensajeError);
		log_error(loggerMemoria,"[ACCESO_MEMORIA_WRITE] OFFSET MAYOR AL TAMANIO DEL FRAME.  DIR_FISICA: %d %d, VALOR: %d",
					dirFisica->nroMarco, dirFisica->desplazamiento, valorAEscribir);
		return;
	}

	//valido que el nro frame sea valido
	if(nroFrame>cantidadFramesTotal){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		char* mensajeError = string_new();
		string_append(&mensajeError, "ERROR_NRO_FRAME");
		enviarMensaje(socketAceptado, MEMORIA_SWAP, mensajeError, strlen(mensajeError)+1, ACCESO_MEMORIA_WRITE);
		free(mensajeError);
		log_error(loggerMemoria,"[ACCESO_MEMORIA_WRITE] NRO DE FRAME INEXISTENTE.  DIR_FISICA: %d %d, VALOR: %d",
					dirFisica->nroMarco, dirFisica->desplazamiento, valorAEscribir);
		return;
	}

	pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
	memcpy(memoriaPrincipal+(nroFrame*tamanioDeCadaFrame)+offset,
			string_itoa(valorAEscribir),
			strlen(string_itoa(valorAEscribir)));
	pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);

	//busco la pagina que piden y actualizo el bit de modificado porque se hizo write
	if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
		t_tabla2N* tabla2N;
		t_fila2N* pag2N;
		bool update = false;
		pthread_mutex_lock(&mutexLista2N);
			for(int i=0; i<list_size(listaTodasTablas2N) && !update; i++){
				tabla2N = list_get(listaTodasTablas2N, i);
				for(int j=0; j<list_size(tabla2N->tablaPaginas2N) && !update; j++){
					pag2N = list_get(tabla2N->tablaPaginas2N, j);
					if(pag2N->nroFrame == nroFrame){
						pag2N->bitModificado = 1;
						pag2N->bitDeUso= 1;
						update = true;
					}
				}
			}
		pthread_mutex_unlock(&mutexLista2N);
	}
	else{
		t_tabla2N* tabla2N;
		t_fila2N* pag2N;
		bool update = false;
		pthread_mutex_lock(&mutexLista2N);
			for(int i=0; i<list_size(listaTodasTablas2N) && !update; i++){
				tabla2N = list_get(listaTodasTablas2N, i);
				for(int j=0; j<list_size(tabla2N->tablaPaginas2N) && !update; j++){
					pag2N = list_get(tabla2N->tablaPaginas2N, j);
					if(pag2N->nroFrame == nroFrame){
						pag2N->bitDeUso = 1;
						update = true;
					}
				}
			}
		pthread_mutex_unlock(&mutexLista2N);
	}

	usleep(configMemoriaSwap.retardoMemoria * 1000);

	char* cadena = string_new();
	string_append(&cadena, "OK");

	enviarMensaje(socketAceptado, MEMORIA_SWAP, cadena, strlen("OK")+1, ACCESO_MEMORIA_WRITE);

	free(cadena);
	log_debug(loggerMemoria,"[FIN - ACCESO_MEMORIA_WRITE] DIR_FISICA: %d %d, VALOR: %d",
				dirFisica->nroMarco, dirFisica->desplazamiento, valorAEscribir);
}

void accesoMemoriaCopy(t_direccionFisica* dirFisicaDestino, t_direccionFisica* dirFisicaOrigen, int pid, int socketAceptado){
	log_debug(loggerMemoria,"[INIT - ACCESO_MEMORIA_COPY] DIR_FISICA_DEST: %d %d, DIR_FISICA_ORIGEN: %d %d",
				dirFisicaDestino->nroMarco, dirFisicaDestino->desplazamiento,
				dirFisicaOrigen->nroMarco, dirFisicaOrigen->desplazamiento);

	int origen_nroFrame = dirFisicaOrigen->nroMarco;
	int origen_offset = dirFisicaOrigen->desplazamiento;
	int dest_nroFrame = dirFisicaDestino->nroMarco;
	int dest_offset = dirFisicaDestino->desplazamiento;

	//valido que los offset sean validos
	if(origen_offset>tamanioDeCadaFrame){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		enviarMensaje(socketAceptado, MEMORIA_SWAP, "ERROR_DESPLAZAMIENTO_ORIGEN",
				strlen("ERROR_DESPLAZAMIENTO_ORIGEN")+1, ACCESO_MEMORIA_COPY);

		log_error(loggerMemoria,"[ACCESO_MEMORIA_COPY] OFFSET_ORIGEN MAYOR AL TAMANIO DEL FRAME.  DIR_FISICA: %d%d",
					dirFisicaOrigen->nroMarco, dirFisicaOrigen->desplazamiento);
		return;
	}
	if(dest_offset>tamanioDeCadaFrame){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		enviarMensaje(socketAceptado, MEMORIA_SWAP, "ERROR_DESPLAZAMIENTO_DESTINO",
				strlen("ERROR_DESPLAZAMIENTO_DESTINO")+1, ACCESO_MEMORIA_COPY);

		log_error(loggerMemoria,"[ACCESO_MEMORIA_COPY] OFFSET_DESTINO MAYOR AL TAMANIO DEL FRAME.  DIR_FISICA: %d%d",
					dirFisicaDestino->nroMarco, dirFisicaDestino->desplazamiento);
		return;
	}

	//valido que los nros de frame sean validos
	if(origen_nroFrame>cantidadFramesTotal){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		enviarMensaje(socketAceptado, MEMORIA_SWAP, "ERROR_NRO_FRAME_ORIGEN", strlen("ERROR_NRO_FRAME_ORIGEN")+1, ACCESO_MEMORIA_COPY);

		log_error(loggerMemoria,"[ACCESO_MEMORIA_COPY] NRO DE FRAME ORIGEN INEXISTENTE.  DIR_FISICA: %d%d",
					dirFisicaOrigen->nroMarco, dirFisicaOrigen->desplazamiento);
		return;
	}
	if(dest_nroFrame>cantidadFramesTotal){
		usleep(configMemoriaSwap.retardoMemoria * 1000);
		enviarMensaje(socketAceptado, MEMORIA_SWAP, "ERROR_NRO_FRAME_DESTINO", strlen("ERROR_NRO_FRAME_DESTINO")+1, ACCESO_MEMORIA_COPY);

		log_error(loggerMemoria,"[ACCESO_MEMORIA_COPY] NRO DE FRAME DESTINO INEXISTENTE.  DIR_FISICA: %d%d",
					dirFisicaDestino->nroMarco, dirFisicaDestino->desplazamiento);
		return;
	}

	//int tamanioAEscribir = tamanioDeCadaFrame-origen_offset;

	//log_error(loggerMemoria, "memoria pre copy: %s", (char*)memoriaPrincipal);

	pthread_mutex_lock(&mutexVoidMemoriaPrincipal);
	memcpy(memoriaPrincipal+(dest_nroFrame*tamanioDeCadaFrame)+dest_offset,
			memoriaPrincipal+(origen_nroFrame*tamanioDeCadaFrame)+origen_offset,
			4);   //seteo 4 porque siempre es un entero de 4 bytes
	pthread_mutex_unlock(&mutexVoidMemoriaPrincipal);

	//log_error(loggerMemoria, "memoria post copy: %s", (char*)memoriaPrincipal);

	//busco la pagina que piden y actualizo el bit de modificado porque se hizo write
	if(string_equals_ignore_case(configMemoriaSwap.algoritmoReemplazo, "CLOCK-M")){
		t_tabla2N* tabla2N;
		t_fila2N* pag2N;
		pthread_mutex_lock(&mutexLista2N);
			for(int i=0; i<list_size(listaTodasTablas2N); i++){
				tabla2N = list_get(listaTodasTablas2N, i);
				for(int j=0; j<list_size(tabla2N->tablaPaginas2N); j++){
					pag2N = list_get(tabla2N->tablaPaginas2N, j);
					if(pag2N->nroFrame == dest_nroFrame){
						pag2N->bitModificado = 1;
						break;
					}
				}
			}
		pthread_mutex_unlock(&mutexLista2N);
	}

	usleep(configMemoriaSwap.retardoMemoria * 1000);

	char* cadena = string_new();
	string_append(&cadena, "OK");

	enviarMensaje(socketAceptado, MEMORIA_SWAP, cadena, strlen("OK")+1, ACCESO_MEMORIA_COPY);
	free(cadena);
	log_debug(loggerMemoria,"[FIN - ACCESO_MEMORIA_COPY] DIR_FISICA_DEST: %d %d, DIR_FISICA_ORIGEN: %d %d",
				dirFisicaDestino->nroMarco, dirFisicaDestino->desplazamiento,
				dirFisicaOrigen->nroMarco, dirFisicaOrigen->desplazamiento);
}






