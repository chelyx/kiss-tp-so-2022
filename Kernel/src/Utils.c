#include "headers/Utils.h"

void initKernel(char* configName){
	iniciarConfigYLog(configName);

	//Variable que asigna IDs a los nuevos procesos
	ID_PROXIMO= 0;

	// listas
	LISTA_NEW = list_create();
	LISTA_READY = list_create();
	LISTA_EXEC = list_create();
	LISTA_BLOCKED = list_create();
	LISTA_BLOCKED_SUSPENDED = list_create();
	LISTA_READY_SUSPENDED = list_create();
	LISTA_SOCKETS = list_create();
	LISTA_EXIT = list_create();
	COLA_BLOQUEO_IO = list_create();
	instrucciones_x_pcb = list_create();

	// mutex
	pthread_mutex_init(&mutex_creacion_ID, NULL);
	pthread_mutex_init(&mutex_lista_new, NULL);
	pthread_mutex_init(&mutex_lista_ready, NULL);
	pthread_mutex_init(&mutex_lista_exec, NULL);
	pthread_mutex_init(&mutex_lista_blocked, NULL);
	pthread_mutex_init(&mutex_lista_blocked_suspended, NULL);
	pthread_mutex_init(&mutex_lista_ready_suspended, NULL);
	pthread_mutex_init(&mutex_lista_cola_io, NULL);
	pthread_mutex_init(&mutex_lista_instrxpcb, NULL);


	// semaforos
	sem_init(&sem_ready, 0, 0);
	sem_init(&sem_bloqueo, 0, 0);
	sem_init(&sem_planif_largo_plazo, 0, 0);
	sem_init(&contador_multiprogramacion, 0, configKernel.gradoMultiprogramacion);
	sem_init(&sem_procesador, 0, 1);

	log_debug(loggerKernel,"Creo Sockets");
	socketMemoriaSwap = iniciarCliente(configKernel.ipMemoria, configKernel.puertoMemoria);
	enviarMensaje(socketMemoriaSwap, KERNEL, NULL, 0, HANDSHAKE_INICIAL);
	log_debug(loggerKernel,"Handshake");
	socketCpuInterrupt = iniciarCliente(configKernel.ipCPU, configKernel.puertoCPUInterrupt);
	socketCpuDispatch = iniciarCliente(configKernel.ipCPU, configKernel.puertoCPUDispatch);

}

void iniciarConfigYLog(char* config) {
	//estas lineas de log y config sirven para ejecutar desde la terminal
	loggerKernel = log_create(PATH_LOG_KERNEL, "kernel", 1, LOG_LEVEL_DEBUG);

	char* pathConfig = string_new();
	string_append(&pathConfig, "../cfg/");
	string_append(&pathConfig, config);
	string_append(&pathConfig, ".config");
	t_config* archivoConfig = config_create(pathConfig);

	//estas lineas de log y config sirven para debuggear en eclipse
	//loggerKernel = log_create("../Kernel/logKernel.log", "kernel", 1, LOG_LEVEL_DEBUG);
	//t_config* archivoConfig = config_create("../Kernel/configKernel.config");
	if (archivoConfig == NULL){
		log_error(loggerKernel,"no se puede leer el archivo config porque estas ejecutando con las lineas comentadas");
		exit(100);
	}
	configKernel = extraerDatosConfig(archivoConfig);
	config_destroy(archivoConfig);
	free(pathConfig);
}

t_configKernel extraerDatosConfig(t_config* archivoConfig) {
	t_configKernel configKernel;
	configKernel.ipCPU = string_new();
	configKernel.ipMemoria = string_new();
	configKernel.puertoMemoria = string_new();
	configKernel.algoritmo = string_new();
	configKernel.puertoCPUDispatch = string_new();
	configKernel.puertoCPUInterrupt = string_new();
	configKernel.puertoEscucha = string_new();

	string_append(&configKernel.ipMemoria, config_get_string_value(archivoConfig, "IP_MEMORIA"));
	string_append(&configKernel.puertoMemoria, config_get_string_value(archivoConfig, "PUERTO_MEMORIA"));
	string_append(&configKernel.ipCPU,config_get_string_value(archivoConfig, "IP_CPU"));
	string_append(&configKernel.puertoCPUDispatch,config_get_string_value(archivoConfig, "PUERTO_CPU_DISPATCH"));
	string_append(&configKernel.puertoCPUInterrupt,config_get_string_value(archivoConfig, "PUERTO_CPU_INTERRUPT"));
	string_append(&configKernel.puertoEscucha,config_get_string_value(archivoConfig, "PUERTO_ESCUCHA"));
	string_append(&configKernel.algoritmo, config_get_string_value(archivoConfig, "ALGORITMO_PLANIFICACION"));

	configKernel.estimacionInicial = config_get_int_value(archivoConfig, "ESTIMACION_INICIAL");
	configKernel.alfa = config_get_double_value(archivoConfig, "ALFA");
	configKernel.gradoMultiprogramacion   = config_get_int_value(archivoConfig, "GRADO_MULTIPROGRAMACION");
	configKernel.tiempoMaximoBloqueado = config_get_int_value(archivoConfig, "TIEMPO_MAXIMO_BLOQUEADO");
	return configKernel;
}

