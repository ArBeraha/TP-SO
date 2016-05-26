/*
 * planificacion.c
 *
 *  Created on: 25/5/2016
 *      Author: utnso
 */
#include "nucleo.h"

/*  ----------INICIO PLANIFICACION ---------- */
int cantidadProcesos() {
	int cantidad;
	pthread_mutex_lock(&lockProccessList);
	cantidad = list_size(listaProcesos);
	// El unlock se hace dos o tres lineas despues de llamar a esta funcion
	return cantidad;
}
void planificacionFIFO() {
	if (!queue_is_empty(colaListos) && !queue_is_empty(colaCPU))
		ejecutarProceso((int) queue_pop(colaListos), (int) queue_pop(colaCPU));

	if (!queue_is_empty(colaSalida))
		destruirProceso((int) queue_pop(colaSalida));
}
void planificacionRR() {
	int i;
	for (i = 0; i < cantidadProcesos(); i++) { // Con cantidadProcesos() se evitan condiciones de carrera.
		t_proceso* proceso = list_get(listaProcesos, i);
		pthread_mutex_unlock(&lockProccessList); // El lock se hace en cantidadProcesos()
		if (proceso->estado == EXEC) {
			if (terminoQuantum(proceso))
				expulsarProceso(proceso);
		}
	}
	planificacionFIFO();
}
void planificarProcesos() {
	switch (algoritmo) {
	// Procesos especificos
	case RR:
		planificacionRR();
		break;
	case FIFO:
		planificacionFIFO();
		break;
	}

	//Planificar IO
	dictionary_iterator(tablaIO, (void*) planificarIO);
}
void planificarIO(char* io_id, t_IO* io) {
	if (io->estado == INACTIVE) {
		io->estado = ACTIVE;
		t_bloqueo* info = malloc(sizeof(t_bloqueo));
		info->IO = io;
		info->PID = (int) queue_pop(io->cola);
		pthread_create(&hiloBloqueos, &detachedAttr, (void*) bloqueo, info);
	}
}
bool terminoQuantum(t_proceso* proceso) {
	return (!(proceso->PCB->PC % config.quantum)); // Si el PC es divisible por QUANTUM quiere decir que hizo QUANTUM ciclos
}