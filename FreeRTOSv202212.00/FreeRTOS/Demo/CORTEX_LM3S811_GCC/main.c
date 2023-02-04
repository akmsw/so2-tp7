#include "DriverLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainCHECK_DELAY						((TickType_t) 100 / portTICK_PERIOD_MS) // 10hz
#define mainCHECK_TASK_PRIORITY		(tskIDLE_PRIORITY + 3)
#define mainQUEUE_SIZE				    (3)

/* Tareas */
static void vSensorTemperatura(void *);

/* Funciones */
unsigned int getRandomNumber(void);

/* Colas */
QueueHandle_t xColaSensor;

/* Variables globales */
static unsigned int temperaturaActual = 24;
static unsigned int semilla = 1;

int main(void) {
  xColaSensor = xQueueCreate(mainQUEUE_SIZE, sizeof(unsigned int));

  if (xColaSensor == NULL) {
    for (;;);
  }

	xTaskCreate(vSensorTemperatura, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
	vTaskStartScheduler();

	return 0;
}

static void vSensorTemperatura(void *pvParameters) {
  TickType_t xTiempoAnteriorTarea = xTaskGetTickCount();

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    for (;;);
  }

	for (;;)	{
		vTaskDelayUntil(&xTiempoAnteriorTarea, mainCHECK_DELAY);

    getRandomNumber() % 2 == 0 ? temperaturaActual++ : temperaturaActual--;

    if (xQueueSend(xColaSensor, &temperaturaActual, portMAX_DELAY) != pdTRUE) {
      for (;;);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      for (;;);
    }
	}
}

unsigned int getRandomNumber(void) {
  semilla = semilla * 1103515245 + 12345;

  return (semilla / 131072) % 65536;
}