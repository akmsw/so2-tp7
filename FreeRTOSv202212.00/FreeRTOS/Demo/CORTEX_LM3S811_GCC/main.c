/* Environment includes */
#include "DriverLib.h"

/* Scheduler includes */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Project-specific definitions */
#define mainCHECK_TASK_PRIORITY (tskIDLE_PRIORITY + 3)
#define mainQUEUE_SIZE (5)
#define _SENSOR_DELAY_ ((TickType_t) 100 / portTICK_PERIOD_MS) // 10[Hz]
#define _MAX_N_ 20
#define _MAX_TEMP_ 30
#define _MIN_TEMP_ 15

/* Project-specific functions */
static void vCircularArrayPush(int[], int, int);
static int dCalculateAverage(int[], int, int);
uint32_t getRandomNumber(void);

/* Project-specific tasks */
static void vGenerateAverage(void *);
static void vTemperatureSensor(void *);

/* Project-specific global variables */
static unsigned int currentTemperature = 24;
static uint32_t nextRand = 1;

/* Project-specific queues */
QueueHandle_t xSensorQueue;
QueueHandle_t xAverageQueue;

/* Main body */
int main(void) {
  xSensorQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
  xAverageQueue = xQueueCreate(mainQUEUE_SIZE, sizeof(int));

	xTaskCreate(vTemperatureSensor, "Temperature sensor simulator", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY + 1, NULL);
  xTaskCreate(vGenerateAverage, "Average temperature calculator", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
	vTaskStartScheduler();

	return 0;
}

/* Tasks bodies */
static void vTemperatureSensor(void *pvParameters) {
	TickType_t xLastExecutionTime;

  xLastExecutionTime = xTaskGetTickCount();

  UBaseType_t uxHighWaterMark;

  uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

  if(uxHighWaterMark < 1) {
    while (true);
  }

  while (true) {
    uint32_t randomNumber = getRandomNumber();

    if (randomNumber % 2 == 0) {
      if (currentTemperature < _MAX_TEMP_) {
        currentTemperature++;
      }
    } else {
      if (currentTemperature > _MIN_TEMP_) {
        currentTemperature--;
      }
    }

    xQueueSend(xSensorQueue, &currentTemperature, 0);

    uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);

    if(uxHighWaterMark < 1) {
      while (true);
    }

    vTaskDelayUntil(&xLastExecutionTime, _SENSOR_DELAY_);
  }
}

static void vGenerateAverage(void *pvParameters) {
  int temperatureArray[_MAX_N_] = {};
  int average;
  int valueToAdd;
  int windowSize = 10;

  while (true) {
    xQueueReceive(xSensorQueue, &valueToAdd, 0);

    vCircularArrayPush(temperatureArray, _MAX_N_, valueToAdd);

    average = dCalculateAverage(temperatureArray, _MAX_N_, windowSize);

    xQueueSend(xAverageQueue, &average, 0);
  }
}

/* Functions bodies */
uint32_t getRandomNumber(void) {
  nextRand = nextRand * 1103515245 + 12345 ;

  return (uint32_t) (nextRand / 131072) % 65536 ;
}

static void vCircularArrayPush(int array[], int arraySize, int valueToAdd) {
  for (int i = 0; i < arraySize - 1; i++) {
    array[i] = array[i + 1];
  }

  array[arraySize - 1] = valueToAdd;
}

static int dCalculateAverage(int array[], int arraySize, int windowSize) {
  int average = 0;

  for (int i = 0; i < windowSize; i++) {
    average += array[(arraySize - 1) - i];
  }

  average /= windowSize;

  return average;
}