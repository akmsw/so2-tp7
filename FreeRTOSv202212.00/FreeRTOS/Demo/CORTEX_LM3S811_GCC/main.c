#include "DriverLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainCHECK_DELAY						((TickType_t) 100 / portTICK_PERIOD_MS) // 10hz
#define mainCHECK_TASK_PRIORITY		(tskIDLE_PRIORITY + 3)
#define mainQUEUE_SIZE				    (4)
#define _MAX_N_                   (20)

/* Tareas */
static void vSensor(void *);
static void vPromedio(void *);

/* Funciones */
void actualizarArregloCircular(int[], int, int);
int numeroAleatorio(void);
int calcularPromedio(int[], int, int);

/* Colas */
QueueHandle_t xColaSensor;
QueueHandle_t xColaPromedio;

/* Variables globales */
static int semilla = 1;
static int temperaturaActual = 24;
static int temperaturaPromedio;

int main(void) {
  xColaSensor = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
  xColaPromedio = xQueueCreate(mainQUEUE_SIZE, sizeof(int));

  if ((xColaSensor == NULL) || (xColaPromedio == NULL)) {
    for (;;);
  }

	xTaskCreate(vSensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vPromedio, "Promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
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

static void vPromedio(void *pvParameters) {
  int valorCensado;
  int tamVentana = 10;

  int arregloTemperatura[_MAX_N_];

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    for (;;);
  }

  for (;;) {
    if (xQueueReceive(xColaSensor, &valorCensado, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    actualizarArregloCircular(arregloTemperatura, _MAX_N_, valorCensado);

    temperaturaPromedio = calcularPromedio(arregloTemperatura, _MAX_N_, tamVentana);

    if (xQueueSend(xColaPromedio, &temperaturaPromedio, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

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