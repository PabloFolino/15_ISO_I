/*
 * MSE_OS_Core.c
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#include "MSE_OS_Core.h"

//======================= Es provisorio ================================================
extern tarea estadoTarea1,estadoTarea2;
//======================================================================================

/*==================[Definición de variables globales]=================================*/

static osControl control_OS;

/*************************************************************************************************
	 *  @brief Inicializa las tareas que correran en el OS.
     *
     *  @details
     *   Inicializa una tarea para que pueda correr en el OS implementado.
     *   Es necesario llamar a esta funcion para cada tarea antes que inicie
     *   el OS.
     *
	 *  @param *tarea			Puntero a la tarea que se desea inicializar.
	 *  @param *stack			Puntero al espacio reservado como stack para la tarea.
	 *  @param *stack_pointer   Puntero a la variable que almacena el stack pointer de la tarea.
	 *  @return     None.
***************************************************************************************************/
void os_InitTarea(void *entryPoint, tarea *task, prioridadTarea prioridad)  {
	static uint8_t id = 0;				//el id sera correlativo a medida que se generen mas tareas

	/*
	 * Al principio se efectua un pequeño checkeo para determinar si llegamos a la cantidad maxima de
	 * tareas que pueden definirse para este OS. En el caso de que se traten de inicializar mas tareas
	 * que el numero maximo soportado, se guarda un codigo de error en la estructura de control del OS
	 * y la tarea no se inicializa. La tarea idle debe ser exceptuada del conteo de cantidad maxima
	 * de tareas
	 */

	if(control_OS.cantidad_Tareas < MAX_TASK_COUNT)  {				// Llegue al máximo de tareas

		task->stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;				//necesario para bit thumb
		task->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;	//direccion de la tarea (ENTRY_POINT)
//		task->stack[STACK_SIZE/4 - LR] = (uint32_t)returnHook;		//Retorno de la tarea (no deberia darse)

		/*
		 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
		 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
		 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
		 * se termina de ejecutar getContextoSiguiente
		 */
		task->stack[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;

		task->stack_pointer = (uint32_t) (task->stack + STACK_SIZE/4 - FULL_STACKING_SIZE);

		/*
		 * En esta seccion se guarda el entry point de la tarea, se le asigna id a la misma y se pone
		 * la misma en estado READY. Todas las tareas se crean en estado READY.
		 * Se asigna la prioridad de la misma.
		 */
		task->entry_point = entryPoint;
		task->id = id;
		task->estado = TAREA_READY;
		task->prioridad = prioridad;

		/*
		 * Actualizacion de la estructura de control del OS, guardando el puntero a la estructura de tarea
		 * que se acaba de inicializar, y se actualiza la cantidad de tareas definidas en el sistema.
		 * Luego se incrementa el contador de id, dado que se le otorga un id correlativo a cada tarea
		 * inicializada, segun el orden en que se inicializan.
		 */
//		control_OS.listaTareas[id] = task;
//		control_OS.cantidad_Tareas++;
//		control_OS.cantTareas_prioridad[prioridad]++;

		id++;
	}

	else {
		/*
		 * En el caso que se hayan excedido la cantidad de tareas que se pueden definir, se actualiza
		 * el ultimo error generado en la estructura de control del OS y se llama a errorHook y se
		 * envia informacion de quien es quien la invoca.
		 */
//		os_setError(ERR_OS_CANT_TAREAS,os_InitTarea);
	}
}









/*************************************************************************************************
	 *  @brief Inicializa el OS.
     *
     *  @details
     *   Inicializa el OS seteando la prioridad de PendSV como la mas baja posible. Es necesario
     *   llamar esta funcion antes de que inicie el sistema. Es recomendable llamarla luego de
     *   inicializar las tareas
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void os_Init(void)  {
	/*
	 * Todas las interrupciones tienen prioridad 0 (la máxima) al iniciar la ejecución. Para que
	 * no se de la condicion de fault mencionada en la teoria, debemos bajar su prioridad en el
	 * NVIC. La cuenta matematica que se observa da la probabilidad mas baja posible.
	 */
	NVIC_SetPriority(PendSV_IRQn, (1 << __NVIC_PRIO_BITS)-1);
}


/*************************************************************************************************
	 *  @brief SysTick Handler.
     *
     *  @details
     *   El handler del Systick no debe estar a la vista del usuario. Dentro se setea como
     *   pendiente la excepcion PendSV.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void SysTick_Handler(void)  {

	/**
	 * Se setea el bit correspondiente a la excepción PendSV
	 * Habilita para que pueda ser llamada la PendSV
	 */
	SCB->ICSR = SCB_ICSR_PENDSVSET_Msk;

	/**
	 * Instruction Synchronization Barrier; flushes the pipeline and ensures that
	 * all previous instructions are completed before executing new instructions
	 */
	__ISB();

	/**
	 * Data Synchronization Barrier; ensures that all memory accesses are
	 * completed before next instruction is executed
	 */
	__DSB();
}




/*************************************************************************************************
	 *  @brief Funcion para determinar el proximo contexto.
     *
     *  @details
     *   Esta funcion en este momento hace las veces de scheduler y tambien obtiene el siguiente
     *   contexto a ser cargado. El cambio de contexto se ejecuta en el handler de PendSV, dentro
     *   del cual se llama a esta funcion
     *
	 *  @param 		sp_actual	Este valor es una copia del contenido de MSP al momento en
	 *  			que la funcion es invocada.
	 *  @return     El valor a cargar en MSP para apuntar al contexto de la tarea siguiente.
***************************************************************************************************/
uint32_t getContextoSiguiente(uint32_t sp_actual)  {
	static int32_t tarea_actual = -1;			/* Es una variable local y no se pierde */
	uint32_t sp_siguiente;

	/**
	 * Este bloque switch-case hace las veces de scheduler. Es el mismo codigo que
	 * estaba anteriormente implementado en el Systick handler
	 */
	switch(tarea_actual)  {

	/**
	 * Tarea actual es tarea1. Recuperamos el stack pointer (MSP) y lo
	 * almacenamos en sp_tarea1. Luego cargamos en la variable de retorno
	 * sp_siguiente el valor del stack pointer de la tarea2
	 */
	case 1:
//		control_OS.tarea_actual->stack_pointer = sp_actual;
//		sp_siguiente = control_OS.tarea_siguiente->stack_pointer;
		estadoTarea1.stack_pointer=sp_actual;
		sp_siguiente =estadoTarea2.stack_pointer;
		tarea_actual = 2;
		break;

	/**
	 * Tarea actual es tarea2. Recuperamos el stack pointer (MSP) y lo
	 * almacenamos en sp_tarea2. Luego cargamos en la variable de retorno
	 * sp_siguiente el valor del stack pointer de la tarea1
	 */
	case 2:
//		control_OS.tarea_actual->stack_pointer = sp_actual;
//		sp_siguiente = control_OS.tarea_siguiente->stack_pointer;
		estadoTarea2.stack_pointer=sp_actual;
		sp_siguiente =estadoTarea1.stack_pointer;
		tarea_actual = 1;
		break;

	/**
	 * Este es el caso del inicio del sistema, donde no se ha llegado aun a la
	 * primer ejecucion de tarea1. Por lo que se cargan los valores correspondientes
	 */

	default:
		sp_siguiente = estadoTarea1.stack_pointer;
		//control_OS.tarea_actual->stack_pointer = sp_actual;
		tarea_actual = 1;
		break;
	}

	return sp_siguiente;
}




