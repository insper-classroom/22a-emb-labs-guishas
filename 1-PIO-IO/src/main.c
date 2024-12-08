/**
 * 5 semestre - Eng. da Computação - Insper
 * Rafael Corsi - rafael.corsi@insper.edu.br
 *
 * Projeto 0 para a placa SAME70-XPLD
 *
 * Objetivo :
 *  - Introduzir ASF e HAL
 *  - Configuracao de clock
 *  - Configuracao pino In/Out
 *
 * Material :
 *  - Kit: ATMEL SAME70-XPLD - ARM CORTEX M7
 */

/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include "asf.h"

/************************************************************************/
/* defines                                                              */
/************************************************************************/

#define LED_PIO				PIOC 
#define LED_PIO_ID			ID_PIOC
#define LED_PIO_IDX			8
#define LED_PIO_IDX_MASK	(1 << LED_PIO_IDX)

#define BUT_PIO				PIOA
#define BUT_PIO_ID			ID_PIOA
#define BUT_PIO_IDX			11
#define BUT_PIO_IDX_MASK	(1u << BUT_PIO_IDX)

#define LED1_EXT1_PIO			PIOA
#define LED1_EXT1_PIO_ID		ID_PIOA
#define LED1_EXT1_PIO_IDX		0
#define LED1_EXT1_PIO_IDX_MASK	(1 << LED1_EXT1_PIO_IDX)

#define BUT1_EXT1_PIO			PIOD
#define BUT1_EXT1_PIO_ID		ID_PIOD
#define BUT1_EXT1_PIO_IDX		28
#define BUT1_EXT1_PIO_IDX_MASK	(1u << BUT1_EXT1_PIO_IDX)

#define LED2_EXT1_PIO			PIOC
#define LED2_EXT1_PIO_ID		ID_PIOC
#define LED2_EXT1_PIO_IDX		30
#define LED2_EXT1_PIO_IDX_MASK	(1 << LED2_EXT1_PIO_IDX)

#define BUT2_EXT1_PIO			PIOC
#define BUT2_EXT1_PIO_ID		ID_PIOC
#define BUT2_EXT1_PIO_IDX		31
#define BUT2_EXT1_PIO_IDX_MASK	(1u << BUT2_EXT1_PIO_IDX)

#define LED3_EXT1_PIO			PIOB
#define LED3_EXT1_PIO_ID		ID_PIOB
#define LED3_EXT1_PIO_IDX		2
#define LED3_EXT1_PIO_IDX_MASK	(1 << LED3_EXT1_PIO_IDX)

#define BUT3_EXT1_PIO			PIOA
#define BUT3_EXT1_PIO_ID		ID_PIOA
#define BUT3_EXT1_PIO_IDX		19
#define BUT3_EXT1_PIO_IDX_MASK	(1u << BUT3_EXT1_PIO_IDX)

/************************************************************************/
/* constants                                                            */
/************************************************************************/

/************************************************************************/
/* variaveis globais                                                    */
/************************************************************************/

/************************************************************************/
/* prototypes                                                           */
/************************************************************************/

void init(void);

/************************************************************************/
/* interrupcoes                                                         */
/************************************************************************/

/************************************************************************/
/* funcoes                                                              */
/************************************************************************/

// Função de inicialização do uC
void init(void) {
	// Initialize the board clock
	sysclk_init();
	
	// Desativar watch dog timer
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// Ativa o PIO na qual o LED foi conectado
	// para que possamos controlar o LED
	pmc_enable_periph_clk(LED_PIO_ID);
	pmc_enable_periph_clk(BUT_PIO_ID);
	pmc_enable_periph_clk(LED1_EXT1_PIO_ID);
	pmc_enable_periph_clk(BUT1_EXT1_PIO_ID);
	
	// Inicializa o pino 8 (PC8) do PIO C como saída
	pio_set_output(LED_PIO, LED_PIO_IDX_MASK, 1, 0, 0);
	pio_set_output(LED1_EXT1_PIO, LED1_EXT1_PIO_IDX_MASK, 1, 0, 0);
	pio_set_output(LED2_EXT1_PIO, LED2_EXT1_PIO_IDX_MASK, 1, 0, 0);
	pio_set_output(LED3_EXT1_PIO, LED3_EXT1_PIO_IDX_MASK, 1, 0, 0);
	
	// Configura pino ligado ao botão como entrada com pull-up
	pio_set_input(BUT_PIO, BUT_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT_PIO, BUT_PIO_IDX_MASK, 1);
	
	pio_set_input(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK, 1);
	
	pio_set_input(BUT2_EXT1_PIO, BUT2_EXT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT2_EXT1_PIO, BUT2_EXT1_PIO_IDX_MASK, 1);
	
	pio_set_input(BUT3_EXT1_PIO, BUT3_EXT1_PIO_IDX_MASK, PIO_DEFAULT);
	pio_pull_up(BUT3_EXT1_PIO, BUT3_EXT1_PIO_IDX_MASK, 1);
}

/************************************************************************/
/* Main                                                                 */
/************************************************************************/

// Funcao principal chamada na inicalizacao do uC.
int main(void)
{
  init();

  // super loop
  // aplicacoes embarcadas não devem sair do while(1).
  while (1)
  {  

	if (!pio_get(BUT_PIO, PIO_INPUT, BUT_PIO_IDX_MASK)) {
		for (int i = 0; i < 5; i++) {
			pio_set(PIOC, LED_PIO_IDX_MASK);
			delay_ms(500);
			pio_clear(PIOC, LED_PIO_IDX_MASK);
			delay_ms(500);
		}
		pio_set(PIOC, LED_PIO_IDX_MASK);
	}
	
	if (!pio_get(BUT1_EXT1_PIO, PIO_INPUT, BUT1_EXT1_PIO_IDX_MASK)) {
		for (int i = 0; i < 5; i++) {
			pio_set(PIOA, LED1_EXT1_PIO_IDX_MASK);
			delay_ms(500);
			pio_clear(PIOA, LED1_EXT1_PIO_IDX_MASK);
			delay_ms(500);
		}
		pio_set(PIOA, LED1_EXT1_PIO_IDX_MASK);
	}
	
	if (!pio_get(BUT2_EXT1_PIO, PIO_INPUT, BUT2_EXT1_PIO_IDX_MASK)) {
		for (int i = 0; i < 5; i++) {
			pio_set(PIOC, LED2_EXT1_PIO_IDX_MASK);
			delay_ms(500);
			pio_clear(PIOC, LED2_EXT1_PIO_IDX_MASK);
			delay_ms(500);
		}
		pio_set(PIOC, LED2_EXT1_PIO_IDX_MASK);	
	}
	
	if (!pio_get(BUT3_EXT1_PIO, PIO_INPUT, BUT3_EXT1_PIO_IDX_MASK)) {
		for (int i = 0; i < 5; i++) {
			pio_set(PIOB, LED3_EXT1_PIO_IDX_MASK);
			delay_ms(500);
			pio_clear(PIOB, LED3_EXT1_PIO_IDX_MASK);
			delay_ms(500);
		}
		pio_set(PIOB, LED3_EXT1_PIO_IDX_MASK);	
	}
  }
  return 0;
}
