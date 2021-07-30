/*
 * MSE_OS_Core.c
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#include "MSE_OS_Core.h"

//======================= Es provisorio ================================================



//======================================================================================

/*==================[Deaclaración de funciones locales]================================*/
static void scheduler(void);


/*==================[Definición de variables globales]=================================*/

static osControl control_OS;

/*************************************************************************************************
	 *  @brief Inicializa el OS.
     *
     *  @details
     *   Inicializa el OS seteando la prioridad de PendSV como la mas baja posible. Es setear
     *   la prioridad de PendSV antes de que inicie el sistema.
     *   Se llama esta función luego de inicializar cada tareas.
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

	// Se configura las variables de estado del S.O.
	control_OS.estado_sistema=OS_FROM_RESET;
	control_OS.tarea_actual=NULL;
	control_OS.tarea_siguiente=NULL;

	/* En control_OS.listaTareas[] se tiene los punteros de cada tarea y en
	 * control_OS.cantidad_Tareas se tiene la cantidad de tareas instanciadas.
	 * Se conpleta el resto del vector con NULL
	 */
	for(uint8_t c=control_OS.cantidad_Tareas+1;c<MAX_TASK_COUNT;c++){
		control_OS.listaTareas[c]=NULL;
	}
}


/*************************************************************************************************
	 *  @brief Inicializa las tareas que correran en el OS.
     *
     *  @details
     *   Inicializa una tarea para que pueda correr en el OS implementado.
     *   Es necesario llamar a esta funcion para cada tarea antes que inicie el OS.
     *   Setea la PRIORIDAD de la tarea, los ticks_bloqueada=0, y estado = TAREA_READY
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
		task->ticks_bloqueada=0;

		/*
		 * Actualizacion de la estructura de control del OS, guardando el puntero a la estructura de tarea
		 * que se acaba de inicializar, y se actualiza la cantidad de tareas definidas en el sistema.
		 * Luego se incrementa el contador de id, dado que se le otorga un id correlativo a cada tarea
		 * inicializada, segun el orden en que se inicializan.
		 */
		control_OS.cantidad_Tareas=id;     		// Informo al S.O. la cantidad de tareas
		control_OS.listaTareas[id] = task;		// Se carga los punteros de cada tarea
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
	 *  @brief setPendSV Handler.
     *
     *  @details
     *  Dentro se setea como pendiente la excepcion PendSV.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void setPendSV(void)  {

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
	uint32_t sp_siguiente;

	/*
	 * Esta funcion efectua el cambio de contexto. Se guarda el MSP (sp_actual) en la variable
	 * correspondiente de la estructura de la tarea corriendo actualmente.
	 * Se carga en la variable sp_siguiente el stack pointer de la tarea siguiente, que fue
	 * definida por el scheduler. Se actualiza la misma a estado RUNNING
	 * y se retorna al handler de PendSV
	 */


	// Se guarda el contexto de la tarea actual y paso a TAREA_READY
	// Se supone que la tarea estaba en TAREA_RUNNING
	if(control_OS.estado_sistema==OS_FROM_RESET){
		sp_siguiente = control_OS.tarea_siguiente->stack_pointer;
		}
	else{
		control_OS.tarea_actual->stack_pointer = sp_actual;
		control_OS.tarea_actual->estado = TAREA_READY;
	}


	// Se cambia a la tarea siguiente
	sp_siguiente = control_OS.tarea_siguiente->stack_pointer;
	control_OS.tarea_actual = control_OS.tarea_siguiente;
	control_OS.tarea_actual->estado = TAREA_RUNNING;

	/*
	 * Indicamos que luego de retornar de esta funcion, ya no es necesario un cambio de contexto
	 * porque se acaba de gestionar.
	 */
	control_OS.estado_sistema = OS_NORMAL_RUN;

	return sp_siguiente;
}

/*************************************************************************************************
	 *  @brief SysTick Handler.
     *
     *  @details
     *   El handler del Systick no debe estar a la vista del usuario. En este handler se llama al
     *   scheduler y luego de determinarse cual es la tarea siguiente a ejecutar, se setea como
     *   pendiente la excepcion PendSV.
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
void SysTick_Handler(void)  {
//	uint8_t id_tarea;
//	tarea* task;		//variable para legibilidad

	/*
	 * Actualiza ticks_bloqueada de cada tare y si tienen un valor de ticks de bloqueo mayor
	 * a cero, se decrementan en una unidad. Si este contador llega a cero, entonces
	 * se debe pasar la tarea a READY.
	 */
//	id_tarea = 0;
//	while (control_OS.listaTareas[id_tarea] != NULL)  {
//		task = (tarea*)control_OS.listaTareas[id_tarea];
//
//		if( task->ticks_bloqueada > 0 )  {
//			if((--task->ticks_bloqueada == 0) && (task->estado == TAREA_BLOCKED))  {
//				task->estado = TAREA_READY;
//			}
//		}
//
//		id_tarea++;
//	}

	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */

	scheduler();

}

/*************************************************************************************************
	 *  @brief Funcion que efectua las decisiones de scheduling.
     *
     *  @details
     *   Segun el critero al momento de desarrollo, determina que tarea debe ejecutarse luego, y
     *   por lo tanto provee los punteros correspondientes para el cambio de contexto. Esta
     *   implementacion de scheduler es muy sencilla, del tipo Round-Robin
     *
	 *  @param 		None.
	 *  @return     None.
***************************************************************************************************/
static void scheduler(void)  {

	static uint8_t id_tarea;


	/* Si vengo de un RESET del S.O. se carga la primer tarea de la cola
	 * Por ahora hace de scheduler, sin tomar en cuenta las prioridades
	 * ni el estado de las tareas.
	 * Se supone que como mínimo hay dos tareas.
	 */
	if(control_OS.estado_sistema==OS_FROM_RESET){
		//control_OS.estado_sistema=OS_NORMAL_RUN;
		control_OS.tarea_siguiente=control_OS.listaTareas[id_tarea];
		id_tarea++;
		}
	else{
		if(id_tarea<control_OS.cantidad_Tareas+1){
			control_OS.tarea_siguiente=control_OS.listaTareas[id_tarea];
			id_tarea++;
			}
		else{
			id_tarea=0;
			control_OS.tarea_siguiente=control_OS.listaTareas[id_tarea];
			id_tarea++;
		}
	}
	// Por ahora el cambio de contexto siempre es necesario
	control_OS.cambioContextoNecesario=true;
	if(control_OS.cambioContextoNecesario)
		setPendSV();

}

//
