
/*=============================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/17
 * Archivo: main.c
 * Version: 1
 *===========================================================================*/
/*Descripción:
 * Ese módulo  es un ejemplo del programa principal para el sistema operativo.
 * Este programa se probó en la EDU-CIAA-NXP.
 * Es un S.O. estático, que en tiempo de compilación se especifican la cantidad
 * de tareas a utilizar, la cantidad de dsemáforos , y la cantidad de colas.
 *
 * Examen:
 * 	El programa mide el tiempo que pasa entre los flancos de dos teclas
 * TEC1 y TEC2.
 * Si las teclas se presionan en forma que se solapen informa por puerto
 * serie el tiempo trenscurrido entre los flancos y hace una secuencia de
 * leds, dependiendo que tecla se presionó primiro y qué tecla se solto
 * primero.
 * Si se presiona en forma no intercalada veces la misma tecla y la otra no,
 * informa por puerto serie ese ERROR.
 * Se implementó una rurina antirebote mixta(de complicado nomás), el primer
 * cambio se detecta por interrupciones y el segundo se verifica por encuesta.
 *
 * Nota: la tareaLed se la pone con PRIORIDAD_1 para demostar que funcionan
 * el scheduler. También se puede verificar que cuando todas las tareas de
 * encuentran BLOQUEADAS entra en funcionamiento la tareaIDLE.
 *
 *
 *===========================================================================*/

/*==================[inclusions]=============================================*/

#include "main.h"

#include "board.h"

#include "MSE_OS_Core.h"

#include "MSE_API.h"

#include "sapi.h"

#include "string.h"

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

#define BAUDRATE	460800

#define TEC1_PORT_NUM   0
#define TEC1_BIT_VAL    4

#define TEC2_PORT_NUM   0
#define TEC2_BIT_VAL    8

#define antiRebote		30

/*==================[internal data definition]===============================*/
enum _estadoBot  {
	NIVEL_1,
	DESCENDENTE,
	NIVEL_0,
	ASCENDENTE
};

typedef enum _estadoBot statusBot;


struct _dataLed{
	uint8_t Led;				// led a encender puede ser LEDS_VERDE, LEDS_ROJO ,
								// LEDS_AMARILLO o LEDS_RGB_AZUL
	uint16_t delta_suma;

};

typedef struct _dataLed dataLed;


struct _statusBotones {
	uint64_t b1_fanco_desc;		// ticks en donde se produce el flanco descendente del boton 1
	uint64_t b1_fanco_asc;		// ticks en donde se produce el flanco ascendente del boton 1
	statusBot b1_estado;
	uint64_t b2_fanco_desc;		// ticks en donde se produce el flanco descendente del boton 2
	uint64_t b2_fanco_asc;		// ticks en donde se produce el flanco ascendente del boton 2
	uint8_t contaNiveles;		// cuenta la cantidad de niveles detectados
	statusBot b2_estado;
};

typedef struct _statusBotones statusBotones;

/*==================[Global data declaration]==============================*/
// Reservo espacio para el estado de cada tarea
tarea estadoTareaBoton1,estadoTareaBoton2;
tarea estadoTareaUpdate,estadoTareaLed;

// Creo los semáforos binarios
semaforo semTecla1_descendente, semTecla1_ascendente;
semaforo semTecla2_descendente, semTecla2_ascendente;
semaforo semTecla1_OK, semTecla2_OK;

cola bufferLed;			// Creo una cola

statusBotones status_button;

/*==================[internal functions declaration]=========================*/
// Mensajes predifinidos
//char MSG_LedVerde[]={"Led verde encendido:"};
//char MSG_LedRojo[]={"Led rojo encendido:"};
//char MSG_LedAzul[]={"Led azul encendido:"};
//char MSG_LedAmarillo[]={"Led amarillo encendido:"};
//char MSG_Asc[]={"Tiempos entre flancos ascendentes: "};
//char MSG_Desc[]={"Tiempos entre flancos descendentes: "};
//char MSG_Mseg[]={" ms"};


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


// Inicializa la variable que controla la máquina de estado del programa
void statusBUttonInit(void){
	status_button.b1_fanco_asc=0;
	status_button.b1_fanco_desc=0;
	status_button.b1_estado=NIVEL_1;
	status_button.b2_fanco_asc=0;
	status_button.b2_fanco_desc=0;
	status_button.contaNiveles=0;
	status_button.b2_estado=NIVEL_1;
}

