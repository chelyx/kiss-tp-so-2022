#include "psicoLibrary.h"

int iniciarCliente(char* ip, char* puerto) {  //para cliente de la cpu y de la memoria
	struct addrinfo hints;
	struct addrinfo *server_info;
	struct addrinfo *p;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(ip, puerto, &hints, &server_info);
	int socket_cliente = 0;
	for(p=server_info; p!=NULL; p=p->ai_next){
		if((socket_cliente=socket(p->ai_family, p->ai_socktype, p->ai_protocol))==-1){
			continue;
		}
		if(connect(socket_cliente, p->ai_addr, p->ai_addrlen) == -1){
			printf("\n\nno se pudo conectar el cliente al servidor\n\n");
			continue;
		}
		break;
	}
	if(p == NULL){
		printf("\n\nno se pudo conectar p == NULL\n\n");
		freeaddrinfo(server_info);
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(server_info);
	return socket_cliente;
}

void enviarMensaje(int socket, t_enviadoPor unModulo, void* mensaje, int tamanioMensaje, t_tipoMensaje tipoMensaje){
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->header.tipoMensaje = tipoMensaje;
	paquete->header.cliente = unModulo;
	uint32_t r = 0;
	if(tamanioMensaje<=0 || mensaje==NULL){
		paquete->header.tamanioMensaje = sizeof(uint32_t);
		paquete->mensaje = &r;
	} else {
		paquete->header.tamanioMensaje = tamanioMensaje;
		paquete->mensaje = mensaje;
	}
	enviarPaquete(socket, paquete);
	free(paquete);
}

void enviarPaquete(int socket, t_paquete* paquete) {
	uint32_t cantAEnviar = sizeof(t_infoMensaje) + paquete->header.tamanioMensaje;
	void* datos = malloc(cantAEnviar);
	memcpy(datos, &(paquete->header), sizeof(t_infoMensaje));

	if (paquete->header.tamanioMensaje > 0){
		memcpy(datos + sizeof(t_infoMensaje), (paquete->mensaje), paquete->header.tamanioMensaje);
	}

	uint32_t enviado = 0; //bytes enviados
	uint32_t totalEnviado = 0;

	do {
		enviado = send(socket, datos + totalEnviado, cantAEnviar - totalEnviado, MSG_NOSIGNAL);
		totalEnviado += enviado;
		if(enviado==-1){
			break;
		}
	} while (totalEnviado != cantAEnviar);

	free(datos);
}
/*
void enviarInterruptCPU(int socket, uint32_t pcbId, t_tipoMensaje tipoMensaje){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t);
	void* stream = malloc(buffer->size);
	memcpy(stream , &pcbId, sizeof(uint32_t));
	buffer->stream= stream;

	crearPaquete(buffer, tipoMensaje, socket);
}
*/
void serializarPCB(int socket, PCB* pcb, t_tipoMensaje tipoMensaje) {
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = sizeof(uint32_t)*5
				+ sizeof(double)*3
				+ strlen(pcb->instrucciones) + 1;

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &pcb->id, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->tamanio, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->programCounter, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->tablaPag, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &pcb->estimacion_actual, sizeof(double));
	offset += sizeof(double);
	memcpy(stream + offset, &pcb->real_anterior, sizeof(double));
	offset += sizeof(double);
	memcpy(stream + offset, &pcb->ejecutados_total, sizeof(double));
	offset += sizeof(double);

	//primero agregamos el largo del char*
	memcpy(stream + offset, &pcb->ins_length, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, pcb->instrucciones, strlen(pcb->instrucciones) + 1);

	buffer->stream= stream;

	crearPaquete(buffer, tipoMensaje, socket);
}

void crearPaquete(t_buffer* buffer, t_tipoMensaje op, int unSocket) {
	t_pqte* paquete = malloc(sizeof(t_pqte));
	paquete->codigo_operacion = (uint8_t) op;
	paquete->buffer = buffer;

	void* a_enviar = malloc(buffer->size + sizeof(uint8_t) + sizeof(uint32_t));
	int offset = 0;

	memcpy(a_enviar + offset, &(paquete->codigo_operacion), sizeof(uint8_t));
	offset += sizeof(uint8_t);
	memcpy(a_enviar + offset, &(paquete->buffer->size), sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(a_enviar + offset, paquete->buffer->stream, paquete->buffer->size);

	send(unSocket, a_enviar, buffer->size + sizeof(uint8_t) + sizeof(uint32_t), 0);

	free(a_enviar);
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

t_pqte* recibirPaquete(int socket) {
	t_pqte* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));

	// Primero recibimos el codigo de operacion
	int rec = recv(socket, &(paquete->codigo_operacion), sizeof(uint8_t), MSG_WAITALL);
	if(rec <= 0){
		return NULL;
	}

	// Después ya podemos recibir el buffer. Primero su tamaño seguido del contenido
	recv(socket, &(paquete->buffer->size), sizeof(uint32_t), MSG_WAITALL);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	recv(socket, paquete->buffer->stream, paquete->buffer->size, MSG_WAITALL);

	return paquete;
}

