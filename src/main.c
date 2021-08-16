
/*=============================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/14
 * Archivo: main.c
 * Version: 1
 *===========================================================================*/
/*Descripción:
 * Ese módulo  es un ejemplo del programa principal para el sistema operativo.
 * Este programa se probó en la EDU-CIAA-NXP.
 * Es un S.O. estático, que en tiempo de compilación se especifican la cantidad
 * de tareas a utilizar, la cantidad de dsemáforos , y la cantidad de colas.
 *
 *===========================================================================*/

/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"

#include "MSE_API.h"

#include "sapi.h"

/*==================[Macros and definitions]=================================*/

#define MILISEC		1000		// Con 1000 el systick=1ms
#define VALOR		1000
#define RESET_C		1000000

#define LEDS_RGB_ROJO  	0x00
#define LEDS_RGB_VERDE  0x01
#define LEDS_RGB_AZUL	0x02
#define LEDS_AMARILLO	0x03
#define LEDS_ROJO       0x04
#define LEDS_VERDE		0x05



#define BAUDRATE	115200


#define TEC1_PORT_NUM   0
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   0
#define TEC2_BIT_VAL    8


/*==================[Global data declaration]==============================*/
tarea estadoTarea1,estadoTarea2,estadoTarea3,estadoTarea4;	// Reservo espacio para el estado de cada tarea
tarea estadoTarea5,estadoTarea6,estadoTarea7;
tarea estadoUART;
tarea g_sEncenderLed, g_sApagarLed;

semaforo sem1;			// Creo los semáforos binarios
semaforo semTecla1_descendente, semTecla1_ascendente;

cola buffer1;			// Creo una cola
cola colaUart;

/*=================[Variables para testear el SO]============================*/
//char buffer_test[]={"Hola es una prueba"};
char buffer_test[]={"123456789abcdefghijklmnopqrstuvwxyz"};
//char msgLedVerde[]={"Led verde encendido:"+'\n'+'\r'};
//char msgLedRojo[]={"Led rojo encendido:"+'\n'+'\r'};
//char msgLedAzul[]={"Led azul encendido:"+'\n'+'\r'};
//char msgLedAmarillo[]={"Led amarillo encendido:"+'\n'+'\r'};
//char msgAsc[]={"\t"+"Tiempos entre flancos ascendentes: "};
//char msgDesc[]={"\t"+"Tiempos entre flancos descendentes: "};
//char msgMseg[]={" ms"+'\n'+'\r'};

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

	/*
	 * Seteamos la interrupcion 0 para el flanco descendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 0, TEC1_PORT_NUM, TEC1_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 0 ) );

	/*
	 * Seteamos la interrupcion 1 para el flanco ascendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 1, TEC1_PORT_NUM, TEC1_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) ); // INT1 flanc
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 1 ) );

	/*
	 * Seteamos la interrupcion 2 para el flanco descendente en la tecla 1
	 */
	Chip_SCU_GPIOIntPinSel( 2, TEC2_PORT_NUM, TEC2_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) ); // INT0 flanco descendente
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
	Chip_PININT_EnableIntLow( LPC_GPIO_PIN_INT, PININTCH( 2 ) );

	/*
	 * Seteamos la interrupcion 3 para el flanco ascendente en la tecla 2
	 */
	Chip_SCU_GPIOIntPinSel( 3, TEC2_PORT_NUM, TEC2_BIT_VAL );
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) ); // INT1 flanc
	Chip_PININT_SetPinModeEdge( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
	Chip_PININT_EnableIntHigh( LPC_GPIO_PIN_INT, PININTCH( 3 ) );

	uartConfig(UART_USB, BAUDRATE);

}

// Convierte de int a char
// Se declara las variables por ejemplo:
//			char smun[5]
//			int num=587
// Se la invoca usando:
//			itoa(num,snum,10);
void itoa (uint16_t value, char *result, uint8_t base) {
	// Se chequea si la base es válida
	if (base < 2 || base > 36) {
		*result = '\0';
		}
	else {
		char* ptr = result, *ptr1 = result, tmp_char;
		int tmp_value;
		do {
			tmp_value = value; value /= base;
			*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
		} while ( value );
		// Aplica el signo si es negativo
		if (tmp_value < 0) *ptr++ = '-';
		*ptr-- = '\0';
		while (ptr1 < ptr) {
			tmp_char = *ptr;
			*ptr--= *ptr1;
			*ptr1++ = tmp_char;
			}
	}
}


/*==================[Definicion de tareas para el OS]==========================*/

