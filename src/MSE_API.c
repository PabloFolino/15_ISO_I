
#include "MSE_API.h"

/*************************************************************************************************
	 *  @brief Función que inicializa un semáforo binario
     *
     *  @details
     *  El semáforo se crea en etapa de complación mediante la creaión de una variable "semaforo"
     *  Esta función conigura los valores iniciales de esa variable.
     *
	 *  @param 		semaforo* sem,uint32_t delayTicks.
	 *  @return     bool.
***************************************************************************************************/
void os_SemaforoInit(semaforo* sem){
	sem->tareaSemaforo=NULL;
	sem->estadoSemaforo=TOMADO;
	sem->delaySemaforo=portMax_DELAY;
}


/*************************************************************************************************
	 *  @brief Función que toma un semáforo por un tiempo determinado
     *
     *  @details
     *  Cuando la tarea toma el semáforo, se pasa a estado TAREA_BLOCKED. Si el semáforo ya estaba
     *  bloqueado, sigue estando en estado bloqueado y se reescribe el delayTicks.
     *  Devuelve pdTrue si se pudo tomar correctamente, o pdFalse si durante delayTicks nadie lo liberó.
     *
	 *  @param 		semaforo* sem,uint32_t delayTicks.
	 *  @return     bool.
***************************************************************************************************/
statusSemTake os_SemaforoTake (semaforo* sem,uint64_t delayTicks) {
	uint64_t ticks_finales;
	tarea * tareaActual;

	// Leo el valor del contador de ticks
	ticks_finales= os_getSytemTicks()+delayTicks;

	// Actualiza estado
	irqOff();
	tareaActual=os_getTareaActual();
	sem->tareaSemaforo=tareaActual;
	tareaActual->estado=TAREA_BLOCKED;
	tareaActual->ticks_bloqueada=TICKS_ON;	// Se hace para que SysTick_Handler no lo ponga en TAREA_READY
	sem->delaySemaforo=ticks_finales;
	sem->estadoSemaforo=TOMADO;
	irqOn();
	// Llama al scheduler
	os_Yield();

	while(os_getSytemTicks()<ticks_finales){
		tareaActual->ticks_bloqueada=TICKS_ON; 	// Se hace para que SysTick_Handler no lo ponga en TAREA_READY
		if(sem->estadoSemaforo==LIBERADO)
			return pdTrue;
		}
	return pdFalse;									// no fue liberado durante los delayTicks
}

/********************************************************************************
	 *  @brief Liberar un semaforo
     *
     *  @details
     *   Esta función se utiliza para liberar un semáforo.
     *
	 *  @param		sem		Semaáforo a liberar
	 *  @return     None.
 *******************************************************************************/
void os_SemaforoGive(semaforo* sem)  {
	tarea * tareaLiberar;

	tareaLiberar=sem->tareaSemaforo;

	if (sem->estadoSemaforo == TOMADO && tareaLiberar!= NULL)  {
		tareaLiberar->ticks_bloqueada=TICKS_OFF;
		tareaLiberar->estado=TAREA_READY;
		sem->estadoSemaforo = LIBERADO;
	}
}
