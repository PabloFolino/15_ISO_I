/*=============================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/14
 * Archivo: MSE_OS_IRQ.c
 * Version: 1
 *===========================================================================*/
/*Descripción:
 * Este módulo  declara el vector de interuupciones de la  palca EDU-CIAA-NXP.
 * Es un S.O. estático, que en tiempo de compilación se especifican la cantidad
 * de tareas a utilizar, la cantidad de dsemáforos , y la cantidad de colas.
 *
 *===========================================================================*/


#include "MSE_OS_IRQ.h"


static void* isr_vector_usuario[CANT_IRQ];		//vector de punteros a funciones para nuestras interrupciones


/*************************************************************************************************
	 *  @brief Instala una interrupción.
     *
     *  @details
     *   Debemos pasarle el tipo de interrupción y la función del usuario que desea  atender esa
     *    interrupción.
     * 	 La funcion devuelve true si fue exitosa o false en caso contrario.
     *
	 *  @param 		LPC43XX_IRQn_Type irq, void* usr_isr
	 *  @return     true o false.
***************************************************************************************************/
bool os_InstalarIRQ(LPC43XX_IRQn_Type irq, void* usr_isr)  {
	bool status = false;

	/*
	 * Determinamos si la insterrupción no fue definida anteriormente por el usuario
	 * Entonces cargamos el puntero de la funcion del usuario y  habilitamos esa
	 * interrupción en el NVIC
	 */

	if (isr_vector_usuario[irq] == NULL) {
		isr_vector_usuario[irq] = usr_isr;
		NVIC_ClearPendingIRQ(irq);
		NVIC_EnableIRQ(irq);
		status = true;
	}

	return status;
}



/*************************************************************************************************
	 *  @brief Desinstala una interrupción.
     *
     *  @details
     *  Debemos pasarle el tipo de interrupcion que queremos quitar del vector que definimos con
     *  los punteros a funciones del usuario La funcion devuelve true si fue exitosa o false en
     *  caso contrario.
     *
	 *  @param 		LPC43XX_IRQn_Type irq
	 *  @return     true o false.
***************************************************************************************************/
bool os_RemoverIRQ(LPC43XX_IRQn_Type irq)  {
	bool status = 0;

	if (isr_vector_usuario[irq] != NULL) {
		isr_vector_usuario[irq] = NULL;
		NVIC_ClearPendingIRQ(irq);
		NVIC_DisableIRQ(irq);
		status = true;
	}

	return status;
}



/*************************************************************************************************
	 *  @brief Función que llama a todas las  interrupciones.
     *
     *  @details
     *  Se encarga de llamar a la funcion de usuario que haya sido cargada.
     *
     *  IMORTANTE : LAS FUNCIONES DE USUARIO LLAMADAS POR ESTA FUNCION SE EJECUTAN EN MODO HANDLER
     *  DE IGUAL FORMA. CUIDADO CON LA CARGA DE CODIGO EN ELLAS, MISMAS REGLAS QUE EN BARE METAL.
     *
	 *  @param 		LPC43XX_IRQn_Type IRQn
	 *  @return     none.
***************************************************************************************************/
static void os_IRQHandler(LPC43XX_IRQn_Type IRQn)  {
	estadoOS estadoPrevio_OS;
	void (*funcion_usuario)(void);

	//Guardamos el estado del sistema para restablecerlo al salir de la interrupción.
	estadoPrevio_OS = os_getEstadoSistema();

	// Actualizamos el estado del sistema operativo
	os_setEstadoSistema(OS_IRQ_RUN);

	// Llamamos a la funcion definida por el usuario
	funcion_usuario = isr_vector_usuario[IRQn];
	funcion_usuario();

	// Retomamos el estado anterior de sistema operativo
	os_setEstadoSistema(estadoPrevio_OS);


	// Debemos limpiar la interrupcion que acabamos de atender, sino se entra por siempre
	// a la funcion de interrupcion
	NVIC_ClearPendingIRQ(IRQn);


	// Si hubo alguna llamada desde una interrupción a una API liberando un evento, entonces
	// llamamos al scheduler
	if (os_getFlagISR())  {
		os_setFlagISR(false);
		os_Yield();
	}
}

/*==================[interrupt service routines]=============================*/

