#include "DriverLib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#define mainCHECK_DELAY				    ((TickType_t) 100 / portTICK_PERIOD_MS) // 10hz
#define mainCHECK_TASK_PRIORITY   (tskIDLE_PRIORITY + 3)
#define mainQUEUE_SIZE				    4
#define mainBAUD_RATE		          9600
#define _MAX_N_                   20
#define _MAX_TAM_VENTANA          10
#define _MAX_TEMP_                30
#define _MIN_TEMP_                15
#define _COLS_DISPLAY_            87
#define _STATS_DELAY_             ((TickType_t) 1000 / portTICK_PERIOD_MS)

/* Tareas */
static void vSensor(void *);
static void vCalcularPromedio(void *);
static void vDibujar(void *);
static void vStats(void *);

/* Funciones */
void iniciarDisplay(void);
void iniciarUART(void);
void dibujarEjes(void);
void configurarTimer(void);
void Timer0IntHandler(void);
void imprimirEstadisticas(void);
void enviarCadenaUART0(const char *);
void actualizarArregloCircular(int[], int, int);
int numeroAleatorio(void);
int calcularPromedio(int[], int, int);
int convertirCadenaAEntero(char *);
unsigned long obtenerValor(void);
char* obtenerCaracterEquivalente(int);
char* utoa(unsigned, char *, int);

/* Colas */
QueueHandle_t colaSensor;
QueueHandle_t colaPromedio;
QueueHandle_t colaUART;

/* Variables globales */
static int semilla;
static int temperaturaActual;
static int tamVentana;

unsigned long ulHighFrequencyTimerTicks;

TaskStatus_t *pxTaskStatusArray;

int main(void) {
  colaSensor = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
  colaPromedio = xQueueCreate(mainQUEUE_SIZE, sizeof(int));
  colaUART = xQueueCreate(mainQUEUE_SIZE, sizeof(char *));

  if ((colaSensor == NULL) || (colaPromedio == NULL) || (colaUART == NULL)) {
    while (true);
  }

  temperaturaActual = 24;
  tamVentana = 10;
  semilla = 1;

  iniciarUART();
  iniciarDisplay();
	xTaskCreate(vSensor, "Sensor", configSENSOR_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY + 1, NULL);
  xTaskCreate(vCalcularPromedio, "Prom.", configAVG_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY, NULL);
  xTaskCreate(vDibujar, "Dibujar", configDIBUJAR_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL);
  xTaskCreate(vStats, "Stats", configSTATS_STACK_SIZE, NULL, mainCHECK_TASK_PRIORITY - 1, NULL);
	vTaskStartScheduler();

	return 0;
}