// Se informa que se tocaron las teclas en forma no intercalada
void informo_error(void) {
	statusBUttonInit();
	uartWriteString (UART_USB,"ERROR: -- se tocaron dos veces la misma tecla --\n\r");
}



/*==================[Definicion de tareas para el OS]==========================*/

//==================[Tareas para probar el SO]==================================
/* Si los LEDs titilan, significa que las dos variables de cada tarea
 * se incrementan en forma pareja, y por otro lado informan visualmente
 * que el SO se encuentra funcionando
 */
void tareaBoton1(void)  {
	while (1) {
		while(status_button.b1_estado==NIVEL_1){
			os_SemaforoTake(&semTecla1_descendente,portMax_DELAY);
			tareaDelay(antiRebote);
			if(gpioRead(TEC1)==0){
				// Guardo t1 del boton 1
				status_button.b1_fanco_desc=os_getSytemTicks();
				status_button.contaNiveles++;
				status_button.b1_estado=NIVEL_0;
				}
			}
		while(status_button.b1_estado==NIVEL_0){
			os_SemaforoTake(&semTecla1_ascendente,portMax_DELAY);
			tareaDelay(antiRebote);
			 if(gpioRead(TEC1)==1){
				// Guardo t2 del boton 1
				status_button.b1_fanco_asc=os_getSytemTicks();
				status_button.contaNiveles++;
				status_button.b1_estado=NIVEL_1;
				if(status_button.b2_fanco_asc==0 &&status_button.b2_fanco_desc==0){
					// Se informa que se tocaron las teclas en forma no intercalada
					informo_error();
					}
				else{
					// Informo que el boton 1 se produjeron los dos flancos
					os_SemaforoGive(&semTecla1_OK);
					}
				}
			}
	}
}

void tareaBoton2(void)  {
	while (1) {
		while(status_button.b2_estado==NIVEL_1){
			os_SemaforoTake(&semTecla2_descendente,portMax_DELAY);
			tareaDelay(antiRebote);
			if(gpioRead(TEC2)==0){
				// Guardo t1 del boton 2
				status_button.b2_fanco_desc=os_getSytemTicks();
				status_button.contaNiveles++;
				status_button.b2_estado=NIVEL_0;
				}
			}
		while(status_button.b2_estado==NIVEL_0){
			os_SemaforoTake(&semTecla2_ascendente,portMax_DELAY);
			tareaDelay(antiRebote);
			if(gpioRead(TEC2)==1){
				// Guardo t2 del boton 2
				status_button.b2_fanco_asc=os_getSytemTicks();
				status_button.contaNiveles++;
				status_button.b2_estado=NIVEL_1;
				if(status_button.b1_fanco_asc==0 && status_button.b1_fanco_desc==0){
					// Informo error se tocaron 2 veces misma tecla
					informo_error();
					}
				else{
					// Se informa que se tocaron las teclas en forma no intercalada
					os_SemaforoGive(&semTecla2_OK);
					}
				}
			}
	}
}



