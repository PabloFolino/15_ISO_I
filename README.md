# MSE_ISO_I
Materia:Implementación de Sistemas Operativos I de la Maestría en Sistemas Embebidos de la FIUBA

Autor: Pablo D. Folino

Profesor: Gonzalo Sanchez

Año: 2022

Se parte del commit  Nº4 del repositorio ttps://github.com/gonzaloesanchez/MSE_OS.git "Implementada getContextoSiguiente y PendSV_Handler"

# Requerimientos del Sistema Operativo

| Versión | Descripción | Cumplido |
| :-: | :-: | :-: |
[1.0] | 1. El sistema operativo (de aquí en más nombrado como OS) será del tipo estático.| ✔ |
[x.0] | 2. La cantidad de tareas que soportara el OS será ocho. | x |
[x.0] | 3. El OS debe administrar las IRQ del hardware.  | x |
[x.0] | 4. El kernel debe poseer una estructura de control la cual contenga como mínimo los siguientes campos(). |   |
[x.0] | 4.a. Último error ocurrido | x |
[x.0] | 4.b. Estado de sistema operativo, por ejemplo: Reset, corriendo normal, interrupción,etc. | x |
[x.0] | 4.c. Bandera que indique la necesidad de ejecutar un scheduling al salir de una IRQ.| x |
[x.0] | 4.d. Puntero a la tarea en ejecución.| x |
[x.0] | 4.e. Puntero a la siguiente tarea a ejecutar. | x |
[x.0] | 5. Cada tarea tendrá asociada una estructura de control que, como mínimo, tendrá los siguientes campos: |   |
[x.0] | 5.a. Stack (array). | x |
[x.0] | 5.b. Stack Pointer. | x |
[x.0] | 5.c. Punto de entrada (usualmente llamado ​ entryPoint ).| x |
[x.0] | 5.d. Estado de ejecución. | x |
[x.0] | 5.e. Prioridad. | x |
[x.0] | 5.f.Número de ID. | x |
[x.0] | 5.g. Ticks bloqueada.| x |
[x.0] | 6. Los estados de ejecución de una tarea serán los siguientes: |   |
[x.0] | 6.a. Corriendo (Running). | x |
[x.0] | 6.b. Lista para ejecución (Ready). | x |
[x.0] | 6.c. Bloqueada (Blocked).| x |
[x.0] | 6.d. Suspendida (Suspended) - ​ Opcional | x |
[x.0] | 7. El tamaño de stack para cada tarea será de 256 bytes. | ✔ |
[x.0] | 8. La implementación de prioridades será de 4 niveles, donde el nivel cero (0) será el de más alta prioridad y tres (3) será el nivel de menor prioridad. | x |
[x.0] | 9. La política de scheduling entre tareas de la misma prioridad será del tipo Round-Robin. | x |
[x.0] | 10. El tick del sistema será de 1 [ms].| ✔ |
[x.0] | 11. El OS debe tener ​ hooks ​ definidos como funciones ​ WEAK​ para la ejecución de código en las siguientes condiciones: |   |
[x.0] | 11.a. Tick del sistema (​ tickHook ). | x |
[x.0] | 11.b. Ejecución de código en segundo plano (​ taskIdle ). | x |
[x.0] | 11.c. Error y retorno de una de las tareas (​ returnHook ​ ).| x |
[x.0] | 11.d. Error del OS (​ errorHook ). | x |
[x.0] | 12. El OS debe poseer una API que contenga como mínimo las siguientes funciones:|   |
[x.0] | 12.a. Función de retardos (delay). | x |
[x.0] | 12.b. Semáforos binarios. | x |
[x.0] | 12.c. Colas (​ queue ).| x |
[x.0] | 12.d. Secciones críticas. | x |
[x.0] | 12.e. Forzado de Scheduling (​ cpu yield ). | x |

