#include "DriverLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainCHECK_DELAY						( ( TickType_t ) 100 / portTICK_PERIOD_MS ) // 10hz
#define mainBAUD_RATE				      ( 19200 )
#define mainFIFO_SET				      ( 0x10 )
#define mainCHECK_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainQUEUE_SIZE				    ( 3 )

/* Tareas */
static void vSensorTemperatura( void *pvParameters );

/* Variables globales */
static int temperaturaActual;

int main(void) {
  temperaturaActual = 0;

	xTaskCreate( vSensorTemperatura, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL );
	vTaskStartScheduler();

	return 0;
}

static void vSensorTemperatura(void *pvParameters) {
  TickType_t xLastExecutionTime = xTaskGetTickCount();

	while (true)	{
		vTaskDelayUntil( &xLastExecutionTime, mainCHECK_DELAY );

    temperaturaActual++;
	}
}