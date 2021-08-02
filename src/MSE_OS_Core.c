/*
 * MSE_OS_Core.c
 *
 *  Created on: 26 mar. 2020
 *      Author: gonza
 */

#include "MSE_OS_Core.h"

//======================= Es provisorio ================================================

//======================================================================================

/*===================[Declaración de funciones locales]================================*/

static void scheduler(void);
void setPendSV(void);
uint32_t getContextoSiguiente(uint32_t sp_actual);
void SysTick_Handler(void);
uint8_t busqueda(uint8_t prioridadScan, estadoTarea estadoT);
void __attribute__((weak)) idleTask(void);

void __attribute__((weak)) returnHook(void);
void __attribute__((weak)) tickHook(void);
void __attribute__((weak)) errorHook(void *caller);



/*==================[Definición de variables globales]=================================*/

static osControl control_OS;
static tarea tareaIdle;

/*==================[Funciones del Sistema Operativo]=================================*/

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

	/*
	 * Se inicializa una tarea Idle, la cual no es visible al usuario. Esta tarea se agrega a
	 * control_OS.listaTareas[] como una tarea más, con la prioridad más baja definida por la
	 * constante PRIORITY_COUNT.
	 * Como esta tarea debe estar siempre presente y el usuario no la inicializa.
	 *
	 */
	os_InitTarea(idleTask, &tareaIdle,PRIORITY_COUNT);

	/*
	 * En control_OS.listaTareas[] se tiene los punteros de cada tarea y en
	 * control_OS.cantidad_Tareas se tiene la cantidad de tareas instanciadas.
	 * Se conpleta el resto del vector con NULL.
	 *
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

	if(control_OS.cantidad_Tareas < MAX_TASK_COUNT+1)  {			// Llegue al máximo de tareas se toma
																	// en cuenta el lugar para la tarea
																	// idelTask

		task->stack[STACK_SIZE/4 - XPSR] = INIT_XPSR;				//necesario para bit thumb
		task->stack[STACK_SIZE/4 - PC_REG] = (uint32_t)entryPoint;	//direccion de la tarea (ENTRY_POINT)
		task->stack[STACK_SIZE/4 - LR] = (uint32_t)returnHook;		//Retorno de la tarea (no deberia darse)

		/*
		 * El valor previo de LR (que es EXEC_RETURN en este caso) es necesario dado que
		 * en esta implementacion, se llama a una funcion desde dentro del handler de PendSV
		 * con lo que el valor de LR se modifica por la direccion de retorno para cuando
		 * se termina de ejecutar getContextoSiguiente
		 */
		task->stack[STACK_SIZE/4 - LR_PREV_VALUE] = EXEC_RETURN;

		task->stack_pointer = (uint32_t) (task->stack + STACK_SIZE/4 - FULL_STACKING_SIZE);

		/*
		 * Si es la primera vez se configura prioridadMin_Tarea y prioridadMax_Tarea con un valor inicial
		 * y luego se actualiza a medida que van ingresando las tareas.
		 */
		if (id==0){
			control_OS.prioridadMin_Tarea=prioridad;
			control_OS.prioridadMax_Tarea=prioridad;
			}
		else{
			if(prioridad>control_OS.prioridadMin_Tarea && entryPoint!=idleTask) control_OS.prioridadMin_Tarea=prioridad;
			if(prioridad<control_OS.prioridadMax_Tarea && entryPoint!=idleTask) control_OS.prioridadMax_Tarea=prioridad;
		}

		/*
		* En esta seccion se guarda el entry point de la tarea, se le asigna id a la misma y se pone
		* la misma en estado READY. Todas las tareas se crean en estado READY.
		* Se asigna la prioridad de la misma.
		*/
		task->entry_point = entryPoint;
		task->estado = TAREA_READY;
		task->ticks_bloqueada=0;
		control_OS.listaTareas[id] = task;		// Se carga los punteros de cada tarea

		if(entryPoint==idleTask){
			task->id  = control_OS.cantidad_Tareas;
			task->prioridad = PRIORITY_COUNT;
			}
		else{
			task->id = id;
			task->prioridad = prioridad;
			/*
			 * Actualizacion de la estructura de control del OS, guardando el puntero a la estructura de tarea
			 * que se acaba de inicializar, y se actualiza la cantidad de tareas definidas en el sistema.
			 * Luego se incrementa el contador de id, dado que se le otorga un id correlativo a cada tarea
			 * inicializada, segun el orden en que se inicializan.
			 */
			id++;
			control_OS.cantidad_Tareas=id;     		// Informo al S.O. la cantidad de tareas
		}
	}

	else {
		/*
		 * En el caso que se hayan excedido la cantidad de tareas que se pueden definir, se actualiza
		 * el ultimo error generado en la estructura de control del OS y se llama a errorHook y se
		 * envia informacion de quien es quien la invoca.
		 */
		os_setError(ERR_OS_CANT_TAREAS,os_InitTarea);
	}
}


