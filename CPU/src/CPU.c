#include "headers/CPU.h"

int main(int argc, char** argv) {
	initCPU(argv[1]);

	interrupciones = false;
	iniciar_dispatch();
	iniciar_interrupt();
	recibir_config_memoria();


	signal(SIGINT,  &cerrarCPU);
}

void iniciar_dispatch() {
	pthread_create(&hiloDispatch, NULL, (void*)iniciarDispatch, NULL);
	pthread_detach(hiloDispatch);
}

void iniciar_interrupt() {
	pthread_create(&hiloInterrupt, NULL, (void*)iniciarInterrupt, NULL);
	pthread_join(hiloInterrupt, NULL);
}

void iniciarDispatch(){
	int conexionDispatch = iniciarServidor(configCPU.puertoEscuchaDispatch);
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	socketAceptadoDispatch = 0;
	socketAceptadoDispatch = accept(conexionDispatch, (void*)&dir_cliente, &tam_direccion);
	log_debug(loggerCPU,"[CPU] Puerto Dispatch Escuchando");
	while(1){
		t_pqte* paquete= recibirPaquete(socketAceptadoDispatch);
		if (paquete == NULL) {
			continue;
		}
		PCB* pcb = deserializoPCB(paquete->buffer);
		free(paquete->buffer->stream);
		free(paquete->buffer);
		free(paquete);
		t_list* instrucciones = leerPseudocodigo(pcb->instrucciones);
		log_debug(loggerCPU, "Leyo instrucciones cant %i", list_size(instrucciones));

		cicloInstruccion(instrucciones, pcb);

		list_destroy(instrucciones);
	}
}

void iniciarInterrupt(){
	int conexionInterrupt = iniciarServidor(configCPU.puertoEscuchaInterrupt);
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	socketAceptadoInterrupt = 0;
	socketAceptadoInterrupt = accept(conexionInterrupt, (void*)&dir_cliente, &tam_direccion);
	log_debug(loggerCPU,"[CPU] Puerto Interrupt Escuchando");
	while(1){
		t_paquete paquete;
		recibirMensaje(socketAceptadoInterrupt, &paquete);

		interrupciones = true;
	}
}

