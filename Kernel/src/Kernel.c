#include "headers/Kernel.h"

int main(int argc, char** argv) {
	initKernel(argv[1]);

	iniciar_planif_largo_plazo();
	iniciar_planif_corto_plazo();
	iniciar_planif_io();
	iniciar_mensajes_cpu();
	consolaMultihilo();

	signal(SIGINT,  &cerrarKernel);
}

void consolaMultihilo() {
	log_debug(loggerKernel,"consolaMultihilo");
	int socketConsola = iniciarServidor(configKernel.puertoEscucha);
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);

	while(1) {
		log_debug(loggerKernel,"entro al while consolaMultihilo");
		pthread_t hilo_atender_consola;
		int* socketAceptadoConsola =  malloc(sizeof(int));
		*socketAceptadoConsola = accept(socketConsola, (void*)&dir_cliente, &tam_direccion);
		log_debug(loggerKernel,"Esperando mensaje de consola");

		t_paquete paqueteConsola;
		recibirMensaje(*socketAceptadoConsola, &paqueteConsola);
		log_info(loggerKernel,"RECIBI EL MENSAJE: %s\n", (char*)paqueteConsola.mensaje);

		args_pcb arguments;
		arguments.mensaje = strdup((char*)paqueteConsola.mensaje);
		arguments.socket_consola = *socketAceptadoConsola;

		free(socketAceptadoConsola);
		free(paqueteConsola.mensaje);
		log_info(loggerKernel,"socketAceptadoConsola %i", arguments.socket_consola);

		pthread_create(&hilo_atender_consola, NULL , (void*)crearPCB, (void*)&arguments);
		pthread_detach(hilo_atender_consola);

	}
	log_error(loggerKernel,"Muere hilo multiconsolas");
}

int enviarMensajeMemoriaSwapReady(MSJ_KERNEL_MEMORIA_READY* mensaje) {
	//conexion de cliente con MemoriaSwap, envia y responde

	int tamanioMensajeMemoriaSwap = sizeof(MSJ_KERNEL_MEMORIA_READY);

	enviarMensaje(socketMemoriaSwap, KERNEL, mensaje, tamanioMensajeMemoriaSwap, PASAR_A_READY);
	log_info(loggerKernel, "Envie el mensaje a la memoria swap\n");

	t_paquete paqueteMemoriaSwap;
	recibirMensaje(socketMemoriaSwap, &paqueteMemoriaSwap);
	MSJ_INT* msjINT = malloc(sizeof(MSJ_INT));
	msjINT = paqueteMemoriaSwap.mensaje;

	int rta = msjINT->numero;

	free(msjINT);
	//free(paqueteMemoriaSwap.mensaje);
	log_info(loggerKernel, "Recibi este mensaje: %i", rta);
	return rta;
}

void crearPCB(void* arguments) {
	args_pcb *args = (args_pcb*) arguments;

	list_add(LISTA_SOCKETS, (void*)(args->socket_consola));

	log_debug(loggerKernel, "creando PCB");
	char** mensajesplit = string_split(args->mensaje, "\n");

	PCB* nuevoProceso = malloc(sizeof(PCB));
	nuevoProceso->instrucciones = strdup(args->mensaje);
	nuevoProceso->ins_length = strlen(args->mensaje) +1;


	pthread_mutex_lock(&mutex_creacion_ID);
	nuevoProceso->id = ID_PROXIMO;
	ID_PROXIMO++;
	pthread_mutex_unlock(&mutex_creacion_ID);


	nuevoProceso->tamanio =  atoi(mensajesplit[0]);
	nuevoProceso->programCounter = 0;
	nuevoProceso->estimacion_actual = configKernel.estimacionInicial;
	nuevoProceso->real_anterior = 0.0;
	nuevoProceso->ejecutados_total=0.0;
	nuevoProceso->tablaPag = 0;

	t_list* ins = leerPseudocodigo(args->mensaje);
	int cant = list_size(ins);
	instruccion_x_pcb* nuevo = malloc(sizeof(instruccion_x_pcb) + sizeof(instruccionConsola)*cant);
	nuevo->id = nuevoProceso->id;
	nuevo->instrucciones = ins;

	pthread_mutex_lock(&mutex_lista_instrxpcb);
	list_add(instrucciones_x_pcb, nuevo);
	pthread_mutex_unlock(&mutex_lista_instrxpcb);

	pasar_a_new(nuevoProceso);

	sem_post(&sem_planif_largo_plazo);
	free(args->mensaje);
	free(mensajesplit);
}