/*************************************************************************************************
	 *  @brief Levanta un error de sistema.
     *
     *  @details
     *   En aras de mantener la estructura de control aislada solo en el archivo de core esta
     *   funcion proporciona una forma de setear el codigo de error y lanza una ejecucion
     *   de errorHook
     *
	 *  @param 		err		El codigo de error que ha surgido
	 *  @param		caller	Puntero a la funcion que llama a esta funcion
	 *  @return     none.
***************************************************************************************************/
void os_setError(int32_t err, void* caller)  {
	control_OS.error = err;
	errorHook(caller);
}

/*================[Funciones internas del Sistema Operativo]==========================*/

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
	 *  @brief getContextoSiguiente Funcion para determinar el próximo contexto.
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

	control_OS.tarea_actual->stack_pointer = sp_actual;

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
	tarea *task_aux;		//variable auxiliar

	/*
	 * Actualiza ticks_bloqueada de cada tarea y si tienen un valor de ticks de bloqueo mayor
	 * a cero, se decrementan en una unidad. Si este contador llega a cero, entonces
	 * se debe pasar la tarea a READY.
	 */

	for(uint8_t id_tarea=0;id_tarea<control_OS.cantidad_Tareas+1;id_tarea++) {
		task_aux =(tarea*)control_OS.listaTareas[id_tarea];
		if (  task_aux->ticks_bloqueada != 0 ) {
			task_aux->ticks_bloqueada--;
			task_aux->estado=TAREA_BLOCKED;
			}
		else{
			task_aux->estado=TAREA_READY;
	    }
	}

	/*
	 * Dentro del SysTick handler se llama al scheduler. Separar el scheduler de
	 * getContextoSiguiente da libertad para cambiar la politica de scheduling en cualquier
	 * estadio de desarrollo del OS. Recordar que scheduler() debe ser lo mas corto posible
	 */

	scheduler();

	/*
	 * Luego de determinar cual es la tarea siguiente segun el scheduler, se ejecuta la funcion
	 * tickhook.
	 */

	tickHook();

}

/*************************************************************************************************
	 *  @brief busqueda de prioridad y estado de las tareas.
     *
     *  @details
     *  Devuelve cuantas tareas hay en una prioridad, con un determinada estado
     *
	 *  @param 		(uint8_t prioridadScan, estadoTarea estadoT).
	 *  @return     cantidad.
***************************************************************************************************/
uint8_t busqueda(uint8_t prioridadScan, estadoTarea estadoT) {
	tarea *task_aux;		//variable auxiliar
	uint8_t cantidad=0;

	for(uint8_t id_tarea=0;id_tarea<control_OS.cantidad_Tareas+1;id_tarea++) {
		task_aux =(tarea*)control_OS.listaTareas[id_tarea];
		if(task_aux->prioridad==prioridadScan && task_aux->estado==estadoT){
			cantidad++;
		};
	};
	return cantidad;
}

/*************************************************************************************************
	 *  @brief roundRobin.
     *
     *  @details
     *  Realiza La política de scheduling Round-Robin entre tareas de la misma prioridad.
     *  Devuelve la próxima id_tarea a ejecutarse.
     *
	 *  @param 		(prioridadTarea scanPrioridad,uint8_t id_tarea).
	 *  @return     id_tarea.
***************************************************************************************************/
uint8_t roundRobin(prioridadTarea scanPrioridad,uint8_t id_tarea) {
	static prioridadTarea scanPrioridad_old=0;
	tarea *task_aux;				//variable auxiliar

	if(scanPrioridad!=scanPrioridad_old || id_tarea>=control_OS.cantidad_Tareas+1){
		id_tarea=0;
	}
	while(true){
		task_aux =(tarea*)control_OS.listaTareas[id_tarea];
		if(task_aux->prioridad==scanPrioridad && task_aux->estado==TAREA_READY){
			control_OS.tarea_siguiente=control_OS.listaTareas[id_tarea];
			id_tarea++;
			break;
			}
		else{
			id_tarea++;
			if(id_tarea>=control_OS.cantidad_Tareas+1) id_tarea=0;
		}
	}

	scanPrioridad_old=scanPrioridad;
	return id_tarea;
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
	static uint8_t id_tarea=0;
	prioridadTarea	scanPrioridad=PRIORIDAD_0;


	/*
	 * Si se viene del reset se pone a la tarea idle como tarea actual
	 */
	if (control_OS.estado_sistema == OS_FROM_RESET)  {
		control_OS.tarea_actual = control_OS.listaTareas[control_OS.cantidad_Tareas];
		control_OS.estado_sistema = OS_NORMAL_RUN;
	}


	/*
	 * Puede darse el caso en que se haya invocado la funcion os_CpuYield() la cual hace una
	 * llamada al scheduler. Si durante la ejecucion del scheduler (la cual fue forzada) y
	 * esta siendo atendida en modo Thread ocurre una excepcion dada por el SysTick, habra una
	 * instancia del scheduler pendiente en modo trhead y otra corriendo en modo Handler invocada
	 * por el SysTick. Para evitar un doble scheduling, se controla que el sistema no este haciendo
	 * uno ya. En caso afirmativo se vuelve prematuramente
	 */
	if (control_OS.estado_sistema == OS_SCHEDULING)  {
		return;
	}

	/*
	 * Cambia el estado del sistema para que no se produzcan schedulings anidados cuando
	 * existen forzados por alguna API del sistema.
	 */
	control_OS.estado_sistema = OS_SCHEDULING;


	for(scanPrioridad=0;scanPrioridad<=PRIORITY_COUNT;scanPrioridad++){
		if(busqueda(scanPrioridad, TAREA_READY)){
			id_tarea=roundRobin(scanPrioridad,id_tarea);
			break;
		}
	}

	// Por ahora el cambio de contexto siempre es necesario
	control_OS.cambioContextoNecesario=true;
	if(control_OS.cambioContextoNecesario)
		setPendSV();

}

