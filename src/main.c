/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"

#include "MSE_API.h"

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
tarea estadoTarea5,estadoTarea6,estadoTarea7,estadoTarea8;

semaforo sem1;			// Creo los semáforos binarios

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
	uint8_t contador=0;

	while (1) {
		j++;
		k++;
		if(j==VALOR && k==VALOR){
			Board_LED_Toggle(LEDS_AMARILLO);
			os_SemaforoGive(&sem1);			// Libero el semáforo1
			tareaDelay(1000);				// En milisegundos
			contador++;
			if(contador==4) os_setTareaPrioridad(&estadoTarea1,PRIORIDAD_2);
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
	statusSemTake returnTake=pdFalse;

	while (1) {
			returnTake=os_SemaforoTake(&sem1,1108);		// Tomo el semáforo1 por 5 segundos
			if(returnTake==pdTrue)
				Board_LED_Toggle(LEDS_VERDE);
	}
}

void tarea5(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
	}
}

void tarea6(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
	}
}

void tarea7(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
	}
}

void tarea8(void)  {
	uint32_t r = 0;
	uint32_t s = 0;
	while (1) {
		r++;
		s++;
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
	os_InitTarea(tarea4, &estadoTarea5,PRIORIDAD_0);
	os_InitTarea(tarea4, &estadoTarea6,PRIORIDAD_0);
	os_InitTarea(tarea4, &estadoTarea7,PRIORIDAD_0);
	os_InitTarea(tarea4, &estadoTarea8,PRIORIDAD_0);

	//Inicializo los semáforos
	os_SemaforoInit(&sem1);


	os_Init();					// Ejecuta el Sistema Operativo


	while (1) {					// Se queda esperando
		__WFI();
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/
