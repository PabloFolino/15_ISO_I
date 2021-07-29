/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"


/*==================[macros and definitions]=================================*/

#define MILISEC		1000		// Con 1000 el systick=1ms
#define VALOR		1000
#define RESET_C		1000000

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
	SysTick_Config(SystemCoreClock / MILISEC);		//systick=1ms
}


/*==================[Definicion de tareas para el OS]==========================*/

//==================[Tareas para probar el SO]====================
/* Si los LEDs titilan, significa que las dos variables de cada tarea
 * se incrementan en forma pareja, y por otro lado informan visualmente
 * que el SO se encuentra funcionando
 */
void tarea1(void)  {
	uint32_t h = 0;
	uint32_t i = 0;
	while (1) {
		h++;
		i++;
		if(h==VALOR & i==VALOR) Board_LED_Toggle(LEDS_LED1);
		if(h==RESET_C & i==RESET_C){
				h=0;
				i=0;
				}
	}
}

void tarea2(void)  {
	uint32_t j = 0;
	uint32_t k = 0;
	while (1) {
		j++;
		k++;
		if(j==VALOR & k==VALOR) Board_LED_Toggle(LEDS_LED2);
		if(j==RESET_C & k==RESET_C){
				j=0;
				k=0;
		}
	}
}
//================================================================

/*======================[Programa principal]==================================*/

int main(void)  {

	initHardware();

	os_Init();

	os_InitTarea(tarea1, &estadoTarea1,PRIORIDAD_0);
	os_InitTarea(tarea2, &estadoTarea2,PRIORIDAD_0);


	while (1) {					// Se queda esperando
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