/*************************************************************************************************
	 *  @brief Función que setea el campo ticks_bloqueada de una tarea
     *
     *  @details
     *   Cuando ticks_bloqueada es distinto de cero la tarea se encuantra en estado BLOCKED.
     *   En la rutina del SysTick cada vez que se entra decrementa en uno el valor del campo
     *   ticks_bloqueada.
     *
	 *  @param 		cantidad de ticks.
	 *  @return     None.
***************************************************************************************************/
void tareaDelay(uint32_t cuentas) {
	// __asm("cpsid i");
	if (cuentas!=0){
		control_OS.tarea_siguiente->ticks_bloqueada=cuentas;
	}
	//__asm("cpsie i");
}



/*************************************************************************************************
	 *  @brief Tarea Idle (segundo plano)
     *
     *  @details
     *   Esta tarea se ejecuta solamente cuando todas las demas tareas estan en estado bloqueado.
     *   Puede ser redefinida por el usuario.
     *
	 *  @param none
	 *
	 *  @return none.
	 *
	 *  @warning		No debe utilizarse ninguna funcion API del OS dentro de esta funcion. No
	 *  				debe ser causa de un re-scheduling.
***************************************************************************************************/
void __attribute__((weak)) idleTask(void)  {
	while(1)  {
		__WFI();
	}
}

/*==================[Definición de hooks débiles]=================================*/

/*
 * Esta seccion contiene los hooks de sistema, los cuales el usuario del OS puede
 * redefinir dentro de su codigo y poblarlos segun necesidad
 */


/*************************************************************************************************
	 *  @brief Hook de retorno de tareas
     *
     *  @details
     *   Esta funcion no deberia accederse bajo ningun concepto, porque ninguna tarea del OS
     *   debe retornar. Si lo hace, es un comportamiento anormal y debe ser tratado.
     *
	 *  @param none
	 *
	 *  @return none.
***************************************************************************************************/
void __attribute__((weak)) returnHook(void)  {
	while(1);
}


/*************************************************************************************************
	 *  @brief Hook de tick de sistema
     *
     *  @details
     *   Se ejecuta cada vez que se produce un tick de sistema. Es llamada desde el handler de
     *   SysTick.
     *
	 *  @param none
	 *
	 *  @return none.
	 *
	 *  @warning	Esta funcion debe ser lo mas corta posible porque se ejecuta dentro del handler
     *   			mencionado, por lo que tiene prioridad sobre el cambio de contexto y otras IRQ.
	 *
	 *  @warning 	Esta funcion no debe bajo ninguna circunstancia utilizar APIs del OS dado
	 *  			que podria dar lugar a un nuevo scheduling.
***************************************************************************************************/
void __attribute__((weak)) tickHook(void)  {
	__asm volatile( "nop" );
}


/*************************************************************************************************
	 *  @brief Hook de error de sistema
     *
     *  @details
     *   Esta funcion es llamada en caso de error del sistema, y puede ser utilizada a fin de hacer
     *   debug. El puntero de la funcion que llama a errorHook es pasado como parametro para tener
     *   informacion de quien la esta llamando, y dentro de ella puede verse el codigo de error
     *   en la estructura de control de sistema. Si ha de implementarse por el usuario para manejo
     *   de errores, es importante tener en cuenta que la estructura de control solo esta disponible
     *   dentro de este archivo.
     *
	 *  @param caller		Puntero a la funcion donde fue llamado errorHook. Implementado solo a
	 *  					fines de trazabilidad de errores
	 *
	 *  @return none.
***************************************************************************************************/
void __attribute__((weak)) errorHook(void *caller)  {
	/*
	 * Revisar el contenido de control_OS.error para obtener informacion. Utilizar os_getError()
	 */
	while(1);
}







//
