/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"


/*==================[Macros and definitions]=================================*/

#define MILISEC		1000		// Con 1000 el systick=1ms
#define VALOR		1000
#define RESET_C		1000000

#define LEDS_RGB_VERDE  0x01
#define LEDS_AMARILLO	0x03
#define LEDS_VERDE		0x05
#define LEDS_ROJO       0x04

/*==================[Global data declaration]==============================*/
tarea estadoTarea1,estadoTarea2,estadoTarea3,estadoTarea4;	// Reservo espacio para el estado de cada tarea

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
		if(h==VALOR && i==VALOR){
			Board_LED_Toggle(LEDS_RGB_VERDE);
			//tareaDelay(2000);				// En milisegundos
		}
		if(h==RESET_C && i==RESET_C){
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
		if(j==VALOR && k==VALOR){
			Board_LED_Toggle(LEDS_AMARILLO);
			tareaDelay(2000);				// En milisegundos

		}
		if(j==RESET_C && k==RESET_C){
				j=0;
				k=0;
		}
	}
}

void tarea3(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
		if(r==VALOR && s==VALOR){
			Board_LED_Toggle(LEDS_ROJO);
		}
		if(r==RESET_C && s==RESET_C){
				r=0;
				s=0;
		}
	}
}

void tarea4(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
		if(r==VALOR && s==VALOR){
			Board_LED_Toggle(LEDS_VERDE);
			tareaDelay(4000);				// En milisegundos
		}
		if(r==RESET_C && s==RESET_C){
				r=0;
				s=0;
		}
	}
}
//================================================================

/*======================[Programa principal]==================================*/

int main(void)  {

	initHardware();


	os_InitTarea(tarea1, &estadoTarea1,PRIORIDAD_1);
	os_InitTarea(tarea2, &estadoTarea2,PRIORIDAD_0);
	os_InitTarea(tarea3, &estadoTarea3,PRIORIDAD_1);
	os_InitTarea(tarea4, &estadoTarea4,PRIORIDAD_0);

	os_Init();					// Ejecuta el Sistema Operativo


	while (1) {					// Se queda esperando
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
