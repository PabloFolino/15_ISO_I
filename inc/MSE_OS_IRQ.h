/*=============================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/14
 * Archivo: MSE_OS_IRQ.h
 * Version: 1
 *===========================================================================*/
/*Descripción:
 *
 * Este módulo  declara el vector de interupciones de la  palca EDU-CIAA-NXP.
 *
 *===========================================================================*/

#ifndef MSE_OS_INC_MSE_OS_IRQ_H_
#define MSE_OS_INC_MSE_OS_IRQ_H_


#include "MSE_OS_Core.h"
#include "MSE_API.h"
#include "board.h"
#include "cmsis_43xx.h"


/********************************************************************************
 * Definicion de las constantes
 *******************************************************************************/
#define CANT_IRQ	53

/********************************************************************************
 * Definicion de las variables externas
 *******************************************************************************/
extern osControl control_OS;


/*=============[Definición de prototipos para las Tareas]=======================*/
bool os_InstalarIRQ(LPC43XX_IRQn_Type irq, void* usr_isr);
bool os_RemoverIRQ(LPC43XX_IRQn_Type irq);


#endif /* MSE_OS_INC_MSE_OS_IRQ_H_ */
