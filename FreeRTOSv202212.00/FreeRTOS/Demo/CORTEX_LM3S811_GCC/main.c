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
static unsigned int temperaturaActual =24;
static unsigned int seed = 1;

int main(void) {
  xColaSensor = xQueueCreate(mainQUEUE_SIZE, sizeof(int));

	xTaskCreate(vSensorTemperatura, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
	vTaskStartScheduler();

	return 0;
}

static void vSensorTemperatura(void *pvParameters) {
  TickType_t xLastExecutionTime = xTaskGetTickCount();

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

	while (true)	{
		vTaskDelayUntil(&xLastExecutionTime, mainCHECK_DELAY);

    getRandomNumber() % 2 == 0 ? temperaturaActual++ : temperaturaActual--;

    xQueueSend(xColaSensor, &temperaturaActual, portMAX_DELAY);

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
	}
}

unsigned int getRandomNumber(void) {
  seed = seed * 1103515245 + 12345 ;

  return (uint32_t) (seed / 131072) % 65536 ;
}