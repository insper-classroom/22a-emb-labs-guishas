/************************************************************************
 * 5 semestre - Eng. da Computao - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Material:
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 *
 * Objetivo:
 *  - Demonstrar interrupção do PIO
 *
 * Periféricos:
 *  - PIO
 *  - PMC
 *
 * Log:
 *  - 10/2018: Criação
 ************************************************************************/

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"
#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

// LED
#define LED_PIO      PIOC
#define LED_PIO_ID   ID_PIOC
#define LED_IDX      8
#define LED_IDX_MASK (1 << LED_IDX)

#define LED1_EXT1_PIO			PIOA
#define LED1_EXT1_PIO_ID		ID_PIOA
#define LED1_EXT1_PIO_IDX		0
#define LED1_EXT1_PIO_IDX_MASK	(1 << LED1_EXT1_PIO_IDX)

// Botão
#define BUT_PIO      PIOA
#define BUT_PIO_ID   ID_PIOA
#define BUT_IDX  11
#define BUT_IDX_MASK (1 << BUT_IDX)

#define BUT1_EXT1_PIO			PIOD
#define BUT1_EXT1_PIO_ID		ID_PIOD
#define BUT1_EXT1_PIO_IDX		28
#define BUT1_EXT1_PIO_IDX_MASK	(1u << BUT1_EXT1_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototype                                                            */
/************************************************************************/
void io_init(void);
void pisca_led(int n, int t);

/************************************************************************/
/* flag                                                           */
/************************************************************************/

volatile char but_flag;
volatile char but1_ext1_aumentar_flag;
volatile char but1_ext1_diminuir_flag;
volatile int delay = 100; //miliseconds

/************************************************************************/
/* handler / callbacks                                                  */
/************************************************************************/

/*
 * Exemplo de callback para o botao, sempre que acontecer
 * ira piscar o led por 5 vezes
 *
 * !! Isso é um exemplo ruim, nao deve ser feito na pratica, !!
 * !! pois nao se deve usar delays dentro de interrupcoes    !!
 */
void but_callback(void)
{
  but_flag = 1;
}

void but1_ext1_callback(void) {
	if (pio_get(BUT1_EXT1_PIO, PIO_INPUT, BUT1_EXT1_PIO_IDX_MASK)) {
		but1_ext1_aumentar_flag = 1;
	} else {
		but1_ext1_diminuir_flag = 1;
	}
}

/************************************************************************/
/* funções                                                              */
/************************************************************************/

// pisca led N vez no periodo T
void pisca_led(int n, int t){
  for (int i=0;i<n;i++){
    pio_clear(LED_PIO, LED_IDX_MASK);
    delay_ms(t);
    pio_set(LED_PIO, LED_IDX_MASK);
    delay_ms(t);
  }
}

void aumenta_freq() {
	if (delay != 0) {
		delay -= 100;
	}
}

void diminui_freq() {
	delay += 100;
}

// Inicializa botao SW0 do kit com interrupcao
void io_init(void)
{

  // Configura led
	pmc_enable_periph_clk(LED_PIO_ID);
	pio_configure(LED_PIO, PIO_OUTPUT_0, LED_IDX_MASK, PIO_DEFAULT);
	
	pmc_enable_periph_clk(LED1_EXT1_PIO_ID);
	pio_configure(LED1_EXT1_PIO, PIO_OUTPUT_0, LED1_EXT1_PIO_IDX_MASK, PIO_DEFAULT);

  // Inicializa clock do periférico PIO responsavel pelo botao
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(BUT1_EXT1_PIO_ID);

  // Configura PIO para lidar com o pino do botão como entrada
  // com pull-up
	pio_configure(BUT_PIO, PIO_INPUT, BUT_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT_PIO, BUT_IDX_MASK, 60);
	
	pio_configure(BUT1_EXT1_PIO, PIO_INPUT, BUT1_EXT1_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK, 60);

  // Configura interrupção no pino referente ao botao e associa
  // função de callback caso uma interrupção for gerada
  // a função de callback é a: but_callback()
	pio_handler_set(BUT_PIO,
                  BUT_PIO_ID,
                  BUT_IDX_MASK,
                  PIO_IT_RISE_EDGE,
                  but_callback);
				  
	pio_handler_set(BUT1_EXT1_PIO,
					BUT1_EXT1_PIO_ID,
					BUT1_EXT1_PIO_IDX_MASK,
					PIO_IT_EDGE,
					but1_ext1_callback);

  // Ativa interrupção e limpa primeira IRQ gerada na ativacao
	pio_enable_interrupt(BUT_PIO, BUT_IDX_MASK);
	pio_get_interrupt_status(BUT_PIO);
	
	pio_enable_interrupt(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_EXT1_PIO);
  
  // Configura NVIC para receber interrupcoes do PIO do botao
  // com prioridade 4 (quanto mais próximo de 0 maior)
	NVIC_EnableIRQ(BUT_PIO_ID);
	NVIC_SetPriority(BUT_PIO_ID, 4); // Prioridade 4
	
	NVIC_EnableIRQ(BUT1_EXT1_PIO_ID);
	NVIC_SetPriority(BUT1_EXT1_PIO_ID, 4);
	
	pio_set(LED_PIO, LED_IDX_MASK);
	pio_set(LED1_EXT1_PIO, LED1_EXT1_PIO_IDX_MASK);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
void main(void)
{
	// Inicializa clock
	sysclk_init();

	// Desativa watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// configura botao com interrupcao
	io_init();
	
	// string para mostrar na tela
	char str[128];
	char num[128];
	
	
	gfx_mono_ssd1306_init();
	// super loop
	// aplicacoes embarcadas no devem sair do while(1).
	while(1) {
		
		sprintf(str, "%s", " ms");
		sprintf(num, "%d", delay);
		gfx_mono_draw_string(num, 0, 0, &sysfont);
		gfx_mono_draw_string(str, 60, 0, &sysfont);
		
		if (but_flag) {
			pisca_led(5, delay);
			but_flag = 0;
		}
		
		if (but1_ext1_aumentar_flag) {
			while (but1_ext1_diminuir_flag == 0) {
				aumenta_freq();
				delay_ms(300);
			}
			but1_ext1_aumentar_flag = 0;
		}
		
		if (but1_ext1_diminuir_flag) {
			diminui_freq();
			but1_ext1_diminuir_flag = 0;
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
			
	}
}
