#include "headers/MMU.h"


t_direccionFisica* traduccion_de_direccion(uint32_t id_tabla_de_paginas_1er_nivel, int direccion_logica,int tamanio_pagina,int cantidad_entradas_tabla){
	t_direccionFisica *direccion = malloc(sizeof(t_direccionFisica));
	int primer_numero_pagina = numero_de_pagina(direccion_logica,tamanio_pagina);
	int r_entrada_tabla_primer_nivel = entrada_tabla_primer_nivel(primer_numero_pagina, cantidad_entradas_tabla);
	int idTablaSegundoNivel = primer_llamado(id_tabla_de_paginas_1er_nivel, primer_numero_pagina, r_entrada_tabla_primer_nivel);
	int r_entrada_tabla_segundo_nivel = entrada_tabla_segundo_nivel(primer_numero_pagina, cantidad_entradas_tabla);

	direccion->nroMarco = segundo_llamado(primer_numero_pagina, idTablaSegundoNivel);
	direccion->desplazamiento = direccion_logica - (primer_numero_pagina * tamanio_pagina);
	log_debug(loggerCPU, "calcule el offset = %d", direccion->desplazamiento);
	return direccion;
}

int numero_de_pagina(int direccion_logica,int tamanio_pagina){
	int numero_de_pagina = (int)(direccion_logica / tamanio_pagina);
	return numero_de_pagina;
}

int entrada_tabla_primer_nivel(int numero_pagina, int cantidad_entradas_tabla){
	int entrada_tabla_primer_nivel = 0;
	if(numero_pagina != 0) { // ROMPE LA DIV POR 0
		entrada_tabla_primer_nivel =  (int)(cantidad_entradas_tabla / numero_pagina);
	}
	return entrada_tabla_primer_nivel;
}


int entrada_tabla_segundo_nivel(int numero_pagina, int cantidad_entradas_tabla){
	int r_entrada_tabla_segundo_nivel = numero_pagina % cantidad_entradas_tabla;
	return r_entrada_tabla_segundo_nivel;
}


int desplazamiento(int direccion_logica,int tamanio_pagina,int numero_pagina){
	int desplazamiento = direccion_logica - (numero_pagina * tamanio_pagina);
	return desplazamiento;
}

int primer_llamado(uint32_t id_tabla_de_paginas_1er_nivel, int primer_numero_pagina, int r_entrada_tabla_primer_nivel){
	MSJ_MEMORIA_CPU_ACCESO_1ERPASO *mensajeAMemoriaPrimerAcceso = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_1ERPASO));
	//mensajeAMemoriaPrimerAcceso->idTablaPrimerNivel = r_entrada_tabla_primer_nivel;
	mensajeAMemoriaPrimerAcceso->idTablaPrimerNivel = id_tabla_de_paginas_1er_nivel;
	mensajeAMemoriaPrimerAcceso->pagina = primer_numero_pagina;
	enviarMensaje(socketMemoria, CPU, mensajeAMemoriaPrimerAcceso, sizeof(MSJ_MEMORIA_CPU_ACCESO_1ERPASO), TRADUCCION_DIR_PRIMER_PASO);
	log_debug(loggerCPU, "Envie entrada primer nivel a memoria");

	t_paquete paqueteMemoria;
	recibirMensaje(socketMemoria, &paqueteMemoria);
	MSJ_INT* msj = malloc(sizeof(MSJ_INT));
	msj = paqueteMemoria.mensaje;
	int segundoNumeroDePagina = msj->numero;

	log_debug(loggerCPU, "Primer llamado traduccion devuelve nroPagina = %d", segundoNumeroDePagina);

	return segundoNumeroDePagina;
}

int segundo_llamado(int segundo_numero_pagina, int r_entrada_tabla_segundo_nivel){
	MSJ_MEMORIA_CPU_ACCESO_2DOPASO *mensajeAMemoriaSegundoAcceso = malloc(sizeof(MSJ_MEMORIA_CPU_ACCESO_2DOPASO));
	mensajeAMemoriaSegundoAcceso->idTablaSegundoNivel = r_entrada_tabla_segundo_nivel;
	mensajeAMemoriaSegundoAcceso->pagina = segundo_numero_pagina;
	enviarMensaje(socketMemoria, CPU, mensajeAMemoriaSegundoAcceso, sizeof(MSJ_MEMORIA_CPU_ACCESO_2DOPASO), TRADUCCION_DIR_SEGUNDO_PASO);

	log_debug(loggerCPU, "Envie entrada segundo nivel a memoria");

	t_paquete paqueteMemoria;
	recibirMensaje(socketMemoria, &paqueteMemoria);
	MSJ_INT* mensajeSegundoLlamado = malloc(sizeof(MSJ_INT));
	mensajeSegundoLlamado = paqueteMemoria.mensaje;
	int nroFrame = mensajeSegundoLlamado->numero;

	log_info(loggerCPU, "(segundo acceso)EL FRAME BUSCADO ES: %d", nroFrame);

	return nroFrame;
}
