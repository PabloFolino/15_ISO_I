#ifndef MSE_API_H_
#define MSE_API_H_


#include "MSE_OS_Core.h"

#define portMax_DELAY 0xFFFFFFFFFFFFFFFF	// Máxima cuenta de Ticks

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

/*=============[Definición de prototipos para las Tareas]=======================*/
/*
 * Los semáforos no necesitan de una función de creación ya que se crean en tiemp de
 * complilación.
 */
void os_SemaforoInit(semaforo* sem); 		// Inicializa valores
statusSemTake os_SemaforoTake(semaforo* sem,uint64_t delayTicks);
void os_SemaforoGive(semaforo* sem);




#endif /* MSE_API_H_ */
