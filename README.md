# Laboratorio N°7 - FreeRTOS
## Introduction
El presente es un trabajo práctico de laboratorio cuyo objetivo es el de desarrollar un programa para un sistema operativo de tiempo real ([FreeRTOS](https://www.freertos.org/)) que implemente distintas tareas involucradas en la simulación de un sensor de temperatura, el gráfico de los valores simulados y la colección de estadísticas de ejecución de las mismas.
## Startup
Para comenzar, se debe clonar el repositorio. Una vez hecho, podemos compilar todo el proyecto aprovechando el archivo makefile, simplemente corriendo el comando `make` dentro de la carpeta `so2-tp7/FreeRTOSv202212.00/FreeRTOS/Demo/CORTEX_LM3S811_GCC/`. Esto compilará el programa y creará los archivos necesarios para la ejecución.
## Development
Se desarrollaron las siguientes tareas:
- Simulación de un sensor de temperatura
- Cálculo del promedio de la temperatura mediante un filtro de ventana deslizante
- Gráfico en el tiempo de los valores promediados
- Recopilación de estadísticas de ejecución de las tareas mostrándolas por puerto serie

Además, se debe permitir el ingreso del tamaño de ventana utilizado para el cálculo del promedio via UART.

A continuación se detallará el desarrollo de cada tarea.

### 🟢 Simulador de sensor de temperatura
Esta primera tarea genera valores aleatorios en base a una semilla, y con ello modifica el valor de temperatura actual.\
Para generar valores aleatorios se recurrió a una función extraída de un ejemplo para la placa SAMA5D3X. La misma está citada en la sección de [referencias](https://github.com/akmsw/so2-tp7#references). Con esta función se obtienen valores pseudo-aleatorios y el criterio de modificación de la temperatura fue:
- Si el número obtenido es divisible por 2, se incrementa la temperatura
- Caso contrario, se decrementa la temperatura

Se trabajó con un rango limitado de 15[°C] a 30[°C].
Para almacenar los valores generados, se creó una cola llamada `colaSensor`.\
Un requisito fue que esta tarea se ejecute con una frecuencia de 10[Hz]. Para esto, se definió un delay `mainCHECK_DELAY` haciendo uso de `portTICK_PERIOD_MS` para pasar el equivalente en milisegundos a delay en ticks. De esta manera, nos queda el delay definido de la forma:\
`#define mainCHECK_DELAY ((TickType_t) 100 / portTICK_PERIOD_MS)`\
Finalmente, haciendo uso del [ejemplo de la documentación de FreeRTOS para ejecutar tareas cada cierto tiempo](https://freertos.org/vtaskdelayuntil.html), se hizo uso de la función `vTaskDelayUntil`.\
La diferencia principal entre `vTaskDelay` y `vTaskDelayUntil` es que en la primera se indica cuánto tiempo debe pasar desde haber llamado a `vTaskDelay` para "despertar" a la tarea (delay relativo); por otro lado, en la segunda se indica el delay para "despertar" a la tarea de forma absoluta desde la última vez que la tarea se "despertó".
### 🟢 Calculador de promedio
Esta tarea toma los valores generados por la tarea anterior guardados en la cola `colaSensor` y los almacena en un arreglo circular. Luego, en base al tamaño de la ventana que se esté usando, efectúa el cálculo del promedio.\
Cada valor promedio generado se almacena en una cola llamada `colaPromedio` que es usada más adelante.
### 🟢 Gráfico
Para la tarea que traza el gráfico de los promedios de las mediciones en el tiempo, se hizo uso de la función `OSRAMImageDraw`, que recibe un arreglo de caracteres en formato UTF-8 y los grafica en el display.\
La disposición del display es de 16 filas por 96 columnas, dividido en dos renglones de 8x96.\
Primero se dibujan los ejes y los rangos a abarcar, luego, se grafica en el eje X las mediciones almacenadas del arreglo circular. Para esto,se hizo un mapeo de cada temperatura del rango establecido con los píxeles a pintar en la gráfica representados como números binaros de 8 bits y se obtuvieron sus caracteres equivalentes en formato UTF-8 mediante el siguiente [conversor online](https://www.rapidtables.com/convert/number/binary-to-string.html).
### 🟢 Recopilación de estadísticas
Para esta tarea, al no poder utilizar varias ed las funciones de librerías estándares (por ejemplo, funciones como `utoa`), se optó por tomar la implementación de la función [vTaskGetRunTimeStats](https://www.freertos.org/a00021.html#vTaskGetRunTimeStats) y adaptarla para utilizar funciones equivalentes que no estén en esas librerías.\
Estas estadísticas se recopilan en una estructura que almacena datos útiles sobre las tareas, como el porcentaje de CPU utilizado y la cantidad de stack libre que le queda.
## Running
Para ejecutar el programa, se utiliza [QEMU](https://www.qemu.org/) como herramienta de emulación de hardware mediante el comando:\
`qemu-system-arm -machine lm3s811evb -kernel gcc/RTOSDemo.axf`\
En caso de querer debuggear, se deben agregar las flags `-s` y `-S` al comando anterior para habilitar la depuración y agregar un breakpoint al comienzo del programa. Luego, con [GDB multiarch](https://en.wikipedia.org/wiki/GNU_Debugger) podemos abrir una nueva terminal dentro de la carpeta del proyecto y correr el comando:\
`gdb-multiarch gcc/RTOSDemo.axf`\
Finalmente, para terminar de linkear GDB con el programa y poder debuggear, debemos ingresar el comando:\
`target remote localhost:1234`
## Screenshots
## References
- [Generador de números aleatorios](https://github.com/istarc/freertos/blob/master/FreeRTOS/Demo/CORTEX_A5_SAMA5D3x_Xplained_IAR/AtmelFiles/libboard_sama5d3x-ek/source/rand.c)
- [Delay until](https://freertos.org/vtaskdelayuntil.html)
- [Conversor binario a UTF-8](https://www.rapidtables.com/convert/number/binary-to-string.html)