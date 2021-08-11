# MSE_ISO_I
Materia:Implementación de Sistemas Operativos I de la Maestría en Sistemas Embebidos de la FIUBA

Autor: Pablo D. Folino

Profesor: Gonzalo Sanchez

Año: 2022

Se parte del commit  Nº4 del repositorio ttps://github.com/gonzaloesanchez/MSE_OS.git "Implementada getContextoSiguiente y PendSV_Handler"

Avances del último commit 11/08 :

	Se crean los archivos MSE_API.c y MSE_API.h.
	1) Se crean las funciones para manejar los semáforos binarios:
			a) os_SemaforoInit(sem); 		--> Inicializa valores
			b) return=statusSemTake os_SemaforoTake(sem,delayTicks);
			c) os_SemaforoGive(sem);
		
	MSE_OS_CORE.c y MSE_OS_CORE.h
	1) Se agregan las funciones:
			a) void os_Yield()	-- > fuerza un sheduling.
			b) irqOn(), irqOff()-- > manejo de secciones críticas.
			c) os_getError() 	--> recupera el último error del sistema.


Avances del commit 01/08 :
 
	main.c y main.h
	1) Se crea en tiempo de compilación las estructuras de las tareas.
	2) Mediante una función se completa dicha estructura, inicializando entre otros el Stack de la tarea.
	3) Se prueba el sistema con cuatro tareas que manejan leds, y algunas se suspenden usando delayTareas().
	
	MSE_OS_CORE.c y MSE_OS_CORE.h
	1) Se genera la estructura de control para cada tarea. Se implementa Prioridades y ticks de bloqueo.
	2) Las tareas pasan de READY, RUNNING y a BLOCKED.
	2) Se complementa la estructura del kernel del S.O.
	4) Se modifica el scheduler() para que detecte el estdo de RESET del sistema.
	5) Se genera la función de la función delayTarea().
	6) Para hacer el Round-Robin se crea una función específica.
	7) Se agrega idleTask.



# Requerimientos del Sistema Operativo

| Commit | Descripción | Cumplido |
| :-: | :-: | :-: |
[05/07] | 1. El sistema operativo (de aquí en más nombrado como OS) será del tipo estático.| ✔ |
[05/07] | 2. La cantidad de tareas que soportara el OS será ocho. | ✔ |
[x.0] | 3. El OS debe administrar las IRQ del hardware.  | x |
[x.0] | 4. El kernel debe poseer una estructura de control la cual contenga como mínimo los siguientes campos(). |   |
[09/08] | 4.a. Último error ocurrido | ✔ |
[31/07] | 4.b. Estado de sistema operativo, por ejemplo: Reset, corriendo normal, interrupción,etc. | ✔ |
[x.0] | 4.c. Bandera que indique la necesidad de ejecutar un scheduling al salir de una IRQ.| x |
[31/07] | 4.d. Puntero a la tarea en ejecución.| ✔ |
[31/07] | 4.e. Puntero a la siguiente tarea a ejecutar. | ✔ |
[x.0] | 5. Cada tarea tendrá asociada una estructura de control que, como mínimo, tendrá los siguientes campos: |   |
[28/07] | 5.a. Stack (array). | ✔ |
[28/07] | 5.b. Stack Pointer. | ✔ |
[28/07] | 5.c. Punto de entrada (usualmente llamado ​ entryPoint ).| ✔ |
[28/07] | 5.d. Estado de ejecución. | ✔ |
[28/07] | 5.e. Prioridad. | ✔ |
[28/07] | 5.f. Número de ID. | ✔ |
[28/07] | 5.g. Ticks bloqueada.| ✔ |
[x.0] | 6. Los estados de ejecución de una tarea serán los siguientes: |   |
[30/07] | 6.a. Corriendo (Running). | ✔ |
[30/07] | 6.b. Lista para ejecución (Ready). | ✔ |
[31/07] | 6.c. Bloqueada (Blocked).| ✔ |
[x.0] | 6.d. Suspendida (Suspended) - ​ Opcional | x |
[28/07] | 7. El tamaño de stack para cada tarea será de 256 bytes. | ✔ |
[31/07] | 8. La implementación de prioridades será de 4 niveles, donde el nivel cero (0) será el de más alta prioridad y tres (3) será el nivel de menor prioridad. | ✔ |
[31/07] | 9. La política de scheduling entre tareas de la misma prioridad será del tipo Round-Robin. | ✔ |
[05/07] | 10. El tick del sistema será de 1 [ms].| ✔ |
[x.0] | 11. El OS debe tener ​ hooks ​ definidos como funciones ​ WEAK​ para la ejecución de código en las siguientes condiciones: |   |
[31/07] | 11.a. Tick del sistema (​ tickHook ). | ✔ |
[01/08] | 11.b. Ejecución de código en segundo plano (​ taskIdle ). | ✔ |
[01/08] | 11.c. Error y retorno de una de las tareas (​ returnHook ​ ).| ✔ |
[01/08] | 11.d. Error del OS (​ errorHook ). | ✔ |
[x.0] | 12. El OS debe poseer una API que contenga como mínimo las siguientes funciones:|   |
[01/08] | 12.a. Función de retardos (delay). | ✔ |
[11/08] | 12.b. Semáforos binarios. | ✔ |
[x.0] | 12.c. Colas (​ queue ).| x |
[11/08] | 12.d. Secciones críticas. | ✔ |
[11/08] | 12.e. Forzado de Scheduling (​ cpu yield ). | ✔ |


El programa posee los siguientes módulos principales:

En el main.c:

![](/documento/diagrama_principal.png)


En el MSE_OS_Core.c y PendSV_Handler.S:

![](/documento/diagrama.png)


