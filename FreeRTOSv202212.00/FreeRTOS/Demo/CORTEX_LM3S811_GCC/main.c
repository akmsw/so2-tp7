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
#define _COLS_DISPLAY_            (87)

/* Tareas */
static void vSensor(void *);
static void vCalcularPromedio(void *);
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
char* obtenerEquivalenteCaracter(int);

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
	xTaskCreate(vSensor, "Sensor", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY + 1, NULL);
  xTaskCreate(vCalcularPromedio, "Calcular promedio", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vDibujar, "Dibujar", configMINIMAL_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL);
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

static void vDibujar(void *pvParameters) {
  int arregloPromedio[_COLS_DISPLAY_] = {};

  for (int i = 0 ; i < _COLS_DISPLAY_; i++) {
    arregloPromedio[i] = _MIN_TEMP_;
  }

  int lectura;

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

  while (true) {
    if (xQueueReceive(xColaPromedio, &lectura, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    actualizarArregloCircular(arregloPromedio, _COLS_DISPLAY_, lectura);
    OSRAMClear();
    dibujarEjes();

    for (int i = 0; i < _COLS_DISPLAY_; i++) {
      OSRAMImageDraw(obtenerEquivalenteCaracter(arregloPromedio[i]), (i + 11), (arregloPromedio[i] > 21 ? 0 : 1), 1, 1);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
  }
}

void iniciarDisplay(void) {
  OSRAMInit(false);
}

void iniciarUART(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
}

void dibujarEjes(void) {
  // 30 superior
  OSRAMImageDraw("", 1, 0, 3, 1);  // 3
  OSRAMImageDraw("", 5, 0, 4, 1); // 0

  // 15 inferior
  OSRAMImageDraw("H|@", 1, 1, 3, 1);  // 1
  OSRAMImageDraw("\\Tt", 5, 1, 3, 1); // 5

  // Eje Y
  OSRAMImageDraw("", 10, 0, 1, 1);
  OSRAMImageDraw("", 10, 1, 1, 1);

  // Eje X
  for (int i = 11; i < _COLS_DISPLAY_ + 11; i++) {
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

char* obtenerEquivalenteCaracter(int valor) {
  if ((valor <= 15) || (valor == 22)) {
    return "@";
  }

  if (valor == 16) {
    return "`";
  }

  if (valor == 17) {
    return "P";
  }

  if (valor == 18) {
    return "H";
  }

  if (valor == 19) {
    return "D";
  }

  if (valor == 20) {
    return "B";
  }

  if (valor == 21) {
    return "A";
  }

  if (valor == 23) {
    return " ";
  }

  if (valor == 24) {
    return "";
  }

  if (valor == 25) {
    return "";
  }

  if (valor == 26) {
    return "";
  }

  if ((valor == 27) || (valor == 28)) {
    return "";
  }

  if (valor >= 29) {
    return "";
  }
}