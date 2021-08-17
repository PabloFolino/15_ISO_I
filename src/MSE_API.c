/*====================================================================================
 * Author: Pablo Daniel Folino  <pfolino@gmail.com>
 * Date: 2021/08/14
 * Archivo: MSE_API.c
 * Version: 1
 *===================================================================================*/
/*Descripción:
 * En este módulo se encuentran las funciones de manejo de semáforos binarios y de
 * colas.
 *
 * Semáforos: son semáforos binarios, se los debe declarar la estructura del semáforo
 * en el main.c, usanto el tipo de datos "semaforo". Se los debe inicializar usando la
 * función os_SemaforoInit(), y se los toma y libera usando las funciones os_SemaforoTake()
 * y os_SemaforoGive(). Coando se inicializa el semáforo se encuentra TOMADO.
 * La función os_SemaforoTake() posee un parámetro de delayTicks)que especifica la
 * cantidad de ticks del sistema que espera para poder tomar el mismo. Esta funciún
 * devuelve pdFalse si no lo pudo tomar, o pdTrue en caso contrario.
 *
 * Colas : se las debe declarar en el main.c con el tipo de dato "cola". Es una cola
 * tipo FIFO de una longitud especificada por la constante LONG_COLA, definida en el
 * MSE_API.h. Siempre se extrae el primer elemento  de la cola(usando la función
 * os_ColaPop) es el que se encuentra en la posición cero. Si la cola se encuentra vacía
 * se bloquea la tarea hasta que aparezca un elemnto. Al extraer un elemento de la cola
 * se produce un shft de los datos de la misma  hacia el inicio de la cola y actualiza
 * la cantidad de elementos que posee.
 * Cada vez que se agrega un  elemento(usando la función os_ColaPush)se lo coloca en la
 * última posición, si la cola se encuentra se bloquea la tarea, hasta que se pueda
 *  agregar un nuevo dato.
 *  Esta cola está diseñada para que solamente una tarea ingrese datos en la misma,
 *  y otra única tarea comsuma datos.
 *
 *===================================================================================*/

#include "MSE_API.h"

/*************************************************************************************************
	 *  @brief Función que inicializa un semáforo binario
     *
     *  @details
     *  El semáforo se crea en etapa de complación mediante la creaión de una variable "semaforo"
     *  Esta función conigura los valores iniciales de esa variable.
     *
	 *  @param 		semaforo* sem,uint32_t delayTicks.
	 *  @return     bool.
***************************************************************************************************/
void os_SemaforoInit(semaforo* sem){
	sem->tareaSemaforo=NULL;
	sem->estadoSemaforo=TOMADO;
	sem->delaySemaforo=portMax_DELAY;
}


/*************************************************************************************************
	 *  @brief Función que toma un semáforo por un tiempo determinado
     *
     *  @details
     *  Cuando la tarea toma el semáforo, se pasa a estado TAREA_BLOCKED. Si el semáforo ya estaba
     *  bloqueado, sigue estando en estado bloqueado y se reescribe el delayTicks.
     *  Devuelve pdTrue si se pudo tomar correctamente, o pdFalse si durante delayTicks nadie lo
     *  liberó.
     *  El máximo de delayTicks es 65535
     *
	 *  @param 		semaforo* sem,uint32_t delayTicks.
	 *  @return     bool.
***************************************************************************************************/
statusSemTake os_SemaforoTake (semaforo* sem,uint64_t delayTicks) {
	static uint64_t ticks_finales;
	tarea * tareaActual;

	// Leo el valor del contador de ticks
	ticks_finales=portMax_DELAY;
	if(delayTicks!=portMax_DELAY)
		ticks_finales= os_getSytemTicks()+delayTicks;

	// Actualiza estado
	irqOff();
	tareaActual=os_getTareaActual();
	sem->tareaSemaforo=tareaActual;
	tareaActual->estado=TAREA_BLOCKED;
	tareaActual->ticks_bloqueada=TICKS_ON;	// Se hace para que SysTick_Handler no lo ponga en TAREA_READY
	sem->delaySemaforo=ticks_finales;
	sem->estadoSemaforo=TOMADO;
	irqOn();
	// Llama al scheduler
	os_Yield();

	while(os_getSytemTicks()<ticks_finales){
		tareaActual->ticks_bloqueada=TICKS_ON; 	// Se hace para que SysTick_Handler no lo ponga en TAREA_READY
		if(sem->estadoSemaforo==LIBERADO)
			sem->estadoSemaforo=TOMADO;
		return pdTrue;
		}
	return pdFalse;									// no fue liberado durante los delayTicks
}

