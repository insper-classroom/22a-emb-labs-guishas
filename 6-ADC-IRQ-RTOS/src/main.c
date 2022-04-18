#include "conf_board.h"
#include <asf.h>

/************************************************************************/
/* BOARD CONFIG                                                         */
/************************************************************************/

#define USART_COM_ID ID_USART1
#define USART_COM USART1

#define AFEC_POT AFEC0
#define AFEC_POT_ID ID_AFEC0
#define AFEC_POT_CHANNEL 0 // Canal do pino PC31

/************************************************************************/
/* RTOS                                                                */
/************************************************************************/

#define TASK_ADC_STACK_SIZE (1024*10 / sizeof(portSTACK_TYPE))
#define TASK_ADC_STACK_PRIORITY (tskIDLE_PRIORITY)

#define TASK_PROC_STACK_SIZE (1024*10 / sizeof(portSTACK_TYPE))
#define TASK_PROC_STACK_PRIORITY (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

/************************************************************************/
/* recursos RTOS                                                        */
/************************************************************************/

/** Queue for msg log send data */
QueueHandle_t xQueueADC;
QueueHandle_t xQueueProc;

typedef struct {
  uint value;
} adcData;

/************************************************************************/
/* prototypes local                                                     */
/************************************************************************/

static void USART1_init(void);
void TC_init(Tc *TC, int ID_TC, int TC_CHANNEL, int freq);
static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel, afec_callback_t callback);
static void configure_console(void);

/************************************************************************/
/* RTOS application funcs                                               */
/************************************************************************/

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,
                                          signed char *pcTaskName) {
  printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
  /* If the parameters have been corrupted then inspect pxCurrentTCB to
   * identify which task has overflowed its stack.
   */
  for (;;) {
  }
}

extern void vApplicationIdleHook(void) { pmc_sleep(SAM_PM_SMODE_SLEEP_WFI); }

extern void vApplicationTickHook(void) {}

extern void vApplicationMallocFailedHook(void) {
  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

  /* Force an assert. */
  configASSERT((volatile void *)NULL);
}

/************************************************************************/
/* handlers / callbacks                                                 */
/************************************************************************/

void TC1_Handler(void) {
  volatile uint32_t ul_dummy;

  ul_dummy = tc_get_status(TC0, 1);

  /* Avoid compiler warning */
  UNUSED(ul_dummy);

  /* Selecina canal e inicializa convers�o */
  afec_channel_enable(AFEC_POT, AFEC_POT_CHANNEL);
  afec_start_software_conversion(AFEC_POT);
}

