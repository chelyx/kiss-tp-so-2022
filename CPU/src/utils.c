#include "headers/utils.h"

void initCPU(char* config) {
	loggerCPU = log_create(PATH_LOG_CPU, "CPU", 1, LOG_LEVEL_DEBUG);

	char* pathConfig = string_new();
	string_append(&pathConfig, "../cfg/");
	string_append(&pathConfig, config);
	string_append(&pathConfig, ".config");
	t_config* archivoConfig = config_create(pathConfig);
	configCPU = extraerDatosConfig(archivoConfig);

	recibir_config_memoria();
	iniciar_TLB(configCPU);
	config_destroy(archivoConfig);
	free(pathConfig);
	sem_init(&sem_exec, 0, 0);
}

t_configCPU extraerDatosConfig(t_config* archivoConfig) {
	t_configCPU config;
	config.reemplazoTLB = string_new();
	config.ipMemoria = string_new();
	config.puertoMemoria = string_new();
	config.puertoEscuchaDispatch = string_new();
	config.puertoEscuchaInterrupt = string_new();

	config.entradasTLB = config_get_int_value(archivoConfig, "ENTRADAS_TLB");
	string_append(&config.reemplazoTLB, config_get_string_value(archivoConfig, "REEMPLAZO_TLB"));
	config.retardoNOOP = config_get_int_value(archivoConfig, "RETARDO_NOOP");
	string_append(&config.ipMemoria, config_get_string_value(archivoConfig, "IP_MEMORIA"));
	string_append(&config.puertoMemoria,config_get_string_value(archivoConfig, "PUERTO_MEMORIA"));
	string_append(&config.puertoEscuchaDispatch,config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_DISPATCH"));
	string_append(&config.puertoEscuchaInterrupt,config_get_string_value(archivoConfig, "PUERTO_ESCUCHA_INTERRUPT"));

	return config;
}

void recibir_config_memoria(){
	log_debug(loggerCPU,"Buscando configuracion inicial de memoria");
	socketMemoria = iniciarCliente(configCPU.ipMemoria, configCPU.puertoMemoria);

	enviarMensaje(socketMemoria, CPU, NULL, 0, HANDSHAKE_INICIAL);
	log_debug(loggerCPU,"ENVIE HANDSHAKE A MEMORIA");


	MSJ_INT* mensaje = malloc(sizeof(MSJ_INT));
	mensaje->numero = 1;

	enviarMensaje(socketMemoria, CPU, mensaje, sizeof(MSJ_INT), CONFIG_DIR_LOG_A_FISICA);
	free(mensaje);
	log_debug(loggerCPU,"Esperando mensaje de memoria para config inicial");

	t_paquete paquete;
	recibirMensaje(socketMemoria, &paquete);

	MSJ_MEMORIA_CPU_INIT* infoDeMemoria = malloc(sizeof(MSJ_MEMORIA_CPU_INIT));
	infoDeMemoria = paquete.mensaje;

	log_debug(loggerCPU,"Recibida informacion de memoria tamanio pagina=%i cantidadEntradasPorTabla=%i", infoDeMemoria->tamanioPagina, infoDeMemoria->cantEntradasPorTabla);

	configCPU.cantidadEntradasPorTabla = infoDeMemoria->cantEntradasPorTabla;
	configCPU.tamanioPagina = infoDeMemoria->tamanioPagina;

	free(infoDeMemoria);
}
