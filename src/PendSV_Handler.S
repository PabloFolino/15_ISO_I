	/*
		.syntax unified

		Two slightly different syntaxes are support for ARM and THUMB instructions.
		The default, divided, uses the old style where ARM and THUMB instructions had their own,
		separate syntaxes. The new, unified syntax, which can be selected via the
		.syntax directive
	*/

	.syntax unified
	.global PendSV_Handler



	/*
		Se cambia a la seccion .text, donde se almacena el programa en flash
	*/
	.text

	/*
		Indicamos que la proxima funcion debe ser tratada como codigo thumb al ser compilada
		Ver documentacion al respecto para mas detalles
	*/
	.thumb_func



PendSV_Handler:

	/*
	* Cuando se ingrea al handler de PendSV lo primero que se ejecuta es un push para
	* guardar los registros R4-R11 y el valor de LR, que en este punto es EXEC_RETURN
	* El push se hace al reves de como se escribe en la instruccion, por lo que LR
	* se guarda en la posicion 9 (luego del stack frame). Como la funcion getContextoSiguiente
	* se llama con un branch con link, el valor del LR es modificado guardando la direccion
	* de retorno una vez se complete la ejecucion de la funcion

	* El pasaje de argumentos a getContextoSiguiente se hace como especifica el AAPCS siendo
	* el unico argumento pasado por RO, y el valor de retorno tambien se almacena en R0
	*
	* NOTA: El primer ingreso a este handler (luego del reset) implica que el push se hace sobre el
	* stack inicial, ese stack se pierde porque no hay seguimiento del MSP en el primer ingreso
	*/


	cpsid i					// Deshabilita interrupciones globales
//=======================================================================================
	tst lr,0x10				// Si EXEC_RETURN[4]=0 se guarda S16-S31
	it eq
	vpusheq {s16-s31}

	push {r4-r11,lr}		// Salva los registros generales por si se usan
	mrs r0,msp				// Guardo el MSP

	bl getContextoSiguiente	// llama a la función del MSE_OS_Core.c

	msr msp,r0				// Recuoero el MSP
	pop {r4-r11,lr}			// Recuperados todos los valores de registros los registros
							// generales

	tst lr,0x10				// Si EXEC_RETURN[4]=0 se recupera S16-S31
	it eq
	vpopeq {s16-s31}
//=======================================================================================

	cpsie	i				// Habilito las interrupciones

	bx lr					// se hace un branch indirect con el valor de LR que es
							// nuevamente EXEC_RETURN

