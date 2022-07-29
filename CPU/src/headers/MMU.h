/*
 * a.h
 *
 *  Created on: 30 may. 2022
 *      Author: utnso
 */

#ifndef A_H_
#define A_H_
#include <math.h>
#include "commons/string.h"
#include "stdlib.h"
#include <psicoLibrary.h>
#include "utils.h"

t_direccionFisica* traduccion_de_direccion(uint32_t id_tabla_de_paginas_1er_nivel, int direccion_logica,int tamanio_pagina,int cantidad_entradas_tabla);
int numero_de_pagina(int direccion_logica,int tamanio_pagina);
int entrada_tabla_primer_nivel(int numero_pagina, int cantidad_entradas_tabla);
int entrada_tabla_segundo_nivel(int numero_pagina, int cantidad_entradas_tabla);
int desplazamiento(int direccion_logica,int tamanio_pagina,int numero_pagina);
int segundo_llamado(int segundo_numero_pagina, int r_entrada_tabla_segundo_nivel);
int primer_llamado(uint32_t id_tabla_de_paginas_1er_nivel, int primer_numero_pagina, int r_entrada_tabla_primer_nivel);


#endif /* A_H_ */