void enviarInterruptCPU() {
	char* mensajeACpuInterrupt = string_new(); //aca se guardaría lo que hay que mandarle a la cpu por interrupt
	string_append(&mensajeACpuInterrupt, "Interrumpite");

	int tamanioMensajeCpuInterrupt = strlen(mensajeACpuInterrupt) + 1;  //lenght del mensaje + '/0'
	enviarMensaje(socketCpuInterrupt, KERNEL, mensajeACpuInterrupt, tamanioMensajeCpuInterrupt,INTERRUPT_INTERRUPCION);

	log_debug(loggerKernel,"Envie el mensaje a la cpu interrupt\n");
	free(mensajeACpuInterrupt);
}


int tamanioPCB(PCB* pcb) {
	int length = strlen(pcb->instrucciones)+1;
	return sizeof(PCB)
			  + sizeof(char*)*length;
}

void pasar_a_new(PCB* pcb){
	pthread_mutex_lock(&mutex_lista_new);
	list_add(LISTA_NEW, pcb);
	pthread_mutex_unlock(&mutex_lista_new);

	log_debug(loggerKernel, "Paso a NEW el proceso %d", pcb->id);
}

void pasar_a_ready(PCB* pcb) {
	pthread_mutex_lock(&mutex_lista_ready);
	list_add(LISTA_READY, pcb);
	pthread_mutex_unlock(&mutex_lista_ready);

	log_debug(loggerKernel, "Paso a READY el proceso %d", pcb->id);
}

void pasar_a_exec(PCB* pcb){
	pthread_mutex_lock(&mutex_lista_exec);
	list_add(LISTA_EXEC, pcb);
	pthread_mutex_unlock(&mutex_lista_exec);

	log_debug(loggerKernel, "Paso a EXEC el proceso %d", pcb->id);
}

void pasar_a_block(PCB* pcb) {
	pthread_mutex_lock(&mutex_lista_blocked);
	list_add(LISTA_BLOCKED,pcb);
	pthread_mutex_unlock(&mutex_lista_blocked);

	log_debug(loggerKernel, "Paso a BLOCK el proceso %d", pcb->id);
}

void pasar_a_susp_ready(PCB* pcb) {
	pthread_mutex_lock(&mutex_lista_ready_suspended);
	list_add(LISTA_READY_SUSPENDED, pcb);
	pthread_mutex_unlock(&mutex_lista_ready_suspended);

	log_debug(loggerKernel, "Paso a READY_SUSPENDED el proceso %d", pcb->id);
}

void pasar_a_susp_block(PCB* pcb) {
	pthread_mutex_lock(&mutex_lista_blocked_suspended);
	list_add(LISTA_BLOCKED_SUSPENDED,pcb);
	pthread_mutex_unlock(&mutex_lista_blocked_suspended);

	log_debug(loggerKernel, "Paso a BLOCKED_SUSPENDED el proceso %d", pcb->id);
}

void pasar_a_exit(PCB* pcb) {
	pthread_mutex_lock(&mutex_lista_exit);
	list_add(LISTA_EXIT, pcb);
	pthread_mutex_unlock(&mutex_lista_exit);

	log_debug(loggerKernel, "Paso a EXIT el proceso %d", pcb->id);
}

void cerrarKernel() {

	pthread_mutex_destroy(&mutex_creacion_ID);
	pthread_mutex_destroy(&mutex_lista_new);
	pthread_mutex_destroy(&mutex_lista_ready);
	pthread_mutex_destroy(&mutex_lista_exec);
	pthread_mutex_destroy(&mutex_lista_blocked);
	pthread_mutex_destroy(&mutex_lista_blocked_suspended);
	pthread_mutex_destroy(&mutex_lista_ready_suspended);

	sem_destroy(&sem_planif_largo_plazo);
	sem_destroy(&contador_multiprogramacion);
	sem_destroy(&sem_ready);
	sem_destroy(&sem_bloqueo);
	sem_destroy(&sem_procesador);

	void _PCB_destroyer(void* elemento) {
		free(((PCB*)elemento)->instrucciones);
		free(((PCB*)elemento));
	}

	void ins_consola(instruccionConsola* ic) {
		free(ic);
	}

	void _Ins_destroyer(instruccion_x_pcb* ixp) {
		list_destroy_and_destroy_elements(ixp->instrucciones, ins_consola);
		free(ixp);
	}

	list_destroy_and_destroy_elements(LISTA_NEW, _PCB_destroyer);
	list_destroy_and_destroy_elements(LISTA_READY, _PCB_destroyer);
	list_destroy_and_destroy_elements(LISTA_EXEC, _PCB_destroyer);
	list_destroy_and_destroy_elements(LISTA_BLOCKED, _PCB_destroyer);
	list_destroy_and_destroy_elements(LISTA_BLOCKED_SUSPENDED, _PCB_destroyer);
	list_destroy_and_destroy_elements(LISTA_READY_SUSPENDED, _PCB_destroyer);
	list_destroy_and_destroy_elements(COLA_BLOQUEO_IO, _PCB_destroyer);

	list_destroy_and_destroy_elements(instrucciones_x_pcb, _Ins_destroyer);

	list_destroy(LISTA_SOCKETS);

	log_destroy(loggerKernel);
}


