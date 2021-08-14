
#ifndef ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_
#define ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_

#include <stdint.h>
#include "board.h"


/************************************************************************************
 * 			Tamaño del stack predefinido para cada tarea expresado en bytes
 ***********************************************************************************/

#define STACK_SIZE 256

//----------------------------------------------------------------------------------



/************************************************************************************
 * 	Posiciones dentro del stack de los registros que lo conforman
 ***********************************************************************************/

#define XPSR			1
#define PC_REG			2
#define LR				3
#define R12				4
#define R3				5
#define R2				6
#define R1				7
#define R0				8
#define LR_PREV_VALUE	9
#define R4				10
#define R5				11
#define R6				12
#define R7				13
#define R8				14
#define R9				15
#define R10 			16
#define R11 			17

//----------------------------------------------------------------------------------


/************************************************************************************
 * 			Valores necesarios para registros del stack frame inicial
 ***********************************************************************************/

#define INIT_XPSR 	1 << 24				//xPSR.T = 1
#define EXEC_RETURN	0xFFFFFFF9			//retornar a modo thread con MSP, FPU no utilizada

//----------------------------------------------------------------------------------


/************************************************************************************
 * 						Definiciones constantes del Sistema Operativo
 ***********************************************************************************/
#define STACK_FRAME_SIZE			8	//
#define FULL_STACKING_SIZE 			17	//16 core registers + valor previo de LR


#define MAX_TASK_COUNT				10	// Cantidad máxima de tareas para este OS
										// internamente se le suma una tarea más
										// la idleTask

#define MAX_PRIORITY				0	// Máxima prioridad que puede tener una tarea
#define MIN_PRIORITY				3	// Mínima prioridad que puede tener una tarea
#define PRIORITY_COUNT		(MIN_PRIORITY-MAX_PRIORITY)+1	//cantidad de prioridades asignables

#define TICKS_ON					0xFFFFFFFF	// Valor máximo de ticks de bloqueo
#define TICKS_OFF					0x00	    // Valor mínimo de ticks de bloqueo
/*==================[Definición codigos de error y warning de OS]=================================*/
#define ERR_OS_CANT_TAREAS		-1
#define ERR_DELAY_FROM_ISR		-2


/*==================[Definición de datos externa]================================================*/

/*==================[Definición de datos para el SiStema Operativo]==============================*/

/********************************************************************************
 * Definición de los estados posibles para las tareas
 *******************************************************************************/

enum _estadoTarea  {
	TAREA_READY,
	TAREA_RUNNING,
	TAREA_BLOCKED,
	TAREA_SUSPENDED
};

typedef enum _estadoTarea estadoTarea;


/********************************************************************************
 * Definición de las prioridades
 *******************************************************************************/

enum _prioridad  {
	PRIORIDAD_0,
	PRIORIDAD_1,
	PRIORIDAD_2,
	PRIORIDAD_3
};

typedef enum _prioridad prioridadTarea;


/********************************************************************************
 * Definición de la estructura de cada tarea
 *******************************************************************************/
struct _tarea  {
	uint32_t stack[STACK_SIZE/4];	// Longitud del Stack
	uint32_t stack_pointer;			// Puntero al Stack
	void *entry_point;				// Puntero al inicio de la tarea
	uint8_t id;						// Número que identifica la tarea
	estadoTarea estado;             // Estado de la tarea
	prioridadTarea prioridad;		// Prioridad de la tarea 0(mayor prioridad) al 3
	uint32_t ticks_bloqueada;		// cantidad de ticks que la tarea debe
									// permanecer bloqueada
};

typedef struct _tarea tarea;

/********************************************************************************
 * Definición de los estados posibles del OS
 *******************************************************************************/

enum _estadoOS  {
	OS_FROM_RESET,				// Inicio luego de un reset
	OS_NORMAL_RUN,				// Estado del sistema corriendo una tarea
	OS_SCHEDULING,				// El OS esta efectuando un scheduling
	OS_IRQ_RUN					// El OS esta corriendo un Handler

};

typedef enum _estadoOS estadoOS;

/********************************************************************************
 * Definición de la estructura de control para el Sistema Operativo
 *******************************************************************************/
struct _osControl  {
	void *listaTareas[MAX_TASK_COUNT+1];		//array de punteros a tareas + idleTask
	int32_t error;								//variable que contiene el ultimo error generado
	uint8_t cantidad_Tareas;					//cantidad de tareas definidas por el usuario
	estadoOS estado_sistema;					//Informacion sobre el estado del OS
	bool cambioContextoNecesario;
	tarea *tarea_actual;						//definicion de puntero para tarea actual
	tarea *tarea_siguiente;						//definicion de puntero para tarea siguiente
	uint8_t prioridadMin_Tarea;					//Prioridad mínima de las tarea definida por el usuario
	uint8_t prioridadMax_Tarea;					//Prioridad mínima de las tarea definida por el usuario

	bool banderaISR;						   //esta bandera se utiliza para la atencion a interrupciones
};

typedef struct _osControl osControl;


/*==================[definición de prototipos]=================================*/
void os_Init(void);				// Inicia el Sistema Operativo
void os_InitTarea(void *entryPoint, tarea *task, uint8_t prioridad);
void tareaDelay(uint32_t );

// Recupera el valor de reloj del sistema
uint64_t os_getSytemTicks(void);
// Setea una fución de Error.
void os_setError(int32_t err, void* caller);
// Setear una prioridad de una tarea ya creada
void os_setTareaPrioridad(tarea *task, uint8_t prioridad);
// Setear los ticks de bloqueo de una tarea
void os_setTicksTarea (tarea *task, uint32_t ticks_de_bloqueo);
// Setear los ticks de bloqueo de una tarea
void os_setTareaEstado(tarea *task, estadoTarea estado);
// Configuro  el estado del sistema.
void os_setEstadoSistema(estadoOS estado);

void os_setFlagISR(bool bandera);

// Recupera el último error del sistema.
int32_t os_getError(void);
// Recupera la cantidad de tareas totales del sistema.
int8_t os_getTareas(void);
// Recupera la tarea Actual
tarea* os_getTareaActual(void);
// Recupera la cantidad de tareas que se encuentran en un ESTADO con una
// PRIORIDAD determinada.
int8_t os_getTareasPrioridadEstado(uint8_t prioridadScan, estadoTarea estadoT);
// Recupera el estado del sistema.
int8_t os_getEstadoSistema(void);
// Recupera cual es la prioridad máxima del sistema.
int8_t os_getPrioridadMax(void);
// Recupera cual es la prioridad mínima del sistema, si tener encuenta a delayTarea.
int8_t os_getPrioridadMin(void);


bool os_getFlagISR(void);

// Fuerza un schedulering
void os_Yield(void);

// Para trabajar secciones críticas del código
void irqOn(void);
void irqOff(void);



#endif /* ISO_I_2020_MSE_OS_INC_MSE_OS_CORE_H_ */