void iniciar_planif_largo_plazo() {
	pthread_create(&planificador_largo_plazo, NULL, (void*)planifLargoPlazo, NULL);
	log_info(loggerKernel, "Inicio planificador largo plazo");
	pthread_detach(planificador_largo_plazo);
}

void iniciar_planif_corto_plazo() {
	pthread_create(&planificador_corto_plazo, NULL, (void*)planifCortoPlazo, NULL);
	log_info(loggerKernel, "Inicio planificador corto plazo %s", configKernel.algoritmo);
	pthread_detach(planificador_corto_plazo);
}

void iniciar_planif_io() {
	pthread_create(&planif_io, NULL, (void*)planifBloqueados, NULL);
	log_info(loggerKernel, "Inicio planificador bloqueados");
	pthread_detach(planif_io);
}

void iniciar_mensajes_cpu() {
	pthread_create(&mensajes_cpu, NULL, (void*)recibirMensajeCPU, NULL);
	log_info(loggerKernel, "Inicio mensajes cpu \n");
	pthread_detach(mensajes_cpu);
}

void planifLargoPlazo() {  //planificador largo plazo: de NEW/SUSPENDED_READY a READY
	log_warning(loggerKernel, "Hilo planif Largo Plazo");
	while(1){
		sem_wait(&sem_planif_largo_plazo);
		sem_wait(&contador_multiprogramacion);

		if(list_is_empty(LISTA_READY_SUSPENDED) == 0){
			log_info(loggerKernel, "Recuperando suspendidos");
			planifMedianoPlazoSuspReady();
		} else {
			log_info(loggerKernel, "Buscando news");
			// pasa de new a ready
			pthread_mutex_lock(&mutex_lista_new);
			PCB* pcb = (PCB*) list_remove(LISTA_NEW, 0);
			pthread_mutex_unlock(&mutex_lista_new);

			MSJ_KERNEL_MEMORIA_READY* mensajeAMemoria = malloc(sizeof(MSJ_KERNEL_MEMORIA_READY));
			mensajeAMemoria->pid = pcb->id;
			mensajeAMemoria->tamanioProceso= pcb->tamanio;
			pcb->tablaPag = enviarMensajeMemoriaSwapReady(mensajeAMemoria);
			free(mensajeAMemoria);

			pthread_mutex_lock(&mutex_lista_exec);
			int size = list_size(LISTA_EXEC);
			pthread_mutex_unlock(&mutex_lista_exec);
			pasar_a_ready(pcb);
			if (strcmp(configKernel.algoritmo, "SRT") == 0 && size > 0){
				enviarInterruptCPU();
				log_debug(loggerKernel, "envie interrupt por nuevo proceso");
			};
			sem_post(&sem_ready);
		}

	}
}


void planifMedianoPlazoSuspReady() {
	// SUSPENDED_READY -> READY
	pthread_mutex_lock(&mutex_lista_ready_suspended);
	PCB* pcb = (PCB*) list_remove(LISTA_READY_SUSPENDED, 0);
	pthread_mutex_unlock(&mutex_lista_ready_suspended);

	MSJ_KERNEL_MEMORIA_READY* mensajeAMemoria = malloc(sizeof(MSJ_KERNEL_MEMORIA_READY));
	mensajeAMemoria->pid = pcb->id;
	mensajeAMemoria->tamanioProceso= pcb->tamanio;

	enviarMensaje(socketMemoriaSwap, KERNEL, mensajeAMemoria, sizeof(MSJ_KERNEL_MEMORIA_READY), PASAR_A_READY);
	free(mensajeAMemoria);

	t_paquete paqueteMemoriaSwap;
	recibirMensaje(socketMemoriaSwap, &paqueteMemoriaSwap);
	char* mensaje = string_new();
	string_append(&mensaje, paqueteMemoriaSwap.mensaje);

	log_debug(loggerKernel, "PID %d CARGADO EN RAM, RESPUESTA MEMORIA: %s", pcb->id, mensaje);
	//pcb->tablaPag = enviarMensajeMemoriaSwapReady(mensajeAMemoria);
	free(mensaje);
	free(paqueteMemoriaSwap.mensaje);
	pasar_a_ready(pcb);
	if (strcmp(configKernel.algoritmo, "SRT") == 0){
		enviarInterruptCPU();
		log_error(loggerKernel, "envie interrupt por de-suspension");
	};
	sem_post(&sem_ready);
}

