#ifndef PETICIONES_H_
#define PETICIONES_H_

#include "MemoriaSwap.h"

void pasarAReady(int pid, int tamanioProceso, int socketAceptado);
void suspender(int pid);
void pasarAExit(int pid);
void configurarDireccionesCPU(int socketAceptado);
void primerPasoTraduccionDireccion(int idTabla1Nivel, int pagina, int socketAceptado);
void segundoPasoTraduccionDireccion(int idTabla2doNivel, int pagina, int socketAceptado);
void accesoMemoriaRead(t_direccionFisica* dirFisica, int pid, int socketAceptado);
void accesoMemoriaWrite(t_direccionFisica* dirFisica, int valorAEscribir, int pid, int socketAceptado);
void accesoMemoriaCopy(t_direccionFisica* dirFisicaDestino, t_direccionFisica* dirFisicaOrigen, int pid, int socketAceptado);
void conexionCPU(void* socketConexion);
void conexionKernel(void* socketConexion);

#endif
