/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"


/*==================[macros and definitions]=================================*/

#define MILISEC		1000

/*==================[Global data declaration]==============================*/
tarea estadoTarea1,estadoTarea2;	// Reservo espacio para el estado de cada tarea

/*==================[internal functions declaration]=========================*/

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/

/** @brief hardware initialization function
 *	@return none
 */
static void initHardware(void)  {
	Board_Init();
	SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock / MILISEC);		//systick 1ms
}


/*==================[Definicion de tareas para el OS]==========================*/
void tarea1(void)  {
	uint16_t h = 0;
	uint16_t i = 0;
	while (1) {
		h++;
		i++;
	}
}

void tarea2(void)  {
	uint16_t j = 0;
	uint16_t k = 0;

	while (1) {
		j++;
		k++;
	}
}

/*============================================================================*/

int main(void)  {

	initHardware();

	os_Init();

	os_InitTarea(tarea1, &estadoTarea1,PRIORIDAD_0);
	os_InitTarea(tarea2, &estadoTarea2,PRIORIDAD_0);
//	os_InitTarea(tarea1, &stack1, &sp_tarea1);
//	os_InitTarea(tarea2, &stack2, &sp_tarea2);

	while (1) {					// Se queda esperando
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