void planifCortoPlazo() { // READY -> EXEC
	log_warning(loggerKernel, "Hilo planif corto plazo");
	while(1) {
		sem_wait(&sem_ready);
		sem_wait(&sem_procesador);
		PCB* pcb;
		if(strcmp(configKernel.algoritmo, "SRT") == 0) {
			pcb = algoritmo_SRT();
		} else if (strcmp(configKernel.algoritmo, "FIFO") == 0){
			pcb = algoritmo_FIFO();
		} else {
			log_error(loggerKernel, "El algoritmo de planificacion ingresado no existe\n");
		}

		log_debug(loggerKernel, "PCB elegido: %i", pcb->id);

		pasar_a_exec(pcb);

		gettimeofday(&timeValBefore, NULL);

		serializarPCB(socketCpuDispatch, pcb, DISPATCH_PCB);
		log_debug(loggerKernel,"/////////////////////Envie el mensaje a la cpu dispatch\n");
	}
}

double media_exponencial(double realAnterior, double estimacionAnterior) {
	return configKernel.alfa * realAnterior + (1 - configKernel.alfa) * estimacionAnterior;
}

PCB* algoritmo_SRT() {
	void* _eleccion_SRT(PCB* elemento1, PCB* elemento2) {

		double rafagasRestantes1 = elemento1->estimacion_actual - elemento1->ejecutados_total;
		double rafagasRestantes2 = elemento2->estimacion_actual - elemento2->ejecutados_total;


		if(rafagasRestantes1 <= rafagasRestantes2) {
			return ((PCB*)elemento1);
		} else {
			return ((PCB*)elemento2);
		}
	}

	pthread_mutex_lock(&mutex_lista_ready);
	PCB* pcb = (PCB*) list_get_minimum(LISTA_READY, (void*)_eleccion_SRT);
	pthread_mutex_unlock(&mutex_lista_ready);

	bool _criterio_remocion_lista(void* elemento) {
		return (((PCB*)elemento)->id == pcb->id);
	}

	pthread_mutex_lock(&mutex_lista_ready);
	list_remove_by_condition(LISTA_READY, _criterio_remocion_lista);
	pthread_mutex_unlock(&mutex_lista_ready); 

	return pcb;
}


PCB* algoritmo_FIFO() {
	pthread_mutex_lock(&mutex_lista_ready);
	PCB* pcb = list_remove(LISTA_READY, 0);
	pthread_mutex_unlock(&mutex_lista_ready);
	return pcb;
}

void recibirMensajeCPU() {
	log_warning(loggerKernel, "Hilo mensaje cpu");

	while(1) {
		t_pqte* paquete = recibirPaquete(socketCpuDispatch);
		if(paquete == NULL) {
			continue;
		}
		PCB* pcb = deserializoPCB(paquete->buffer);

		bool _criterio_remocion_lista(void* elemento) {
			return (((PCB*)elemento)->id == pcb->id);
		}
		// guardar tiempo que estuvo en EXEC
		gettimeofday(&timeValAfter, NULL);
		int realEjecutado = (timeValAfter.tv_sec - timeValBefore.tv_sec)*1000 + (timeValAfter.tv_usec - timeValBefore.tv_usec)/1000;
		//log_warning(loggerKernel, "tiempo en exec %i", realEjecutado);
		switch(paquete->codigo_operacion) {
			case BLOCK_PCB: {
				pcb->real_anterior = pcb->ejecutados_total + realEjecutado;
				pcb->ejecutados_total = 0;
				pcb->estimacion_actual = media_exponencial(pcb->real_anterior, pcb->estimacion_actual);
				pthread_t hilo_control_suspendido;
				pthread_create(&hilo_control_suspendido,NULL,(void*)hiloSuspendedor,pcb);
				pthread_detach(hilo_control_suspendido);
				break;
			}
			case EXIT_PCB: {
				log_debug(loggerKernel, "Me llego el finish del pcb %i", pcb->id);
				pthread_mutex_lock(&mutex_lista_exec);
				list_remove_by_condition(LISTA_EXEC, _criterio_remocion_lista);
				pthread_mutex_unlock(&mutex_lista_exec);
				sem_post(&sem_procesador);

				int socketPCB = (int) list_get(LISTA_SOCKETS, pcb->id);
				log_debug(loggerKernel,"Envio la respuesta a la consola socket %i, pcb->id %i, Tipo mensaje: %i ", socketPCB,
										pcb->id, EXIT_PCB);
				serializarPCB(socketPCB, pcb, EXIT_PCB);
				MSJ_INT* msj = malloc(sizeof(MSJ_INT));
				msj->numero = pcb->id;
				enviarMensaje(socketMemoriaSwap, KERNEL, msj, sizeof(MSJ_INT), PASAR_A_EXIT);
				free(msj);
				log_debug(loggerKernel,"Envio exit a memoria");
				pasar_a_exit(pcb);
				sem_post(&contador_multiprogramacion);
				break;
			}
			case INTERRUPT_INTERRUPCION: {
				pcb->ejecutados_total = pcb->ejecutados_total + realEjecutado;
				pthread_mutex_lock(&mutex_lista_exec);
				list_remove_by_condition(LISTA_EXEC, _criterio_remocion_lista);
				pthread_mutex_unlock(&mutex_lista_exec);
				pasar_a_ready(pcb);
				sem_post(&sem_ready);
				sem_post(&sem_procesador);
				break;
			}
		}

	   free(paquete->buffer->stream);
	   free(paquete->buffer);
	   free(paquete);
	}
}

