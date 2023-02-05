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
#define _MAX_TEMP_                (30)
#define _MIN_TEMP_                (15)
#define _COLS_DISPLAY_            (96)

/* Tareas */
static void vSensor(void *);
static void vCalcularPromedio(void *);
static void vGuardarPromedio(void *);
static void vDibujar(void *);

/* Funciones */
void iniciarDisplay(void);
void iniciarUART(void);
void dibujarEjes(void);
void actualizarArregloCircular(int[], int, int);
int numeroAleatorio(void);
int actualizarTamVentana(int);
int calcularPromedio(int[], int, int);
int atoi(char *);

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
    while (true);
  }

  iniciarUART();
  iniciarDisplay();
	xTaskCreate(vSensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vCalcularPromedio, "Calcular promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vGuardarPromedio, "Guardar promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vDibujar, "Dibujar", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
	vTaskStartScheduler();

	return 0;
}

static void vSensor(void *pvParameters) {
  TickType_t xTiempoAnteriorTarea = xTaskGetTickCount();

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

	while (true)	{
		vTaskDelayUntil(&xTiempoAnteriorTarea, mainCHECK_DELAY);

    if (numeroAleatorio() % 2 == 0) {
      if (temperaturaActual < _MAX_TEMP_) {
        temperaturaActual++;
      }
    } else {
      if (temperaturaActual > _MIN_TEMP_) {
        temperaturaActual--;
      }
    }

    if (xQueueSend(xColaSensor, &temperaturaActual, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
	}
}

static void vCalcularPromedio(void *pvParameters) {
  int valorCensado;
  int temperaturaPromedio;
  int tamVentana = 10;

  int arregloTemperatura[_MAX_N_] = {};

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

  while (true) {
    if (xQueueReceive(xColaSensor, &valorCensado, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    actualizarArregloCircular(arregloTemperatura, _MAX_N_, valorCensado);

    tamVentana = actualizarTamVentana(tamVentana);

    temperaturaPromedio = calcularPromedio(arregloTemperatura, _MAX_N_, tamVentana);

    if (xQueueSend(xColaPromedio, &temperaturaPromedio, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
  }
}

static void vGuardarPromedio(void *pvParameters) {
  int arregloPromedio[_MAX_N_] = {};

  int nuevoValor;

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

  while (true) {
    if (xQueueReceive(xColaPromedio, &nuevoValor, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    actualizarArregloCircular(arregloPromedio, _MAX_N_, nuevoValor);

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
  }
}

static void vDibujar(void *pvParameters) {
  dibujarEjes();
}

void iniciarDisplay(void) {
  OSRAMInit(false);
}

void iniciarUART(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void dibujarEjes(void) {
  // 3
  OSRAMImageDraw("", 1, 0, 1, 1);
  OSRAMImageDraw("", 2, 0, 1, 1);
  OSRAMImageDraw("", 3, 0, 1, 1);

  // 0 superior
  OSRAMImageDraw("", 5, 0, 1, 1);
  OSRAMImageDraw("", 6, 0, 1, 1);
  OSRAMImageDraw("", 7, 0, 1, 1);
  OSRAMImageDraw("", 8, 0, 1, 1);

  // 0 inferior
  OSRAMImageDraw("8", 5, 1, 1, 1);
  OSRAMImageDraw("D", 6, 1, 1, 1);
  OSRAMImageDraw("D", 7, 1, 1, 1);
  OSRAMImageDraw("8", 8, 1, 1, 1);

  // Eje Y
  OSRAMImageDraw("", 10, 0, 1, 1);
  OSRAMImageDraw("", 10, 1, 1, 1);

  // Eje X
  for (int i = 11; i < _COLS_DISPLAY_; i++) {
    OSRAMImageDraw("@", i, 1, 1, 1);
  }
}

void actualizarArregloCircular(int arreglo[], int tamArreglo, int nuevoValor) {
  for(int i = 0; i < tamArreglo - 1; i++) {
    arreglo[i] = arreglo[i + 1];
  }

  arreglo[tamArreglo - 1] = nuevoValor;
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

    tmp = atoi(nuevoTamVentana);

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
    while (true);
  }

  for (int i = 0; i < tamVentana; i++) {
    acumulador += arreglo[(tamArreglo - 1) - i];
  }

  return acumulador / tamVentana;
}

int atoi(char *cadena) {
  int numero = 0;

  char *c = cadena;

  while ((*c >= '0') && (*c <= '9')) {
    numero = numero * 10 + (*c++ - '0');
  }

  return numero;
}