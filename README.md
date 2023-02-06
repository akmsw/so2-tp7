# Laboratorio N°7 - FreeRTOS

## Introduction
El presente es un trabajo práctico de laboratorio cuyo objetivo es el de desarrollar un programa para un sistema operativo de tiempo real ([FreeRTOS](https://www.freertos.org/)) que implemente distintas tareas involucradas en la simulación de un sensor de temperatura, el gráfico de los valores simulados y la colección de estadísticas de ejecución de las mismas.
## Startup
Para comenzar, se debe clonar el repositorio. Una vez hecho, podemos compilar todo el proyecto aprovechando el archivo makefile, simplemente corriendo el comando `make` dentro de la carpeta `so2-tp7/FreeRTOSv202212.00/FreeRTOS/Demo/CORTEX_LM3S811_GCC/`. Esto compilará el programa y creará los archivos necesarios para la ejecución.

Para ejecutar el programa, se utiliza [QEMU](https://www.qemu.org/) como herramienta de emulación de hardware mediante el comando:\
`qemu-system-arm -machine lm3s811evb -kernel gcc/RTOSDemo.axf`

En caso de querer debuggear, se deben agregar las flags `-s -S` al comando anterior para habilitar la depuración y agregar un breakpoint al comienzo del programa. Luego, con [GDB multiarch](https://en.wikipedia.org/wiki/GNU_Debugger) podemos abrir una nueva terminal dentro de la carpeta del proyecto y correr el comando:\
`gdb-multiarch gcc/RTOSDemo.axf`

Finalmente, para terminar de linkear GDB con el programa y poder debuggear, debemos ingresar el comando:\
`target remote localhost:1234`

## Development
Se desarrollaron las siguientes tareas:
- Simulación de un sensor de temperatura
- Cálculo del promedio de la temperatura mediante un filtro de ventana deslizante
- Gráfico en el tiempo de los valores promediados
- Recopilación de estadísticas de ejecución de las tareas

A continuación se detallará el desarrollo de cada tarea.

### Simulador de sensor de temperatura
Esta primera tarea genera valores aleatorios en base a una semilla, y con ello modifica el valor de temperatura actual.\
Para generar valores aleatorios se recurrió a una función extraída de un ejemplo para la placa SAMA5D3X. La misma está citada en la sección de [referencias](https://github.com/akmsw/so2-tp7#references). Con esta función se obtienen valores pseudo-aleatorios y el criterio de modificación de la temperatura fue:
- Si el número obtenido es divisible por 2, se incrementa la temperatura
- Caso contrario, se decrementa la temperatura

Para almacenar los valores generados, se creó una cola llamada `colaSensor`.\
Un requisito fue que esta tarea se ejecute con una frecuencia de 10[Hz]. Para esto, se definió un delay `mainCHECK_DELAY` haciendo uso de `portTICK_PERIOD_MS` para pasar el equivalente en milisegundos a delay en ticks. De esta manera, nos queda el delay definido de la forma:\
`#define mainCHECK_DELAY ((TickType_t) 100 / portTICK_PERIOD_MS)`

Finalmente, haciendo uso del [ejemplo de la documentación de FreeRTOS para ejecutar tareas cada cierto tiempo](https://freertos.org/vtaskdelayuntil.html), se hizo uso de la función `vTaskDelayUntil`.

La diferencia principal entre `vTaskDelay` y `vTaskDelayUntil` es que en la primera se indica cuánto tiempo debe pasar desde haber llamado a `vTaskDelay` para "despertar" a la tarea (delay relativo); por otro lado, en la segunda se indica el delay para "despertar" a la tarea de forma absoluta desde la última vez que la tarea se "despertó".
## Running

## Testing

## Screenshots

## Known issues

## References
- [Generador de números aleatorios](https://github.com/istarc/freertos/blob/master/FreeRTOS/Demo/CORTEX_A5_SAMA5D3x_Xplained_IAR/AtmelFiles/libboard_sama5d3x-ek/source/rand.c)
- [Delay until](https://freertos.org/vtaskdelayuntil.html)
- [Conversor binario a UTF-8](https://www.rapidtables.com/convert/number/binary-to-string.html)