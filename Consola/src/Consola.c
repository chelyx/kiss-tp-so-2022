#include "headers/Consola.h"

int main(int argc, char** argv) {
	LOGGER = log_create(PATH_LOG_CONSOLA, "kernel", 1, LOG_LEVEL_DEBUG);
	log_info(LOGGER, "inicio Consola");
	if(argc < 3) {
		log_error(LOGGER, "no tiene los parametros");
		return EXIT_FAILURE;
	}

	t_config* archivoConfig = config_create(PATH_CONFIG_CONSOLA);
	if (archivoConfig == NULL){
		log_error(LOGGER, "archivoConfig es NULL");
		return EXIT_FAILURE;
	}
	char* ipKernel = string_new();
	char* puertoKernel = string_new();
	string_append(&puertoKernel, config_get_string_value(archivoConfig, "PUERTO_KERNEL"));
	string_append(&ipKernel, config_get_string_value(archivoConfig, "IP_KERNEL"));
	config_destroy(archivoConfig);

	/* argv == parametros:
	 * 		0: ./Consola
	 * 		1: tamaño que tendrá el proceso
	 * 		2: path al txt con las instrucciones del proceso
	*/
	log_debug(LOGGER,"%s", argv[1]);
	log_debug(LOGGER,"%s", argv[2]);

	char* mensaje = crearMensaje(argv[1], argv[2]);
	int tamanioMensaje = strlen(mensaje) + 1;  //lenght del mensaje + '/0'

	int socketKernel = iniciarCliente(ipKernel, puertoKernel);
	enviarMensaje(socketKernel, CONSOLA, mensaje, tamanioMensaje, INSTRUCCIONES);

	free(mensaje);
	log_info(LOGGER, "socket kernel %i", socketKernel);
	log_info(LOGGER,"Envie Instrucciones al kernel");

	t_pqte* paquete = recibirPaquete(socketKernel);
	if (paquete == NULL) {
		log_debug(LOGGER, "paqute null");
	} else {
		PCB* pcb = deserializoPCB(paquete->buffer);

		log_debug(LOGGER, "Me llego cod op %i", paquete->codigo_operacion);
		log_debug(LOGGER, "Me llego el exit del pcb id %i", pcb->id);
	}

	free(puertoKernel);
	free(ipKernel);
	close(socketKernel);
	log_destroy(LOGGER);

}

char* crearMensaje(char* tamanio, char* filename) {
	log_debug(LOGGER, "crearMensaje");

	char* path = string_new();
	string_append(&path, "../instrucciones/");
	string_append(&path, filename);
	FILE* input_file = fopen(path, "r");
	if (!input_file) {
		log_error(LOGGER, "no input_file");
		free(path);
		exit(EXIT_FAILURE);
	}
	char* mensaje = string_new();
	string_append(&mensaje, strcat(tamanio, "\n"));

	char* contents = NULL;
	size_t len = 0;
	while (getline(&contents, &len, input_file) != -1){
		string_append(&mensaje, contents);
		//log_debug(LOGGER, "agrega linea %s", mensaje);
	}
	fclose(input_file);
	free(contents);
	free(path);
	return mensaje;
}