void tareaUpdate(void)  {
	int16_t delta_t1, delta_t2;
	dataLed datoToLed;
	uint8_t statusLedAux;
	char s_ascendente[8],s_descendente[8];

	while (1) {
		// Espero que se presionen las dos teclas
		os_SemaforoTake(&semTecla1_OK, portMax_DELAY);
		os_SemaforoTake(&semTecla2_OK, portMax_DELAY);

//		//===================================
//		tareaDelay(2000);
//		status_button.b1_fanco_asc=1000;
//		status_button.b1_fanco_desc=5900;
//		status_button.b2_fanco_asc=2000;
//		status_button.b2_fanco_desc=3000;
//		status_button.contaNiveles=4;
//		//===================================


		// Calculo los delta, es una sección crítica
		//irqOff();
		delta_t1=status_button.b2_fanco_desc-status_button.b1_fanco_desc;
		delta_t2=status_button.b2_fanco_asc-status_button.b1_fanco_asc;
		statusLedAux=0;
		if(status_button.contaNiveles==4){
			// Selecciono en que estado estoy
			if(delta_t1>0 && delta_t2>0) statusLedAux=LEDS_VERDE;
			if(delta_t1>=0 && delta_t2<0) statusLedAux=LEDS_ROJO;
			if(delta_t1<0 && delta_t2>=0) statusLedAux=LEDS_AMARILLO;
			if(delta_t1<0 && delta_t2<0) statusLedAux=LEDS_RGB_AZUL;
			}

		// Reinicializa la variable
		statusBUttonInit();
		//irqOn();

		if(statusLedAux!=0){
			// Informo que se enciendan los led
			datoToLed.delta_suma=abs(delta_t1)+abs(delta_t2);
			datoToLed.Led=statusLedAux;
			os_ColaPush(&bufferLed,&datoToLed);

			// Convierto a string los números
			itoa(abs(delta_t2),s_ascendente,10);
			itoa(abs(delta_t1),s_descendente,10);

			//  Tx por la UART
			switch(statusLedAux){
			case LEDS_VERDE:
				uartWriteString(UART_USB,"Led verde encendido:\n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos ascendentes:");
				uartWriteString(UART_USB,s_ascendente);
				uartWriteString(UART_USB," mseg \n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos descendentes:");
				uartWriteString(UART_USB,s_descendente);
				uartWriteString(UART_USB," mseg \n\r");
				break;
			case LEDS_ROJO:
				uartWriteString(UART_USB,"Led rojo encendido:\n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos ascendentes:");
				uartWriteString(UART_USB,s_ascendente);
				uartWriteString(UART_USB," mseg \n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos descendentes:");
				uartWriteString(UART_USB,s_descendente);
				uartWriteString(UART_USB," mseg \n\r");
				break;
			case LEDS_AMARILLO:
				uartWriteString(UART_USB,"Led amarillo encendido:\n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos ascendentes:");
				uartWriteString(UART_USB,s_ascendente);
				uartWriteString(UART_USB," mseg \n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos descendentes:");
				uartWriteString(UART_USB,s_descendente);
				uartWriteString(UART_USB," mseg \n\r");
				break;
			case LEDS_RGB_AZUL:
				uartWriteString(UART_USB,"Led azul encendido:\n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos ascendentes:");
				uartWriteString(UART_USB,s_ascendente);
				uartWriteString(UART_USB," mseg \n\r\t");
				uartWriteString(UART_USB,"Tiempos entre flancos descendentes:");
				uartWriteString(UART_USB,s_descendente);
				uartWriteString(UART_USB," mseg \n\r");
				break;
				}
		}
	}
}

void tareaLed(void)  {
	dataLed datoToLed;

	while (1) {
		os_ColaPop(&bufferLed,&datoToLed);
		Board_LED_Set(datoToLed.Led,ON);
		tareaDelay(datoToLed.delta_suma);
		Board_LED_Set(datoToLed.Led,OFF);
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
	os_SemaforoGive(&semTecla2_descendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 2 ) );
}

void tecla2_flanco_asc(void)  {
	os_SemaforoGive(&semTecla2_ascendente);
	Chip_PININT_ClearIntStatus( LPC_GPIO_PIN_INT, PININTCH( 3 ) );
}

/*======================[Programa principal]==================================*/

int main(void)  {

	initHardware();

	// Inicializa la variable
	statusBUttonInit();

	//*************************************************************
	// Configura las tareas
	os_InitTarea(tareaBoton1, &estadoTareaBoton1,PRIORIDAD_0);
	os_InitTarea(tareaBoton2, &estadoTareaBoton2,PRIORIDAD_0);
	os_InitTarea(tareaLed, &estadoTareaLed,PRIORIDAD_1);
	os_InitTarea(tareaUpdate, &estadoTareaUpdate,PRIORIDAD_0);

	// Configuro la cola
	os_ColaInit(&bufferLed,sizeof(dataLed));
	//Configuro los semáforos
	os_SemaforoInit(&semTecla1_ascendente);
	os_SemaforoInit(&semTecla1_descendente);
	os_SemaforoInit(&semTecla2_ascendente);
	os_SemaforoInit(&semTecla2_descendente);
	os_SemaforoInit(&semTecla1_OK);
	os_SemaforoInit(&semTecla2_OK);
	// Instalo las interrupciones
	os_InstalarIRQ(PIN_INT0_IRQn,tecla1_flanco_desc);
	os_InstalarIRQ(PIN_INT1_IRQn,tecla1_flanco_asc);
	os_InstalarIRQ(PIN_INT2_IRQn,tecla2_flanco_desc);
	os_InstalarIRQ(PIN_INT3_IRQn,tecla2_flanco_asc);
	//*************************************************************


	os_Init();					// Ejecuta el Sistema Operativo


	while (1) {					// Se queda esperando
		__WFI();
	}

}



/** @} doxygen end group definition */

/*==================[end of file]============================================*/
