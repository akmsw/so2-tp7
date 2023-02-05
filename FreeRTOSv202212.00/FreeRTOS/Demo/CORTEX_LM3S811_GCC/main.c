#include <stdlib.h>

#include "DriverLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainCHECK_DELAY						((TickType_t) 100 / portTICK_PERIOD_MS) // 10hz
#define mainCHECK_TASK_PRIORITY		(tskIDLE_PRIORITY + 3)
#define mainQUEUE_SIZE				    (4)
#define mainBAUD_RATE		          (19200)
#define _MAX_N_                   (20)
#define _MAX_TAM_VENTANA          (10)

/* Tareas */
static void vSensor(void *);
static void vCalcularPromedio(void *);
static void vGuardarPromedio(void *);

/* Funciones */
void actualizarArregloCircular(int[], int, int);
void iniciarUART(void);
int numeroAleatorio(void);
int actualizarTamVentana(int);
int calcularPromedio(int[], int, int);

/* Colas */
QueueHandle_t xColaSensor;
QueueHandle_t xColaPromedio;

/* Variables globales */
static int semilla = 1;
static int temperaturaActual = 24;

int main(void) {
  xColaSensor = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
  xColaPromedio = xQueueCreate(mainQUEUE_SIZE, sizeof(int));

  if ((xColaSensor == NULL) || (xColaPromedio == NULL)) {
    for (;;);
  }

  iniciarUART();
	xTaskCreate(vSensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vCalcularPromedio, "Calcular promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vGuardarPromedio, "Guardar promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
	vTaskStartScheduler();

	return 0;
}

static void vSensor(void *pvParameters) {
  TickType_t xTiempoAnteriorTarea = xTaskGetTickCount();

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    for (;;);
  }

	for (;;)	{
		vTaskDelayUntil(&xTiempoAnteriorTarea, mainCHECK_DELAY);

    numeroAleatorio() % 2 == 0 ? temperaturaActual++ : temperaturaActual--;

    if (xQueueSend(xColaSensor, &temperaturaActual, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      for (;;);
    }
	}
}

static void vCalcularPromedio(void *pvParameters) {
  int valorCensado;
  int temperaturaPromedio;
  int tamVentana = 10;

  int arregloTemperatura[_MAX_N_] = {};

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    for (;;);
  }

  for (;;) {
    if (xQueueReceive(xColaSensor, &valorCensado, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    actualizarArregloCircular(arregloTemperatura, _MAX_N_, valorCensado);

    tamVentana = actualizarTamVentana(tamVentana);

    temperaturaPromedio = calcularPromedio(arregloTemperatura, _MAX_N_, tamVentana);

    if (xQueueSend(xColaPromedio, &temperaturaPromedio, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      for (;;);
    }
  }
}

static void vGuardarPromedio(void *pvParameters) {
  int arregloPromedio[_MAX_N_] = {};

  int nuevoValor;

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    for (;;);
  }

  for (;;) {
    if (xQueueReceive(xColaPromedio, &nuevoValor, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    actualizarArregloCircular(arregloPromedio, _MAX_N_, nuevoValor);

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      for (;;);
    }
  }
}

void actualizarArregloCircular(int arreglo[], int tamArreglo, int nuevoValor) {
  for(int i = 0; i < tamArreglo - 1; i++) {
    arreglo[i] = arreglo[i + 1];
  }

  arreglo[tamArreglo - 1] = nuevoValor;
}

void iniciarUART(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

int actualizarTamVentana(int tamVentana) {
  if (UARTCharsAvail(UART0_BASE)) {
    int i = 0;
    int tmp;

    char lectura;

    char nuevoTamVentana[2];

    while ((lectura = (char) UARTCharNonBlockingGet(UART0_BASE)) != -1) {
      nuevoTamVentana[i] = lectura;

      if (i == 1) {
        break;
      }

      i++;
    }

    nuevoTamVentana[i] = '\0';

    tmp = strtol(nuevoTamVentana, NULL, 10);

    if (tmp > 1 && tmp < _MAX_TAM_VENTANA) {
      tamVentana = tmp;
    }
  }

  return tamVentana;
}

int numeroAleatorio(void) {
  semilla = semilla * 1103515245 + 12345;

  return (semilla / 131072) % 65536;
}

int calcularPromedio(int arreglo[], int tamArreglo, int tamVentana) {
  int acumulador = 0;

  if (tamVentana > tamArreglo) {
    for (;;);
  }

  for (int i = 0; i < tamVentana; i++) {
    acumulador += arreglo[(tamArreglo - 1) - i];
  }

  return acumulador / tamVentana;
}