void DAC_IRQHandler(void){os_IRQHandler(         DAC_IRQn         );}
void M0APP_IRQHandler(void){os_IRQHandler(       M0APP_IRQn       );}
void DMA_IRQHandler(void){os_IRQHandler(         DMA_IRQn         );}
void FLASH_EEPROM_IRQHandler(void){os_IRQHandler(RESERVED1_IRQn   );}
void ETH_IRQHandler(void){os_IRQHandler(         ETHERNET_IRQn    );}
void SDIO_IRQHandler(void){os_IRQHandler(        SDIO_IRQn        );}
void LCD_IRQHandler(void){os_IRQHandler(         LCD_IRQn         );}
void USB0_IRQHandler(void){os_IRQHandler(        USB0_IRQn        );}
void USB1_IRQHandler(void){os_IRQHandler(        USB1_IRQn        );}
void SCT_IRQHandler(void){os_IRQHandler(         SCT_IRQn         );}
void RIT_IRQHandler(void){os_IRQHandler(         RITIMER_IRQn     );}
void TIMER0_IRQHandler(void){os_IRQHandler(      TIMER0_IRQn      );}
void TIMER1_IRQHandler(void){os_IRQHandler(      TIMER1_IRQn      );}
void TIMER2_IRQHandler(void){os_IRQHandler(      TIMER2_IRQn      );}
void TIMER3_IRQHandler(void){os_IRQHandler(      TIMER3_IRQn      );}
void MCPWM_IRQHandler(void){os_IRQHandler(       MCPWM_IRQn       );}
void ADC0_IRQHandler(void){os_IRQHandler(        ADC0_IRQn        );}
void I2C0_IRQHandler(void){os_IRQHandler(        I2C0_IRQn        );}
void SPI_IRQHandler(void){os_IRQHandler(         I2C1_IRQn        );}
void I2C1_IRQHandler(void){os_IRQHandler(        SPI_INT_IRQn     );}
void ADC1_IRQHandler(void){os_IRQHandler(        ADC1_IRQn        );}
void SSP0_IRQHandler(void){os_IRQHandler(        SSP0_IRQn        );}
void SSP1_IRQHandler(void){os_IRQHandler(        SSP1_IRQn        );}
void UART0_IRQHandler(void){os_IRQHandler(       USART0_IRQn      );}
void UART1_IRQHandler(void){os_IRQHandler(       UART1_IRQn       );}
void UART2_IRQHandler(void){os_IRQHandler(       USART2_IRQn      );}
void UART3_IRQHandler(void){os_IRQHandler(       USART3_IRQn      );}
void I2S0_IRQHandler(void){os_IRQHandler(        I2S0_IRQn        );}
void I2S1_IRQHandler(void){os_IRQHandler(        I2S1_IRQn        );}
void SPIFI_IRQHandler(void){os_IRQHandler(       RESERVED4_IRQn   );}
void SGPIO_IRQHandler(void){os_IRQHandler(       SGPIO_INT_IRQn   );}
void GPIO0_IRQHandler(void){os_IRQHandler(       PIN_INT0_IRQn    );}
void GPIO1_IRQHandler(void){os_IRQHandler(       PIN_INT1_IRQn    );}
void GPIO2_IRQHandler(void){os_IRQHandler(       PIN_INT2_IRQn    );}
void GPIO3_IRQHandler(void){os_IRQHandler(       PIN_INT3_IRQn    );}
void GPIO4_IRQHandler(void){os_IRQHandler(       PIN_INT4_IRQn    );}
void GPIO5_IRQHandler(void){os_IRQHandler(       PIN_INT5_IRQn    );}
void GPIO6_IRQHandler(void){os_IRQHandler(       PIN_INT6_IRQn    );}
void GPIO7_IRQHandler(void){os_IRQHandler(       PIN_INT7_IRQn    );}
void GINT0_IRQHandler(void){os_IRQHandler(       GINT0_IRQn       );}
void GINT1_IRQHandler(void){os_IRQHandler(       GINT1_IRQn       );}
void EVRT_IRQHandler(void){os_IRQHandler(        EVENTROUTER_IRQn );}
void CAN1_IRQHandler(void){os_IRQHandler(        C_CAN1_IRQn      );}
void ADCHS_IRQHandler(void){os_IRQHandler(       ADCHS_IRQn       );}
void ATIMER_IRQHandler(void){os_IRQHandler(      ATIMER_IRQn      );}
void RTC_IRQHandler(void){os_IRQHandler(         RTC_IRQn         );}
void WDT_IRQHandler(void){os_IRQHandler(         WWDT_IRQn        );}
void M0SUB_IRQHandler(void){os_IRQHandler(       M0SUB_IRQn       );}
void CAN0_IRQHandler(void){os_IRQHandler(        C_CAN0_IRQn      );}
void QEI_IRQHandler(void){os_IRQHandler(         QEI_IRQn         );}