//==================[Tareas para probar el SO]==================================
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
//			Board_LED_Toggle(LEDS_RGB_VERDE);
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
			//if(contador==4) os_setTareaPrioridad(&estadoTarea1,PRIORIDAD_2);
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
			returnTake=os_SemaforoTake(&sem1,2000);		// Tomo el semáforo1 por 5 segundos
			if(returnTake==pdTrue)
				Board_LED_Toggle(LEDS_VERDE);
	}
}

void tarea5(void)  {
	uint16_t i=0;;
	while (1) {
		if(gpioRead(TEC1))
			os_ColaPush(&buffer1,&buffer_test[i]);
		if(i>=sizeof(buffer_test)-1){
			i=0;
			}
		else{
			i++;
		}
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


void tareaUART(void)  {
	uint32_t i = 0;
	char data;
	char snum[10];
	uint16_t c;

	while (1) {
		os_ColaPop(&buffer1,&data);
		tareaDelay(100);
		uartWriteByte(UART_USB,data);
		if(data==buffer_test[i])
			Board_LED_Toggle(LEDS_RGB_AZUL);
		if(i>=sizeof(buffer_test)-1){
			itoa(278,snum,10);
			c=0;
			while(snum[c]!='\0'){
					uartWriteByte(UART_USB,snum[c]);
					c++;
			};
			uartWriteByte(UART_USB,13);
			uartWriteByte(UART_USB,10);
			i=0;
			}
		else{
			i++;
		}
	}
}

void encenderLed(void)  {
	char msg[25];
	uint8_t i;

	strcpy(msg,"Se presiono la tecla 1\n\r");

	while (1) {

		os_SemaforoTake(&semTecla1_descendente, portMax_DELAY);
		gpioWrite(LED1,true);

		i = 0;
		while(msg[i] != NULL)  {
			os_ColaPop(&colaUart,(msg + i));
			i++;
		}
	}
}

void apagarLed(void)  {
	char msg[25];
	uint8_t i;

	strcpy(msg,"Se solto la tecla 1\n\r");

	while (1) {

		os_SemaforoTake(&semTecla1_ascendente,portMax_DELAY);
		gpioWrite(LED1,false);

		i = 0;
		while(msg[i] != NULL)  {
			os_ColaPop(&colaUart,(msg + i));
			i++;
		}
	}
}
//==============================================================================
//==================[Atención a Interrupciones]=================================
void tecla1_flanco_desc(void) {
	os_SemaforoGive(&semTecla1_descendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 0 ) );
}

void tecla1_flanco_asc(void)  {
	os_SemaforoGive(&semTecla1_ascendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 1 ) );
}

void tecla2_flanco_desc(void) {
	os_SemaforoGive(&semTecla1_descendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
}

void tecla2_flanco_asc(void)  {
	os_SemaforoGive(&semTecla1_ascendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
}

/*======================[Programa principal]==================================*/

int main(void)  {

	initHardware();

	os_InitTarea(tarea1, &estadoTarea1,PRIORIDAD_0);
	os_InitTarea(tarea2, &estadoTarea2,PRIORIDAD_0);
	os_InitTarea(tarea3, &estadoTarea3,PRIORIDAD_0);
	os_InitTarea(tarea4, &estadoTarea4,PRIORIDAD_0);
	os_InitTarea(tarea5, &estadoTarea5,PRIORIDAD_0);
	os_InitTarea(tarea6, &estadoTarea6,PRIORIDAD_0);
	os_InitTarea(tarea7, &estadoTarea7,PRIORIDAD_0);
	os_InitTarea(tareaUART, &estadoUART,PRIORIDAD_0);

//*************************************************************
	os_InitTarea(encenderLed, &g_sEncenderLed,PRIORIDAD_0);
	os_InitTarea(apagarLed, &g_sApagarLed,PRIORIDAD_0);

	os_ColaInit(&colaUart,sizeof(char));
	os_SemaforoInit(&semTecla1_ascendente);
	os_SemaforoInit(&semTecla1_descendente);




	os_InstalarIRQ(PIN_INT0_IRQn,tecla1_flanco_desc);
	os_InstalarIRQ(PIN_INT1_IRQn,tecla1_flanco_asc);
	os_InstalarIRQ(PIN_INT2_IRQn,tecla2_flanco_desc);
	os_InstalarIRQ(PIN_INT3_IRQn,tecla2_flanco_asc);
	//*************************************************************



	//Inicializo los semáforos
	os_SemaforoInit(&sem1);

	os_ColaInit(&buffer1, sizeof(char));

	os_Init();					// Ejecuta el Sistema Operativo


	while (1) {					// Se queda esperando
		__WFI();
	}
}



/** @} doxygen end group definition */

/*==================[end of file]============================================*/