void hiloSuspendedor(PCB* pcb){
	bool _criterio_igual_id(void* elemento) {
		return (((PCB*)elemento)->id == pcb->id);
	}

	//EXEC -> BLOCKED
	pthread_mutex_lock(&mutex_lista_exec);
	list_remove_by_condition(LISTA_EXEC, _criterio_igual_id);
	pthread_mutex_unlock(&mutex_lista_exec);

	sem_post(&sem_procesador);
	pasar_a_block(pcb);

	pthread_mutex_lock(&mutex_lista_cola_io);
	list_add(COLA_BLOQUEO_IO, pcb);
	pthread_mutex_unlock(&mutex_lista_cola_io);
	sem_post(&sem_bloqueo);

	sleep(configKernel.tiempoMaximoBloqueado/1000);
			
	if(list_any_satisfy(LISTA_BLOCKED,_criterio_igual_id)){
		// BLOCKED -> SUSPENDED_BLOCK
		pthread_mutex_lock(&mutex_lista_blocked);
		list_remove_by_condition(LISTA_BLOCKED,_criterio_igual_id);
		pthread_mutex_unlock(&mutex_lista_blocked);

		//CUANDO PASAS EL PROCESO A SUSPENDIDO, HAY QUE AVISARLE A LA MEMORIA PARA Q PASE LAS PAGINAS A DISCO
		MSJ_INT* msj = malloc(sizeof(MSJ_INT));
		msj->numero = pcb->id;
		enviarMensaje(socketMemoriaSwap, KERNEL, msj, sizeof(MSJ_INT), SUSPENDER);
		free(msj);
		pasar_a_susp_block(pcb);
		sem_post(&contador_multiprogramacion);
	}
}

void planifBloqueados() {
	log_warning(loggerKernel, "Hilo planifBloqueados");
	while (1) {
		sem_wait(&sem_bloqueo);

		pthread_mutex_lock(&mutex_lista_cola_io);
		PCB* pcb = list_remove(COLA_BLOQUEO_IO, 0);
		pthread_mutex_unlock(&mutex_lista_cola_io);

		bool _criterio_ins(void* elemento) {
			return (((instruccion_x_pcb*)elemento)->id == pcb->id);
		}

		pthread_mutex_lock(&mutex_lista_instrxpcb);
		instruccion_x_pcb* insxpcb = list_find(instrucciones_x_pcb,_criterio_ins);
		pthread_mutex_unlock(&mutex_lista_instrxpcb);

		int index = pcb->programCounter - 1; //en cpu le aumenta el pc al prox

		instruccionConsola* ins = list_get(insxpcb->instrucciones, index);
		uint32_t duracion = ins->parametro1 /1000;
		log_warning(loggerKernel, "pcb->programCounter %i", pcb->programCounter);
		log_warning(loggerKernel, "ins->parametro1 %i", ins->parametro1);
		log_warning(loggerKernel, "duracion %i", duracion);
		sleep(duracion);
		bool _criterio_igual_id(void* elemento) {
			return (((PCB*)elemento)->id == pcb->id);
		}

		if(list_any_satisfy(LISTA_BLOCKED,_criterio_igual_id)) {
			pthread_mutex_lock(&mutex_lista_blocked);
			list_remove_by_condition(LISTA_BLOCKED,_criterio_igual_id);
			pthread_mutex_unlock(&mutex_lista_blocked);

			pasar_a_ready(pcb);
			if (strcmp(configKernel.algoritmo, "SRT") == 0){
				enviarInterruptCPU();
				log_error(loggerKernel, "envie interrupt por fin de IO");
			};
			sem_post(&sem_ready);
		} else if (list_any_satisfy(LISTA_BLOCKED_SUSPENDED,_criterio_igual_id)) {
			pthread_mutex_lock(&mutex_lista_blocked_suspended);
			list_remove_by_condition(LISTA_BLOCKED_SUSPENDED,_criterio_igual_id);
			pthread_mutex_unlock(&mutex_lista_blocked_suspended);

			pasar_a_susp_ready(pcb);
			sem_post(&sem_planif_largo_plazo);
		}
	}

}