/********************************************************************************
	 *  @brief Liberar un semaforo
     *
     *  @details
     *   Esta función se utiliza para liberar un semáforo.
     *
	 *  @param		sem		Semáforo a liberar
	 *  @return     None.
 *******************************************************************************/
void os_SemaforoGive(semaforo* sem)  {
	tarea * tareaLiberar;

	tareaLiberar=sem->tareaSemaforo;

	if (sem->estadoSemaforo == TOMADO && tareaLiberar!= NULL)  {
		irqOff();
		tareaLiberar->ticks_bloqueada=TICKS_OFF;
		tareaLiberar->estado=TAREA_READY;
		sem->estadoSemaforo = LIBERADO;
		irqOn();
	}
}





/********************************************************************************
	 *  @brief Inicializa una cola
     *
     *  @details
     *   Esta función se utiliza para inicializar una cola.
     *
	 *  @param		cola* buffer, uint8_t longDato
	 *  @return     None.
 *******************************************************************************/
void os_ColaInit(cola* buffer, uint16_t longDato){
	buffer->tareaIn=NULL;
	buffer->tareaOut=NULL;
	buffer->longElemento=longDato;
	buffer->contadorElementos=0;
	buffer->cantElementosMax=(uint16_t)(LONG_COLA/longDato);
	for(int i=0;i<LONG_COLA;i++)buffer->dato[i]=0;    // no tendía que ser necesario
}



/********************************************************************************
	 *  @brief Escribe un dato en la cola
     *
     *  @details
     *   Esta función se utiliza poner elementos en la cola. Si la cola está
     *   llena, bloquea la tareaIn con la función  os_setTareaEstado. Si se
     *   bloquea la tareaIn se llama al scheduler.
     *
	 *  @param		cola* buffer, dato
	 *  @return     None.
 *******************************************************************************/
void os_ColaPush(cola* buffer,void* dato){
	tarea *tareaAux;

	while(1){
		if(buffer->contadorElementos<buffer->cantElementosMax){
			// Como hay lugar
			memcpy(buffer->dato+buffer->contadorElementos,dato,buffer->longElemento);
			buffer->contadorElementos++;
			// Debo desploquear la tareaOut ya que se puso un elemento
			if(buffer->tareaOut!=NULL){
				void irqOff(void);
				tareaAux=buffer->tareaOut;
				os_setTareaEstado(tareaAux, TAREA_READY);
				buffer->tareaOut=NULL;
				void irqOn(void);
				}
			break;
			}
		else{
			// Si la cola está llena se debe bloquear la tarea, hasta que tenga lugar
			// El desbloquelo(TAREA_READY) lo hace os_ColaPop()
			void irqOff(void);
			tareaAux = os_getTareaActual();
			buffer->tareaIn=tareaAux;
			os_setTareaEstado(tareaAux, TAREA_BLOCKED);
			void irqOn(void);
			// Llama al scheduler
			os_Yield();

		}
	}
}


/********************************************************************************
	 *  @brief Inicializa una cola
     *
     *  @details
     *   Esta función se utiliza sacar elementos en la cola. Si la cola está
     *   vacía, bloquea la tareaOut con la función  os_setTareaEstado. Si se
     *   bloquea la tareaOut se llama al scheduler.
     *
	 *  @param		cola* buffer, uint8_t longDato
	 *  @return     None.
 *******************************************************************************/
void os_ColaPop(cola* buffer, void *dato){
	tarea *tareaAux;

	while(1){
		if(buffer->contadorElementos!=0){
			// Si la cola no esta vacía se saca un elemento
			memcpy(dato,buffer->dato,buffer->longElemento);

			// Corrimiento del buffer!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			for(uint16_t i=0;i<buffer->contadorElementos;i=i+buffer->longElemento){
				for(uint16_t c=0;c<buffer->longElemento;c++){
					buffer->dato[i+c]=buffer->dato[i+c+buffer->longElemento];
					}
				}
			buffer->contadorElementos--;
			// Debo desploquear la tareaIn ya que se sacó un elemento
			if(buffer->tareaIn!=NULL){
				void irqOff(void);
				tareaAux=buffer->tareaIn;
				os_setTareaEstado(tareaAux, TAREA_READY);
				buffer->tareaIn=NULL;
				void irqOn(void);
				}
			break;
			}
		else{
			// Si la cola no tiene datos debo bloquear la tarea
			void irqOff(void);
			tareaAux = os_getTareaActual();
			buffer->tareaOut = tareaAux;
			os_setTareaEstado(tareaAux, TAREA_BLOCKED);
			void irqOn(void);
			// Llama al scheduler
			os_Yield();
		}
	}
}


