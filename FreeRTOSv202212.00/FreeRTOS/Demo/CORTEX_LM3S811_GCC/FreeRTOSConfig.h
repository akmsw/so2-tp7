#ifndef FREERTOS_CONFIG_H
  #define FREERTOS_CONFIG_H
  #define configUSE_PREEMPTION		                  1
  #define configUSE_IDLE_HOOK			                  0
  #define configUSE_TICK_HOOK			                  0
  #define configCPU_CLOCK_HZ			                  ((unsigned long) 20000000)
  #define configTICK_RATE_HZ			                  ((TickType_t) 1000)
  #define configMINIMAL_STACK_SIZE	                ((unsigned short) 70
  #define configTOTAL_HEAP_SIZE		                  ((size_t) (5000))
  #define configMAX_TASK_NAME_LEN		                10
  #define configUSE_16_BIT_TICKS		                0
  #define configIDLE_SHOULD_YIELD		                0
  #define configMAX_PRIORITIES		                  5
  #define configKERNEL_INTERRUPT_PRIORITY 		      255
  #define configMAX_SYSCALL_INTERRUPT_PRIORITY      191
  #define configGENERATE_RUN_TIME_STATS             1
  #define configSUPPORT_DYNAMIC_ALLOCATION          1
  #define configUSE_TRACE_FACILITY                  1
  #define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()  (configurarTimer0())
  #define portGET_RUN_TIME_COUNTER_VALUE()          (obtenerValor())
  #define INCLUDE_vTaskPrioritySet		              0
  #define INCLUDE_uxTaskPriorityGet		              0
  #define INCLUDE_vTaskDelete				                0
  #define INCLUDE_vTaskCleanUpResources	            0
  #define INCLUDE_vTaskSuspend			                0
  #define INCLUDE_vTaskDelayUntil			              1
  #define INCLUDE_vTaskDelay				                1
  #define INCLUDE_uxTaskGetStackHighWaterMark       1
#endif