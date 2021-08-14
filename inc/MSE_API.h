/*=============================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/14
 * Archivo: MSE_API.h
 * Version: 1
 *===========================================================================*/
/*Descripción:
 *
 *
 *===========================================================================*/


#ifndef MSE_API_H_
#define MSE_API_H_

#include "MSE_OS_Core.h"

/********************************************************************************
 * Definicion de las constantes
 *******************************************************************************/
#define portMax_DELAY 0xFFFFFFFFFFFFFFFF	// Máxima cuenta de Ticks

#define LONG_COLA	 64						// Espacio de la cola en memoria

/********************************************************************************
 * Definicion de la estructura para los semaforos
 *******************************************************************************/
enum _estadoSemTake  {
	pdFalse,
	pdTrue
};

typedef enum _estadoSemTake statusSemTake;

enum _estadoSem  {
	LIBERADO,
	TOMADO
};

typedef enum _estadoSem statusSem;

struct _semaforo {
	tarea* tareaSemaforo;		// Tarea asociada al semáforo
	uint64_t delaySemaforo;		// Puede ser portMax_DELAY, son cantidad de ticks
	statusSem estadoSemaforo;

};

typedef struct _semaforo semaforo;
/********************************************************************************
 * Definicion de la estructura para las colas
 *******************************************************************************/
struct _cola {
	tarea* tareaIn;				// Puntero de la cola asociada a tareaIn
	tarea* tareaOut;			// Puntero de la cola asociada a tareaOut
	uint8_t dato[LONG_COLA];	// Es un array de punteros a datos
	uint16_t cantElementosMax;
	uint16_t contadorElementos;
	uint16_t longElemento;
};

typedef struct _cola cola;


/*=============[Definición de prototipos para las Tareas]=======================*/

void os_SemaforoInit(semaforo* sem); 		// Inicializa valores
statusSemTake os_SemaforoTake(semaforo* sem,uint64_t delayTicks);
void os_SemaforoGive(semaforo* sem);

void os_ColaInit(cola* buffer, uint16_t longDato); 		// Inicializa valores
void os_ColaPush(cola* buffer,void* dato);				// Ingresa un dato
void os_ColaPop(cola* buffer,void* dato);				// Saca un dato


#endif /* MSE_API_H_ */
