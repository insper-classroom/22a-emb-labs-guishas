#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

#define CALL_PIO			  PIOD
#define CALL_PIO_ID        ID_PIOD
#define CALL_PIO_IDX		  28
#define CALL_PIO_IDX_MASK  (1u << CALL_PIO_IDX)

#define TRIG_PIO			  PIOA
#define TRIG_PIO_ID        ID_PIOA
#define TRIG_PIO_IDX		  24
#define TRIG_PIO_IDX_MASK  (1u << TRIG_PIO_IDX)

#define ECHO_PIO			  PIOD
#define ECHO_PIO_ID        ID_PIOD
#define ECHO_PIO_IDX		  26
#define ECHO_PIO_IDX_MASK  (1u << ECHO_PIO_IDX)

float freq = (float) 1/(0.000116);
double pulsos;

volatile char call_flag = 0;
volatile char echo_flag = 0;
volatile char erro_flag = 0;

void init(void);

void call_callback(void){
	call_flag = 1;
}

void echo_callback(void){
	if (pio_get(ECHO_PIO, PIO_INPUT, ECHO_PIO_IDX_MASK)) {
		if (call_flag){
			RTT_init(freq, 0 , 0);
			echo_flag = 1;
		} else {
			erro_flag = 1;
		}
	} else {
		echo_flag = 0;
		pulsos = rtt_read_timer_value(RTT);
	}
}

static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource) {

	uint16_t pllPreScale = (int) (((float) 32768) / freqPrescale);
	
	rtt_sel_source(RTT, false);
	rtt_init(RTT, pllPreScale);
	
	if (rttIRQSource & RTT_MR_ALMIEN) {
		uint32_t ul_previous_time;
		ul_previous_time = rtt_read_timer_value(RTT);
		while (ul_previous_time == rtt_read_timer_value(RTT));
		rtt_write_alarm_time(RTT, IrqNPulses+ul_previous_time);
	}

	/* config NVIC */
	NVIC_DisableIRQ(RTT_IRQn);
	NVIC_ClearPendingIRQ(RTT_IRQn);
	NVIC_SetPriority(RTT_IRQn, 4);
	NVIC_EnableIRQ(RTT_IRQn);

	/* Enable RTT interrupt */
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN))
	rtt_enable_interrupt(RTT, rttIRQSource);
	else
	rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		erro_flag = 1;
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
	}
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask)) {
		pio_clear(pio, mask);	
	} else {
		pio_set(pio, mask);
	}
}

void piscar_trig(){
	pio_clear(TRIG_PIO, TRIG_PIO_IDX_MASK);
	delay_us(10);
	pio_set(TRIG_PIO, TRIG_PIO_IDX_MASK);

}

void desenha_distancia(int distance){
	char dist[10];
	gfx_mono_generic_draw_filled_rect(0, 0, 50, 32, GFX_PIXEL_CLR);
	sprintf(dist, "%d", distance);
	gfx_mono_draw_string(dist, 0, 0, &sysfont);
	gfx_mono_draw_string("cm", 0, 18, &sysfont);
}

void desenha_erro_2() {
	gfx_mono_draw_string("ERRO", 0, 0, &sysfont);
	gfx_mono_draw_string("DIS", 0, 18, &sysfont);
}

void desenha_erro() {
	gfx_mono_draw_string("ERRO", 0, 10, &sysfont);
}

float get_dist() {
	while(echo_flag){
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
	
	float d = (float) (100.0*pulsos*340)/(freq*2);
	
	if (d > 400){
		return -1;
	}
	
	return d;
}

void init(void){

	sysclk_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;

	pmc_enable_periph_clk(TRIG_PIO_ID);
	pio_set_output(TRIG_PIO, TRIG_PIO_IDX_MASK, 0, 0, 1);
	
	pmc_enable_periph_clk(CALL_PIO_ID);
	pio_configure(CALL_PIO, PIO_INPUT, CALL_PIO_IDX_MASK, PIO_PULLUP | PIO_DEBOUNCE);
	pio_set_debounce_filter(CALL_PIO, CALL_PIO_IDX_MASK, 60);
	pio_handler_set(CALL_PIO, CALL_PIO_ID, CALL_PIO_IDX_MASK, PIO_IT_FALL_EDGE, call_callback);
	pio_enable_interrupt(CALL_PIO, CALL_PIO_IDX_MASK);
	pio_get_interrupt_status(CALL_PIO);
	NVIC_EnableIRQ(CALL_PIO_ID);
	NVIC_SetPriority(CALL_PIO_ID, 4);

	pmc_enable_periph_clk(ECHO_PIO_ID);
	pio_configure(ECHO_PIO, PIO_INPUT,ECHO_PIO_IDX_MASK, PIO_DEFAULT);
	pio_set_debounce_filter(ECHO_PIO, ECHO_PIO_IDX_MASK, 60);
	pio_handler_set(ECHO_PIO, ECHO_PIO_ID, ECHO_PIO_IDX_MASK, PIO_IT_EDGE, echo_callback);
	pio_enable_interrupt(ECHO_PIO, ECHO_PIO_IDX_MASK);
	pio_get_interrupt_status(ECHO_PIO);
	NVIC_EnableIRQ(ECHO_PIO_ID);
	NVIC_SetPriority(ECHO_PIO_ID, 5);
}


int main (void)
{
	board_init();
	delay_init();

	init();

	gfx_mono_ssd1306_init();

	gfx_mono_generic_draw_vertical_line(52, 0, 32, GFX_PIXEL_SET);
	gfx_mono_generic_draw_horizontal_line(52, 31, 75, GFX_PIXEL_SET);
	
	/* Insert application code here, after the board has been initialized. */
	while(1) {
		
		if (erro_flag){
			desenha_erro();
			erro_flag = 0;
		}
		
		if (echo_flag){
			int distancia = get_dist();
			
			if (distancia != -1){
				desenha_distancia(distancia);
			} else {
				desenha_erro_2();
				erro_flag = 0;
			}
			
			call_flag = 0;
		}
		
		if (call_flag) {
			piscar_trig();
		}
		
		pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
