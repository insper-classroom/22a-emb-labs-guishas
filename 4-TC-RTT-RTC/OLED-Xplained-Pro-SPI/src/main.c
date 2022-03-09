#include <asf.h>

#include "gfx_mono_ug_2832hsweg04.h"
#include "gfx_mono_text.h"
#include "sysfont.h"

// GLOBALS

volatile char flag_rtc_alarm = 0;

// DEFINES

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

// STRUCT

typedef struct  {
	uint32_t year;
	uint32_t month;
	uint32_t day;
	uint32_t week;
	uint32_t hour;
	uint32_t minute;
	uint32_t second;
} calendar;

// PROTOTIPOS

void init(void);
void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq);
void pin_toggle(Pio *pio, uint32_t mask);
static void RTT_init(float freqPrescale, uint32_t IrqNPulses, uint32_t rttIRQSource);
void set_alarm_btn();
void but_callback();

// FUNCOES

void set_alarm_btn() {
	uint32_t current_hour, current_min, current_sec;
	uint32_t current_year, current_month, current_day, current_week;
	rtc_get_time(RTC, &current_hour, &current_min, &current_sec);
	rtc_get_date(RTC, &current_year, &current_month, &current_day, &current_week);
	
	/* configura alarme do RTC para daqui 20 segundos */
	rtc_set_date_alarm(RTC, 1, current_month, 1, current_day);
	rtc_set_time_alarm(RTC, 1, current_hour, 1, current_min, 1, current_sec + 5);
}

void pin_toggle(Pio *pio, uint32_t mask) {
	if(pio_get_output_data_status(pio, mask)) {
		pio_clear(pio, mask);
	} else {
		pio_set(pio, mask);
	}
}

void TC1_Handler() {
	volatile uint32_t status = tc_get_status(TC0, 1);
	
	pin_toggle(LED1_EXT1_PIO, LED1_EXT1_PIO_IDX_MASK);
}

void RTT_Handler(void) {
	uint32_t ul_status;

	/* Get RTT status - ACK */
	ul_status = rtt_get_status(RTT);

	/* IRQ due to Alarm */
	if ((ul_status & RTT_SR_ALMS) == RTT_SR_ALMS) {
		
	}
	
	/* IRQ due to Time has changed */
	if ((ul_status & RTT_SR_RTTINC) == RTT_SR_RTTINC) {
		pin_toggle(LED2_EXT1_PIO, LED2_EXT1_PIO_IDX_MASK);    // BLINK Led
	}

}

void RTC_Handler(void) {
	uint32_t ul_status = rtc_get_status(RTC);
	
	/* seccond tick */
	if ((ul_status & RTC_SR_SEC) == RTC_SR_SEC) {
		// o código para irq de segundo vem aqui
	}
	
	/* Time or date alarm */
	if ((ul_status & RTC_SR_ALARM) == RTC_SR_ALARM) {
		// o código para irq de alame vem aqui
		flag_rtc_alarm = 1;
	}

	rtc_clear_status(RTC, RTC_SCCR_SECCLR);
	rtc_clear_status(RTC, RTC_SCCR_ALRCLR);
	rtc_clear_status(RTC, RTC_SCCR_ACKCLR);
	rtc_clear_status(RTC, RTC_SCCR_TIMCLR);
	rtc_clear_status(RTC, RTC_SCCR_CALCLR);
	rtc_clear_status(RTC, RTC_SCCR_TDERRCLR);
}

void TC_init(Tc * TC, int ID_TC, int TC_CHANNEL, int freq){
	uint32_t ul_div;
	uint32_t ul_tcclks;
	uint32_t ul_sysclk = sysclk_get_cpu_hz();

	/* Configura o PMC */
	pmc_enable_periph_clk(ID_TC);

	/** Configura o TC para operar em  freq hz e interrupçcão no RC compare */
	tc_find_mck_divisor(freq, ul_sysclk, &ul_div, &ul_tcclks, ul_sysclk);
	tc_init(TC, TC_CHANNEL, ul_tcclks | TC_CMR_CPCTRG);
	tc_write_rc(TC, TC_CHANNEL, (ul_sysclk / ul_div) / freq);

	/* Configura NVIC*/
	NVIC_SetPriority(ID_TC, 4);
	NVIC_EnableIRQ((IRQn_Type) ID_TC);
	tc_enable_interrupt(TC, TC_CHANNEL, TC_IER_CPCS);
}