void cicloInstruccion(t_list* instrucciones, PCB* pcb) {
	// fetch
	uint32_t index = pcb->programCounter;
	instruccionConsola* insActual = list_get(instrucciones, index);
	pcb->programCounter += 1;
	log_info(loggerCPU,"insActual->identificador: %i", insActual->identificador);
	log_info(loggerCPU,"insActual->pc: %i", index);


	//decode y fetch operands
	t_direccionFisica* dirFisicaDestinoCopy;
	if(insActual->identificador == COPY) {
		log_debug(loggerCPU, "Buscar operandos en memoria");
		dirFisicaDestinoCopy = malloc(sizeof(t_direccionFisica));
		dirFisicaDestinoCopy = calcularDireccionFisica(pcb->tablaPag ,insActual->parametro2);
	}

	//execute
	log_debug(loggerCPU, "Ejecutando pcb->id %i", pcb->id);

	bool retornePCB = false;
	switch(insActual->identificador){
		case IO:
			serializarPCB(socketAceptadoDispatch, pcb, BLOCK_PCB);
			log_debug(loggerCPU,"Envie BLOCK al kernel por IO");
			limpiar_entradas_TLB();
			retornePCB = true;
			break;
		case NO_OP:
			log_debug(loggerCPU,"NO_OP");
			usleep(1000 * configCPU.retardoNOOP);
			break;
		case READ: //READ(dirección_lógica)
			log_debug(loggerCPU, "READ %i", insActual->parametro1);

			t_direccionFisica* dirFisicaRead = malloc(sizeof(t_direccionFisica));
			dirFisicaRead = calcularDireccionFisica(pcb->tablaPag, insActual->parametro1);

			MSJ_MEMORIA_CPU_READ* mensajeAMemoriaRead = malloc(sizeof(MSJ_MEMORIA_CPU_READ));

			mensajeAMemoriaRead->desplazamiento = dirFisicaRead->desplazamiento;
			mensajeAMemoriaRead->nroMarco = dirFisicaRead->nroMarco;
			mensajeAMemoriaRead->pid = pcb->id;
			enviarMensaje(socketMemoria, CPU, mensajeAMemoriaRead, sizeof(MSJ_MEMORIA_CPU_READ), ACCESO_MEMORIA_READ);
			log_debug(loggerCPU, "Envie direccion fisica a memoria swap: MARCO: %d, OFFSET: %d\n", mensajeAMemoriaRead->nroMarco, mensajeAMemoriaRead->desplazamiento);

			t_paquete paqueteMemoriaSwap;
			recibirMensaje(socketMemoria, &paqueteMemoriaSwap);
			MSJ_INT* mensajeRead = malloc(sizeof(MSJ_INT));
			mensajeRead = paqueteMemoriaSwap.mensaje;

			log_debug(loggerCPU, "Mensaje leido: %d", mensajeRead->numero);


			free(dirFisicaRead);
			free(mensajeAMemoriaRead);
			free(mensajeRead);
			break;
		case WRITE: //WRITE(dirección_lógica, valor)
			log_debug(loggerCPU, "WRITE %i %i", insActual->parametro1, insActual->parametro2);

			t_direccionFisica* dirFisicaWrite = malloc(sizeof(dirFisicaWrite));
			dirFisicaWrite = calcularDireccionFisica(pcb->tablaPag, insActual->parametro1);
			log_debug(loggerCPU, "dir fisica: nroMarco= %d offset=%d", dirFisicaWrite->nroMarco,dirFisicaWrite->desplazamiento);

			MSJ_MEMORIA_CPU_WRITE* mensajeAMemoriaWrite = malloc(sizeof(MSJ_MEMORIA_CPU_WRITE));
			mensajeAMemoriaWrite->desplazamiento = dirFisicaWrite->desplazamiento;
			mensajeAMemoriaWrite->nroMarco = dirFisicaWrite->nroMarco;
			mensajeAMemoriaWrite->valorAEscribir = insActual->parametro2;
			mensajeAMemoriaWrite->pid = pcb->id;

			log_debug(loggerCPU, "valorAEscrbir = %i", insActual->parametro2);

			enviarMensaje(socketMemoria, CPU, mensajeAMemoriaWrite, sizeof(MSJ_MEMORIA_CPU_WRITE), ACCESO_MEMORIA_WRITE);
			log_debug(loggerCPU, "Envie direccion fisica a memoria swap\n");

			t_paquete paqueteMemoriaSwapWrite;
			recibirMensaje(socketMemoria, &paqueteMemoriaSwapWrite);

			char* mensajeWrite = string_new();
			string_append(&mensajeWrite, paqueteMemoriaSwapWrite.mensaje);

			log_debug(loggerCPU, "Mensaje escrito: %s", mensajeWrite);

			free(mensajeWrite);
			free(dirFisicaWrite);
			free(mensajeAMemoriaWrite);
			break;
		case COPY: //COPY(dirección_lógica_destino, dirección_lógica_origen)
			log_debug(loggerCPU, "COPY %i %i", insActual->parametro1, insActual->parametro2);
			//funciona como un write: el parametro2 es el valor, lo tendrias que haber buscado en el fetchOperands
			t_direccionFisica* dirFisicaOrigenCopy = malloc(sizeof(t_direccionFisica));
			dirFisicaOrigenCopy = calcularDireccionFisica(pcb->tablaPag, insActual->parametro1);

			MSJ_MEMORIA_CPU_COPY* mensajeAMemoriaCopy = malloc(sizeof(MSJ_MEMORIA_CPU_COPY));

			mensajeAMemoriaCopy->nroMarcoOrigen = dirFisicaOrigenCopy->nroMarco;
			mensajeAMemoriaCopy->desplazamientoOrigen = dirFisicaOrigenCopy->desplazamiento;

			mensajeAMemoriaCopy->nroMarcoDestino = dirFisicaDestinoCopy->nroMarco;
			mensajeAMemoriaCopy->desplazamientoDestino = dirFisicaDestinoCopy->desplazamiento;
			mensajeAMemoriaCopy->pid = pcb->id;

			enviarMensaje(socketMemoria, CPU, mensajeAMemoriaCopy, sizeof(MSJ_MEMORIA_CPU_COPY), ACCESO_MEMORIA_COPY);
			log_debug(loggerCPU, "Envie direcciones fisicas a memoria swap");

			t_paquete paqueteMemoriaSwapCopy;
			recibirMensaje(socketMemoria, &paqueteMemoriaSwapCopy);

			char* mensajeCopy = string_new();
			string_append(&mensajeCopy, paqueteMemoriaSwapCopy.mensaje);

			log_debug(loggerCPU, "Mensaje copiado: %s", mensajeCopy);

			free(dirFisicaDestinoCopy);
			free(dirFisicaOrigenCopy);
			free(mensajeAMemoriaCopy);
			free(mensajeCopy);
			break;
		case EXIT:
			serializarPCB(socketAceptadoDispatch, pcb, EXIT_PCB);
			log_debug(loggerCPU,"Envie EXIT al kernel");
			retornePCB = true;
			limpiar_entradas_TLB();
			break;
	}
	//check interrupt
	if(!interrupciones && !retornePCB) {
		cicloInstruccion(instrucciones, pcb);
	} else if (interrupciones && !retornePCB) {
		// devuelvo pcb a kernel
		log_debug(loggerCPU,"Devuelvo pcb por interrupcion");
		serializarPCB(socketAceptadoDispatch, pcb, INTERRUPT_INTERRUPCION);
		interrupciones = false;
		limpiar_entradas_TLB();
	}
}

t_direccionFisica* calcularDireccionFisica(uint32_t id_tabla_de_paginas_1er_nivel,uint32_t direccionLogica) {
	// Direccion Logica / Tamaño de pagina = Numero de pagina
	int nroPagina = direccionLogica / configCPU.tamanioPagina;
	int nroMarco = buscar_en_TLB(nroPagina);

	t_direccionFisica* dirFisica = malloc(sizeof(t_direccionFisica));
	if(nroMarco != -1){ //CASO: LA PAGINA ESTA EN LA TLB
		// Direccion fisica = Numero de marco * tamaño de marco + offset
		//dirFisica = malloc(sizeof(t_direccionFisica));
		dirFisica->nroMarco = nroMarco;
		dirFisica->desplazamiento = direccionLogica % configCPU.tamanioPagina;
	} else { //CASO: LA PAGINA NO ESTA EN LA TLB, USA LA MMU
		dirFisica = traduccion_de_direccion(id_tabla_de_paginas_1er_nivel, direccionLogica, configCPU.tamanioPagina, configCPU.cantidadEntradasPorTabla);
		actualizar_TLB(nroPagina, dirFisica->nroMarco);
	}

	return dirFisica;
}

void cerrarCPU(){
	log_warning(loggerCPU, "cerrarCPU");
	log_destroy(loggerCPU);
}