static void AFEC_pot_Callback(void) {
  adcData adc;
  adc.value = afec_channel_get_value(AFEC_POT, AFEC_POT_CHANNEL);
  BaseType_t xHigherPriorityTaskWoken = pdTRUE;
  xQueueSendFromISR(xQueueProc, &adc, &xHigherPriorityTaskWoken);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_adc(void *pvParameters) {

  // configura ADC e TC para controlar a leitura
  config_AFEC_pot(AFEC_POT, AFEC_POT_ID, AFEC_POT_CHANNEL, AFEC_pot_Callback);
  TC_init(TC0, ID_TC1, 1, 10);
  tc_start(TC0, 1);

  // vari�vel para recever dados da fila
  adcData adc;

  while (1) {
    if (xQueueReceive(xQueueADC, &(adc), 1000)) {
      printf("ADC: %d \n", adc);
    } else {
      printf("Nao chegou um novo dado em 1 segundo");
    }
  }
}

static void task_proc(void *pvParameters) {
	adcData adcD;
	int soma = 0;
	int contador = 0;
	
	while (1) {
		if (xQueueReceive(xQueueProc, &(adcD), 1000)) {
			if (contador == 10) {
				BaseType_t xHigherPriorityTaskWoken = pdTRUE;
				adcData data = {.value = soma};
				xQueueSendFromISR(xQueueADC, &adcD, &xHigherPriorityTaskWoken);
				soma = 0;
				contador = 0;
			} else {
				soma += adcD.value;
				contador++;
			}
		} else {
			printf("Nao chegou um novo dado em 1 segundo");
		}
	}
}

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

/**
 * \brief Configure the console UART.
 */
static void configure_console(void) {
  const usart_serial_options_t uart_serial_options = {
      .baudrate = CONF_UART_BAUDRATE,
      .charlength = CONF_UART_CHAR_LENGTH,
      .paritytype = CONF_UART_PARITY,
      .stopbits = CONF_UART_STOP_BITS,
  };

  /* Configure console UART. */
  stdio_serial_init(CONF_UART, &uart_serial_options);

  /* Specify that stdout should not be buffered. */
  setbuf(stdout, NULL);
}

static void config_AFEC_pot(Afec *afec, uint32_t afec_id, uint32_t afec_channel,
                            afec_callback_t callback) {
  /*************************************
   * Ativa e configura AFEC
   *************************************/
  /* Ativa AFEC - 0 */
  afec_enable(afec);

  /* struct de configuracao do AFEC */
  struct afec_config afec_cfg;

  /* Carrega parametros padrao */
  afec_get_config_defaults(&afec_cfg);

  /* Configura AFEC */
  afec_init(afec, &afec_cfg);

  /* Configura trigger por software */
  afec_set_trigger(afec, AFEC_TRIG_SW);

  /*** Configuracao espec�fica do canal AFEC ***/
  struct afec_ch_config afec_ch_cfg;
  afec_ch_get_config_defaults(&afec_ch_cfg);
  afec_ch_cfg.gain = AFEC_GAINVALUE_0;
  afec_ch_set_config(afec, afec_channel, &afec_ch_cfg);

  /*
  * Calibracao:
  * Because the internal ADC offset is 0x200, it should cancel it and shift
  down to 0.
  */
  afec_channel_set_analog_offset(afec, afec_channel, 0x200);

  /***  Configura sensor de temperatura ***/
  struct afec_temp_sensor_config afec_temp_sensor_cfg;

  afec_temp_sensor_get_config_defaults(&afec_temp_sensor_cfg);
  afec_temp_sensor_set_config(afec, &afec_temp_sensor_cfg);

  /* configura IRQ */
  afec_set_callback(afec, afec_channel, callback, 1);
  NVIC_SetPriority(afec_id, 4);
  NVIC_EnableIRQ(afec_id);
}

void TC_init(Tc *TC, int ID_TC, int TC_CHANNEL, int freq) {
  uint32_t ul_div;
  uint32_t ul_tcclks;
  uint32_t ul_sysclk = sysclk_get_cpu_hz();

  pmc_enable_periph_clk(ID_TC);

  tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
  tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
  tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

  NVIC_SetPriority((IRQn_Type)ID_TC, 4);
  NVIC_EnableIRQ((IRQn_Type)ID_TC);
  tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/

/**
 *  \brief FreeRTOS Real Time Kernel example entry point.
 *
 *  \return Unused (ANSI-C compatibility).
 */
int main(void) {
  sysclk_init();
  board_init();
  configure_console();

  xQueueADC = xQueueCreate(100, sizeof(adcData));
  if (xQueueADC == NULL)
    printf("falha em criar a queue xQueueADC \n");
	
  xQueueProc = xQueueCreate(100, sizeof(uint32_t));
  if (xQueueProc == NULL) {
	  printf("falha em criar a xQueueProx\n");
  }

  if (xTaskCreate(task_adc, "ADC", TASK_ADC_STACK_SIZE, NULL,
                  TASK_ADC_STACK_PRIORITY, NULL) != pdPASS) {
    printf("Failed to create test ADC task\r\n");
  }
  
  if (xTaskCreate(task_proc, "PROC", TASK_PROC_STACK_SIZE, NULL, TASK_PROC_STACK_PRIORITY, NULL) != pdPASS) {
	  printf("Failed to create test PROC task\r\n");
  }

  vTaskStartScheduler();

  while (1) {  }

  /* Will only get here if there was insufficient memory to create the idle
   * task. */
  return 0;
}