static float get_time_rtt(){
	uint ul_previous_time = rtt_read_timer_value(RTT);
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
	if (rttIRQSource & (RTT_MR_RTTINCIEN | RTT_MR_ALMIEN)) {
		rtt_enable_interrupt(RTT, rttIRQSource);
	} else {
		rtt_disable_interrupt(RTT, RTT_MR_RTTINCIEN | RTT_MR_ALMIEN);
	}
}

void RTC_init(Rtc *rtc, uint32_t id_rtc, calendar t, uint32_t irq_type) {
	/* Configura o PMC */
	pmc_enable_periph_clk(ID_RTC);

	/* Default RTC configuration, 24-hour mode */
	rtc_set_hour_mode(rtc, 0);

	/* Configura data e hora manualmente */
	rtc_set_date(rtc, t.year, t.month, t.day, t.week);
	rtc_set_time(rtc, t.hour, t.minute, t.second);

	/* Configure RTC interrupts */
	NVIC_DisableIRQ(id_rtc);
	NVIC_ClearPendingIRQ(id_rtc);
	NVIC_SetPriority(id_rtc, 4);
	NVIC_EnableIRQ(id_rtc);

	/* Ativa interrupcao via alarme */
	rtc_enable_interrupt(rtc,  irq_type);
}

void but_callback() {
	set_alarm_btn();
}

void init() {
	board_init();
	sysclk_init();
	delay_init();
	gfx_mono_ssd1306_init();
	
	WDT->WDT_MR = WDT_MR_WDDIS;
	
	// LEDS
	pmc_enable_periph_clk(LED1_EXT1_PIO_ID);
	pio_set_output(LED1_EXT1_PIO, LED1_EXT1_PIO_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(LED2_EXT1_PIO_ID);
	pio_set_output(LED2_EXT1_PIO, LED2_EXT1_PIO_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(LED3_EXT1_PIO_ID);
	pio_set_output(LED3_EXT1_PIO, LED3_EXT1_PIO_IDX_MASK, 1, 0, 0);
	
	pmc_enable_periph_clk(BUT1_EXT1_PIO_ID);
	pio_set_input(BUT1_EXT1_PIO_ID, BUT1_EXT1_PIO_IDX_MASK, PIO_DEBOUNCE);
	pio_pull_up(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK, 1);
	
	pio_handler_set(BUT1_EXT1_PIO,
					BUT1_EXT1_PIO_ID,
					BUT1_EXT1_PIO_IDX_MASK,
					PIO_IT_RISE_EDGE,
					but_callback);
	
	calendar rtc_initial = {2018, 3, 19, 12, 15, 45 ,1};
	RTC_init(RTC, ID_RTC, rtc_initial, RTC_IER_ALREN);
	
	pio_enable_interrupt(BUT1_EXT1_PIO, BUT1_EXT1_PIO_IDX_MASK);
	pio_get_interrupt_status(BUT1_EXT1_PIO);
	
	NVIC_EnableIRQ(BUT1_EXT1_PIO_ID);
	NVIC_SetPriority(BUT1_EXT1_PIO_ID, 4);
}

int main (void)
{
	init();
  
	//gfx_mono_draw_filled_circle(20, 16, 16, GFX_PIXEL_SET, GFX_WHOLE);
	//gfx_mono_draw_string("mundo", 50,16, &sysfont);
	char hora[128];
	char min[128];
	char sec[128];
	
	uint32_t current_hour_draw, current_min_draw, current_sec_draw;
	
	TC_init(TC0, ID_TC1, 1, 4);
	tc_start(TC0, 1);
	RTT_init(0.25, 0, RTT_MR_RTTINCIEN);

	while (1) {
		rtc_get_time(RTC, &current_hour_draw, &current_min_draw, &current_sec_draw);
		
		sprintf(hora, "%d", current_hour_draw);
		sprintf(min, "%d", current_min_draw);
		if (current_sec_draw < 10) {
			sprintf(sec, "0%d", current_sec_draw);
		} else {
			sprintf(sec, "%d", current_sec_draw);
		}
		
		gfx_mono_draw_string(hora, 10, 0, &sysfont);
		gfx_mono_draw_string(":", 30, 0, &sysfont);
		gfx_mono_draw_string(min, 40, 0, &sysfont);
		gfx_mono_draw_string(":", 60, 0, &sysfont);
		gfx_mono_draw_string(sec, 70, 0, &sysfont);
		
		if (flag_rtc_alarm) {
			pio_clear(LED3_EXT1_PIO, LED3_EXT1_PIO_IDX_MASK);
			delay_ms(200);
			pio_set(LED3_EXT1_PIO, LED3_EXT1_PIO_IDX_MASK);
			flag_rtc_alarm = 0;
		}
		
		//pmc_sleep(SAM_PM_SMODE_SLEEP_WFI);
	}
}