PCB* deserializoPCB(t_buffer* buffer){
	PCB* pcb = malloc(sizeof(PCB));

	void* stream = buffer->stream;

	// Deserializamos los campos que tenemos en el buffer
	memcpy(&(pcb->id), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(pcb->tamanio), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(pcb->programCounter), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(pcb->tablaPag), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	memcpy(&(pcb->estimacion_actual), stream, sizeof(double));
	stream += sizeof(double);
	memcpy(&(pcb->real_anterior), stream, sizeof(double));
	stream += sizeof(double);
	memcpy(&(pcb->ejecutados_total), stream, sizeof(double));
	stream += sizeof(double);

	// char*
	memcpy(&(pcb->ins_length), stream, sizeof(uint32_t));
	stream += sizeof(uint32_t);
	pcb->instrucciones = malloc(pcb->ins_length);
	memcpy(pcb->instrucciones, stream, pcb->ins_length);
	return pcb;
}

void recibirMensaje(int socket, t_paquete * paquete) {

	paquete->mensaje = NULL;
	int resul = recibirDatos(&(paquete->header), socket, sizeof(t_infoMensaje));

	if (resul > 0 && paquete->header.tamanioMensaje > 0) {
		paquete->mensaje = malloc(paquete->header.tamanioMensaje);
		resul = recibirDatos(paquete->mensaje, socket, paquete->header.tamanioMensaje);
	}
}

int recibirDatos(void* paquete, int socket, uint32_t cantARecibir) {
	void* datos = malloc(cantARecibir);
	int recibido = 0;
	int totalRecibido = 0;

	do {
		recibido = recv(socket, datos + totalRecibido, cantARecibir - totalRecibido, 0);
		totalRecibido += recibido;
	} while (totalRecibido != cantARecibir && recibido > 0);

	memcpy(paquete, datos, cantARecibir);
	free(datos);
	return recibido;
}

int iniciarServidor(char* puerto) {
	int socketServidor = 0;
	int valor = 0;
	const int enable = 1;

	struct addrinfo hints;
	struct addrinfo *servinfo;
	struct addrinfo *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL,puerto, &hints, &servinfo);

	p=servinfo;
	while(p != NULL){
		socketServidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(socketServidor == -1){
			p = p->ai_next;
			continue;
		}

		if (setsockopt(socketServidor, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0){
		    printf("////////////////////////////////////////////////////////////////////////////setsockopt(SO_REUSEADDR) failed");
			printf("////////////////////////////////////////////////////////////////////////////setsockopt(SO_REUSEADDR) failed");
			printf("////////////////////////////////////////////////////////////////////////////setsockopt(SO_REUSEADDR) failed");
		}

		valor = bind(socketServidor, p->ai_addr, p->ai_addrlen);
		if(valor == -1){
			close(socketServidor);
			p = p->ai_next;
			continue;
		}
		if(valor != -1){
			break;
		}
	}

	valor = listen(socketServidor, SOMAXCONN);
	if(valor < 0){
		perror("error con listen");
		return EXIT_FAILURE;
	}

	freeaddrinfo(servinfo);
	return socketServidor;
}

t_list* leerPseudocodigo(char* mensaje){
	t_list* listaIns = list_create();
	char** lineas = string_split(mensaje, "\n");
	int i = 1; // lees desde la segunda linea porq la primera es el tamaño del proceso
    while (lineas[i] != NULL){
		char** array = string_split(lineas[i], " ");
		instruccionConsola* nuevaIns = malloc(sizeof(instruccionConsola));
		if (strcmp(array[0], "NO_OP") == 0) {
			int cant = atoi(array[1]) - 1;
			for (int i = 0; i < cant; i++) {
				instruccionConsola* no_op = malloc(sizeof(instruccionConsola));
				no_op->identificador = NO_OP;
				no_op->parametro1 = -1;
				no_op->parametro2 = -1;
				list_add(listaIns, no_op);
			}
			nuevaIns->identificador = NO_OP;
			nuevaIns->parametro1 = -1;
			nuevaIns->parametro2 = -1;
		} else if (strcmp(array[0], "I/O") == 0) {
			nuevaIns->identificador = IO;
			nuevaIns->parametro1 = atoi(array[1]);
			nuevaIns->parametro2 = -1;
		}else if (strcmp(array[0], "READ") == 0) {
			nuevaIns->identificador = READ;
			nuevaIns->parametro1 = atoi(array[1]);
			nuevaIns->parametro2 = -1;
		}else if (strcmp(array[0], "WRITE") == 0) {
			nuevaIns->identificador = WRITE;
			nuevaIns->parametro1 = atoi(array[1]);
			nuevaIns->parametro2 = atoi(array[2]);
		}else if (strcmp(array[0], "COPY") == 0) {
			nuevaIns->identificador = COPY;
			nuevaIns->parametro1 = atoi(array[1]);
			nuevaIns->parametro2 = atoi(array[2]);
		}else if (strcmp(array[0], "EXIT") == 0) {
			nuevaIns->identificador = EXIT;
			nuevaIns->parametro1 = -1;
			nuevaIns->parametro2 = -1;
		}
	list_add(listaIns, nuevaIns);
	free(array[0]);
	free(array[1]);
	free(array);
	free(lineas[i]);
	i++;
    }
    free(lineas);
    return listaIns;
}
