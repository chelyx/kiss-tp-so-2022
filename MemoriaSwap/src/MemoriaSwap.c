#include "headers/MemoriaSwap.h"

int main(int argc, char** argv) {
	signal(SIGINT, cerrarMemoria);

	iniciarConfigYLog(argv[1]);
	iniciarSemaforos();
	iniciarListas();
	iniciarEstructuras();

	log_debug(loggerMemoria,"[INIT - MEMORIA] FINALIZADO. COMIENZA MULTIHILO");

	iniciarModuloMultihilo();
}

void iniciarConfigYLog(char* config) {
	loggerMemoria = log_create(PATH_LOG_MEMORIA, "MEMORIA", 1, LOG_LEVEL_DEBUG);
	loggerSwap = log_create(PATH_LOG_SWAP, "SWAP", 1, LOG_LEVEL_DEBUG);

	char* pathConfig = string_new();
	string_append(&pathConfig, "../cfg/");
	string_append(&pathConfig, config);
	string_append(&pathConfig, ".config");
	t_config* archivoConfig = config_create(pathConfig);

	if (archivoConfig == NULL){
		printf("////////////no se puede leer el archivo config porque estas ejecutando con las lineas comentadas");
		exit(EXIT_FAILURE);
	}
	extraerDatosConfig(archivoConfig);
	config_destroy(archivoConfig);
	free(pathConfig);

	log_debug(loggerMemoria,"[FIN - CONFIG Y LOG] CREADOS EXITOSAMENTE");
}

void extraerDatosConfig(t_config* archivoConfig) {
	configMemoriaSwap.algoritmoReemplazo = string_new();
	configMemoriaSwap.pathSwap = string_new();
	configMemoriaSwap.puertoEscucha = string_new();
	string_append(&configMemoriaSwap.algoritmoReemplazo, config_get_string_value(archivoConfig, "ALGORITMO_REEMPLAZO"));
	string_append(&configMemoriaSwap.pathSwap,config_get_string_value(archivoConfig, "PATH_SWAP"));
	string_append(&configMemoriaSwap.puertoEscucha, config_get_string_value(archivoConfig, "PUERTO_ESCUCHA"));

	configMemoriaSwap.tamanioMemoria = config_get_int_value(archivoConfig, "TAM_MEMORIA");
	configMemoriaSwap.tamanioPagina  = config_get_int_value(archivoConfig, "TAM_PAGINA");
	configMemoriaSwap.entradasPorTabla = config_get_int_value(archivoConfig, "ENTRADAS_POR_TABLA");
	configMemoriaSwap.retardoMemoria   = config_get_int_value(archivoConfig, "RETARDO_MEMORIA");
	configMemoriaSwap.marcosPorProceso = config_get_int_value(archivoConfig, "MARCOS_POR_PROCESO");
	configMemoriaSwap.retardoSwap = config_get_int_value(archivoConfig, "RETARDO_SWAP");
}


void iniciarSemaforos(void){
	pthread_mutex_init(&mutexSwap, NULL);
	pthread_mutex_init(&mutexIdTabla1N, NULL);
	pthread_mutex_init(&mutexIdTabla2N, NULL);
	pthread_mutex_init(&mutexLista1N, NULL);
	pthread_mutex_init(&mutexLista2N, NULL);
	pthread_mutex_init(&mutexListaFrames, NULL);
	pthread_mutex_init(&mutexVoidMemoriaPrincipal, NULL);
	pthread_mutex_init(&mutexAlgoritmoReemplazo, NULL);
	pthread_mutex_init(&mutexListaPCB, NULL);
}

void iniciarListas(void){
	tablaDeFrames      = list_create();
	listaTodasTablas1N = list_create();
	listaTodasTablas2N = list_create();
	listaPCB = list_create();
}

void iniciarModuloMultihilo(void){
	conexion = iniciarServidor(configMemoriaSwap.puertoEscucha);
	struct sockaddr_in dir_cliente;
	socklen_t tam_direccion = sizeof(struct sockaddr_in);
	pthread_t hiloConexionCPU;
	pthread_t hiloConexionKernel;

	//se conecta cpu
	int socketAceptadoCPU = 0;
	socketAceptadoCPU = accept(conexion, (void*)&dir_cliente, &tam_direccion);

	t_paquete paqueteCPU;
	recibirMensaje(socketAceptadoCPU, &paqueteCPU);
	if(paqueteCPU.header.cliente == CPU){
		log_debug(loggerMemoria,"[HANSHAKE] se conecto CPU");
		pthread_create(&hiloConexionCPU, NULL, (void*) conexionCPU, (void*)socketAceptadoCPU);
	}

	//se conecta kernel
	int socketAceptadoKernel= 0;
	socketAceptadoKernel= accept(conexion, (void*)&dir_cliente, &tam_direccion);

	t_paquete paqueteKernel;
	recibirMensaje(socketAceptadoKernel, &paqueteKernel);
	if(paqueteKernel.header.cliente == KERNEL){
		log_debug(loggerMemoria,"[HANDSHAKE] Se conecto KERNEL");
		pthread_create(&hiloConexionKernel, NULL, (void*) conexionKernel, (void*)socketAceptadoKernel);
	}

	pthread_join(hiloConexionCPU, NULL);
	pthread_join(hiloConexionKernel, NULL);

}

void cerrarMemoria(void){
	printf("-----------------------------------------");
	printf("Cerrando memoria");
	printf("-----------------------------------------");
	free(configMemoriaSwap.algoritmoReemplazo);
	free(configMemoriaSwap.pathSwap);

	pthread_mutex_destroy(&mutexSwap);
	pthread_mutex_destroy(&mutexIdTabla1N);
	pthread_mutex_destroy(&mutexIdTabla2N);
	pthread_mutex_destroy(&mutexLista1N);
	pthread_mutex_destroy(&mutexLista2N);
	pthread_mutex_destroy(&mutexListaFrames);
	pthread_mutex_destroy(&mutexVoidMemoriaPrincipal);
	pthread_mutex_destroy(&mutexAlgoritmoReemplazo);

	free(memoriaPrincipal);

	list_destroy_and_destroy_elements(tablaDeFrames,(void*)freeFrame);
	list_destroy_and_destroy_elements(listaTodasTablas2N, (void*)free2N);
	list_destroy_and_destroy_elements(listaTodasTablas1N, (void*)free1N);

	log_destroy(loggerMemoria);
	log_destroy(loggerSwap);

	exit(EXIT_SUCCESS);
}

void freeFrame(t_filaFrame* frame){
	//free(frame->frameVoid);
	free(frame);
}

void free2N(t_tabla2N* tabla2N){
	list_destroy_and_destroy_elements(tabla2N->tablaPaginas2N, free);
	free(tabla2N);
}

void free1N(t_tabla1N* tabla1N){
	list_destroy_and_destroy_elements(tabla1N->tablaPaginas1N, free);
	free(tabla1N);
}












