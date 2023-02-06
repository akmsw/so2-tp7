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

## Running

## Testing

## Screenshots

## Known issues

## References
- [Generador de números aleatorios](https://github.com/istarc/freertos/blob/master/FreeRTOS/Demo/CORTEX_A5_SAMA5D3x_Xplained_IAR/AtmelFiles/libboard_sama5d3x-ek/source/rand.c)
- [Delay until](https://freertos.org/vtaskdelayuntil.html)
- [Conversor binario a UTF-8](https://www.rapidtables.com/convert/number/binary-to-string.html)