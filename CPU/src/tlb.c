#include "headers/tlb.h"



/* Campos que vienen del archivo de configuracion
 * CANTIDAD_ENTRADAS_TLB : Num
 * ALGORITMO_REEMPLAZO_TLB: String
 */
void iniciar_TLB(){

	int cantidadEntradasTLB = configCPU.entradasTLB;

	if(cantidadEntradasTLB==0){
		TLBEnable = 0;
		return;
	}

	TLBEnable = 1;

	TLB = malloc(sizeof(tlb));

	TLB->size = cantidadEntradasTLB;
	TLB->entradas = list_create();
	TLB->algoritmo = configCPU.reemplazoTLB;

}

int tlbTieneEntradasLibres(){
	return TLB->size > TLB->entradas->elements_count;
}

//En este caso, la TLB tiene una o mas entradas libres
void llenar_TLB(int nroPagina,int nroFrame){
	entrada_tlb* entry = malloc(sizeof(entrada_tlb));
	entry->nroPagina = nroPagina;
	entry->nroFrame = nroFrame;
	list_add_in_index(TLB->entradas, 0, entry);

}



void actualizar_TLB(int nroPagina,int nroFrame){

	if(tlbTieneEntradasLibres()){
		llenar_TLB(nroPagina, nroFrame);
		return;
	}

	//REEMPLAZO DE PAGINA
	if(strcmp(TLB->algoritmo , "LRU")== 0){
		reemplazo_lru(nroPagina, nroFrame);
	} else {
		reemplazo_fifo(nroPagina, nroFrame);
	}

}

void reemplazo_fifo(int nroPagina, int nroFrame){
	entrada_tlb* nuevo = malloc(sizeof(entrada_tlb));
	nuevo->nroPagina = nroPagina;
	nuevo->nroFrame = nroFrame;
	entrada_tlb* viejo = list_remove(TLB->entradas, TLB->size-1);
	log_warning(loggerCPU, "Reemplaza pagina: %d por nueva pagina %d", viejo->nroPagina, nuevo->nroPagina);
	printf("Reemplaza pagina: %d por nueva pagina %d", viejo->nroPagina, nuevo->nroPagina);
	list_add_in_index(TLB->entradas, 0, nuevo);
	free(viejo);
}

void destruir_entrada(void* entry){
	entrada_tlb* tlbEntry = (entrada_tlb*) entry;
	free(tlbEntry);
}

void limpiar_entradas_TLB(){
	list_clean_and_destroy_elements(TLB->entradas, destruir_entrada);
}

void cerrar_TLB(){
	int i;
	entrada_tlb* actual;
	for(i=0; i< TLB->entradas->elements_count; i++){
		actual = list_get(TLB->entradas, i);
		list_remove_and_destroy_element(TLB->entradas, i, destruir_entrada);
	}
	free(TLB->algoritmo);
	free(TLB);
}

void reemplazo_lru(int nroPagina, int nroFrame){
	int i = list_size(TLB->entradas);
	i--;
	entrada_tlb* nuevaEntrada = malloc(sizeof(entrada_tlb));
	nuevaEntrada->nroPagina = nroPagina;
	nuevaEntrada->nroFrame = nroFrame;
	entrada_tlb* remplazada = list_remove(TLB->entradas, i);
	log_warning(loggerCPU, "Reemplaza pagina: %d por nueva pagina %d", remplazada->nroPagina, nuevaEntrada->nroPagina);
	printf("Reemplaza pagina: %d por nueva pagina %d", remplazada->nroPagina, nuevaEntrada->nroPagina);
	free(remplazada);
	list_add_in_index(TLB->entradas, 0, nuevaEntrada);

}




int buscar_en_TLB(int nroPagina){ //devuelve numero de frame, si esta en la tlb, devuelve -1 si no esta en la tlb
	entrada_tlb* actual;
	for(int i=0; i< TLB->entradas->elements_count; i++){
		actual = list_get(TLB->entradas, i);
		if(actual->nroPagina == nroPagina){
			if(strcmp(TLB->algoritmo , "LRU")== 0){
				actual = list_remove(TLB->entradas,i);
				list_add_in_index(TLB->entradas,0,actual);
			}
			log_debug(loggerCPU, "TLB HIT: Pagina: %i, Frame: %i.\n", actual->nroPagina, actual->nroFrame);
			return actual->nroFrame;
		}
	}
	log_debug(loggerCPU, "TLB MISS.\n");
	return -1;
}