static void vSensor(void *pvParameters) {
  TickType_t tiempoAnteriorTarea = xTaskGetTickCount();

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

	while (true)	{
		vTaskDelayUntil(&tiempoAnteriorTarea, mainCHECK_DELAY);

    if (numeroAleatorio() % 2 == 0) {
      if (temperaturaActual < _MAX_TEMP_) {
        temperaturaActual++;
      }
    } else {
      if (temperaturaActual > _MIN_TEMP_) {
        temperaturaActual--;
      }
    }

    if (xQueueSend(colaSensor, &temperaturaActual, portMAX_DELAY) != pdTRUE) {
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

  int arregloTemperatura[_MAX_N_] = {};

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

  while (true) {
    if (xQueueReceive(colaSensor, &valorCensado, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    actualizarArregloCircular(arregloTemperatura, _MAX_N_, valorCensado);

    temperaturaPromedio = calcularPromedio(arregloTemperatura, _MAX_N_, tamVentana);

    if (xQueueSend(colaPromedio, &temperaturaPromedio, portMAX_DELAY) != pdTRUE) {
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
    if (xQueueReceive(colaPromedio, &lectura, portMAX_DELAY) != pdTRUE) {
      while (true);
    }

    actualizarArregloCircular(arregloPromedio, _COLS_DISPLAY_, lectura);
    OSRAMClear();
    dibujarEjes();

    for (int i = 0; i < _COLS_DISPLAY_; i++) {
      OSRAMImageDraw(obtenerCaracterEquivalente(arregloPromedio[i]), (i + 11), (arregloPromedio[i] > 21 ? 0 : 1), 1, 1);
    }

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }
  }
}

static void vStats(void *pvParameters) {
  UBaseType_t uxArraySize = uxTaskGetNumberOfTasks();

  pxTaskStatusArray = pvPortMalloc(uxArraySize * sizeof(TaskStatus_t));

  if (uxTaskGetStackHighWaterMark(NULL) < 1) {
    while (true);
  }

  while (true) {
    imprimirEstadisticas();

    if (uxTaskGetStackHighWaterMark(NULL) < 1) {
      while (true);
    }

    vTaskDelay(_STATS_DELAY_);
  }
}

void iniciarDisplay(void) {
  OSRAMInit(false);
}

void iniciarUART(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
  UARTConfigSet(UART0_BASE, mainBAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));
  UARTIntEnable(UART0_BASE, UART_INT_RX);
	IntPrioritySet(INT_UART0, configKERNEL_INTERRUPT_PRIORITY);
	IntEnable(INT_UART0);
}

void vUART_ISR(void) {
	int rxNum;

  unsigned long ulStatus;

	char* rxChar;

	ulStatus = UARTIntStatus(UART0_BASE, pdTRUE);

	UARTIntClear(UART0_BASE, ulStatus);

	if (ulStatus & UART_INT_RX) {
		rxNum = UARTCharGet(UART0_BASE) - 48;

		xQueueSend(colaUART, &rxNum, portMAX_DELAY);
	}

  if (xQueueReceive(colaUART, &tamVentana, 0) != pdTRUE) {
    while (true);
  }
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

void configurarTimer0(void) {
  SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
  IntMasterEnable();
  TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
  TimerConfigure(TIMER0_BASE, TIMER_CFG_32_BIT_TIMER);
  TimerLoadSet(TIMER0_BASE, TIMER_A, 1500);
  TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0IntHandler);
  TimerEnable(TIMER0_BASE, TIMER_A);
}

void Timer0IntHandler(void) {
  TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

  ulHighFrequencyTimerTicks++;
}

void imprimirEstadisticas(void) {
  volatile UBaseType_t uxArraySize;
  volatile UBaseType_t x;

  unsigned long ulTotalRunTime;
  unsigned long ulStatsAsPercentage;

  if (pxTaskStatusArray != NULL) {
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);

    ulTotalRunTime /= 100UL;

    enviarCadenaUART0("\r");

    if (ulTotalRunTime > 0) {
      enviarCadenaUART0("TAREA\t|TICKS\t|CPU%\t|STACK FREE\r\n");

      for (x = 0; x < uxArraySize; x++) {
        ulStatsAsPercentage = pxTaskStatusArray[x].ulRunTimeCounter / ulTotalRunTime;

        char counter[8];
        char porcentaje[8];
        char stack[8];

        enviarCadenaUART0(pxTaskStatusArray[x].pcTaskName);
        enviarCadenaUART0("\t|");
        utoa(pxTaskStatusArray[x].ulRunTimeCounter, counter, 10);
        enviarCadenaUART0(counter);
        enviarCadenaUART0("\t|");

        if (ulStatsAsPercentage > 0UL) {
          utoa(ulStatsAsPercentage, porcentaje, 10);
          enviarCadenaUART0(porcentaje);
        } else {
          enviarCadenaUART0("0");
        }

        enviarCadenaUART0("%\t|");
        utoa(pxTaskStatusArray[x].usStackHighWaterMark, stack, 10);
        enviarCadenaUART0(stack);
        enviarCadenaUART0(" Words\r\n");
      }

      enviarCadenaUART0("\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n");
    }
  }
}

void enviarCadenaUART0(const char *cadena) {
  while(*cadena != '\0') {
    UARTCharPut(UART0_BASE, *cadena++);
  }

  UARTCharPut(UART0_BASE, '\0');
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
    while (true);
  }

  for (int i = 0; i < tamVentana; i++) {
    acumulador += arreglo[(tamArreglo - 1) - i];
  }

  return acumulador / tamVentana;
}

int convertirCadenaAEntero(char *cadena) {
  int numero = 0;

  char *c = cadena;

  while ((*c >= '0') && (*c <= '9')) {
    numero = numero * 10 + (*c++ - '0');
  }

  return numero;
}

unsigned long obtenerValor(void) {
  return ulHighFrequencyTimerTicks;
}

char* obtenerCaracterEquivalente(int valor) {
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

char *utoa(unsigned valor, char *dest, int base) {
  const char digitos[] = "0123456789abcdefghijklmnopqrstuvwxyz";

  int i;
  int j;

  unsigned resto;

  char c;

  if ((base < 2) || (base > 36)) {
    dest[0] = '\0';
    return NULL;
  }

  i = 0;

  do {
    resto = valor % base;

    dest[i++] = digitos[resto];

    valor = valor / base;
  } while (valor != 0);

  dest[i] = '\0';

  for (j = 0, i--; j < i; j++, i--) {
    c = dest[j];

    dest[j] = dest[i];
    dest[i] = c;
  }

  return dest;
}