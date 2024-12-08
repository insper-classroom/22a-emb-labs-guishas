/************************************************************************/
/* includes                                                             */
/************************************************************************/

#include <asf.h>
#include <string.h>
#include "ili9341.h"
#include "lvgl.h"
#include "touch/touch.h"

/************************************************************************/
/* LCD / LVGL                                                           */
/************************************************************************/

#define LV_HOR_RES_MAX          (320)
#define LV_VER_RES_MAX          (240)

/*A static or global variable to store the buffers*/
static lv_disp_draw_buf_t disp_buf;

/*Static or global buffer(s). The second buffer is optional*/
static lv_color_t buf_1[LV_HOR_RES_MAX * LV_VER_RES_MAX];
static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
static lv_indev_drv_t indev_drv;

static  lv_obj_t * labelBtn1;

/************************************************************************/
/* RTOS                                                                 */
/************************************************************************/

#define TASK_LCD_STACK_SIZE                (1024*6/sizeof(portSTACK_TYPE))
#define TASK_LCD_STACK_PRIORITY            (tskIDLE_PRIORITY)

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask,  signed char *pcTaskName);
extern void vApplicationIdleHook(void);
extern void vApplicationTickHook(void);
extern void vApplicationMallocFailedHook(void);
extern void xPortSysTickHandler(void);

extern void vApplicationStackOverflowHook(xTaskHandle *pxTask, signed char *pcTaskName) {
	printf("stack overflow %x %s\r\n", pxTask, (portCHAR *)pcTaskName);
	for (;;) {	}
}

extern void vApplicationIdleHook(void) { }

extern void vApplicationTickHook(void) { }

extern void vApplicationMallocFailedHook(void) {
	configASSERT( ( volatile void * ) NULL );
}

/************************************************************************/
/* lvgl                                                                 */
/************************************************************************/

static void power_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void menu_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void clock_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void up_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

static void down_handler(lv_event_t * e) {
	lv_event_code_t code = lv_event_get_code(e);

	if(code == LV_EVENT_CLICKED) {
		LV_LOG_USER("Clicked");
	}
	else if(code == LV_EVENT_VALUE_CHANGED) {
		LV_LOG_USER("Toggled");
	}
}

void lv_termostato(void) {
	static lv_style_t style;
	lv_style_init(&style);
	lv_style_set_bg_color(&style, lv_color_black());
	lv_style_set_border_color(&style, lv_color_black());
	lv_style_set_border_width(&style, 5);
	
    lv_obj_t * btn1 = lv_btn_create(lv_scr_act());
    lv_obj_add_event_cb(btn1, power_handler, LV_EVENT_ALL, NULL);
    lv_obj_align(btn1, LV_ALIGN_BOTTOM_LEFT, 20, -32);
    lv_obj_add_style(btn1, &style, 0);
    lv_obj_set_width(btn1, 25);
    lv_obj_set_height(btn1, 25);
	
	lv_obj_t * btn2 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn2, menu_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn2, LV_ALIGN_BOTTOM_LEFT, 70, -32);
	lv_obj_add_style(btn2, &style, 0);
	lv_obj_set_width(btn2, 25);
	lv_obj_set_height(btn2, 25);
	
	lv_obj_t * labelBtn2 = lv_label_create(btn2);
	lv_label_set_text(labelBtn2, LV_SYMBOL_HOME);
	lv_obj_center(labelBtn2);
	
	lv_obj_t * btn3 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn3, clock_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn3, LV_ALIGN_BOTTOM_LEFT, 120, -32);
	lv_obj_add_style(btn3, &style, 0);
	lv_obj_set_width(btn3, 25);
	lv_obj_set_height(btn3, 25);
	
	lv_obj_t * labelBtn3 = lv_label_create(btn3);
	lv_label_set_text(labelBtn3, LV_SYMBOL_REFRESH);
	lv_obj_center(labelBtn3);
	
	lv_obj_t * btn4 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn4, up_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn4, LV_ALIGN_BOTTOM_LEFT, 225, -32);
	lv_obj_add_style(btn4, &style, 0);
	lv_obj_set_width(btn4, 25);
	lv_obj_set_height(btn4, 25);
	
	lv_obj_t * labelBtn4 = lv_label_create(btn4);
	lv_label_set_text(labelBtn4, LV_SYMBOL_UP);
	lv_obj_center(labelBtn4);
	
	lv_obj_t * btn5 = lv_btn_create(lv_scr_act());
	lv_obj_add_event_cb(btn5, down_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(btn5, LV_ALIGN_BOTTOM_LEFT, 270, -32);
	lv_obj_add_style(btn5, &style, 0);
	lv_obj_set_width(btn5, 25);
	lv_obj_set_height(btn5, 25);
	
	lv_obj_t * labelBtn5 = lv_label_create(btn5);
	lv_label_set_text(labelBtn5, LV_SYMBOL_DOWN);
	lv_obj_center(labelBtn5);

	labelBtn1 = lv_label_create(btn1);
	lv_label_set_text(labelBtn1, LV_SYMBOL_POWER);
    lv_obj_center(labelBtn1);
	
	lv_obj_t * labelBarra = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra, "[ ");
	lv_obj_center(labelBarra);
	lv_obj_align(labelBarra, LV_ALIGN_BOTTOM_LEFT, 5, -30);
	lv_obj_set_width(labelBarra, 30);
	lv_obj_set_height(labelBarra, 30);
	
	lv_obj_t * labelBarra2 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra2, " | ");
	lv_obj_center(labelBarra2);
	lv_obj_align(labelBarra2, LV_ALIGN_BOTTOM_LEFT, 50, -30);
	lv_obj_set_width(labelBarra2, 30);
	lv_obj_set_height(labelBarra2, 30);
	
	lv_obj_t * labelBarra3 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra3, " | ");
	lv_obj_center(labelBarra3);
	lv_obj_align(labelBarra3, LV_ALIGN_BOTTOM_LEFT, 100, -30);
	lv_obj_set_width(labelBarra3, 30);
	lv_obj_set_height(labelBarra3, 30);
	
	lv_obj_t * labelBarra4 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra4, " ]");
	lv_obj_center(labelBarra4);
	lv_obj_align(labelBarra4, LV_ALIGN_BOTTOM_LEFT, 150, -30);
	lv_obj_set_width(labelBarra4, 30);
	lv_obj_set_height(labelBarra4, 30);
	
	lv_obj_t * labelBarra5 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra5, "[ ");
	lv_obj_center(labelBarra5);
	lv_obj_align(labelBarra5, LV_ALIGN_BOTTOM_LEFT, 210, -30);
	lv_obj_set_width(labelBarra5, 30);
	lv_obj_set_height(labelBarra5, 30);
	
	lv_obj_t * labelBarra6 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra6, " | ");
	lv_obj_center(labelBarra6);
	lv_obj_align(labelBarra6, LV_ALIGN_BOTTOM_LEFT, 250, -30);
	lv_obj_set_width(labelBarra6, 30);
	lv_obj_set_height(labelBarra6, 30);
	
	lv_obj_t * labelBarra7 = lv_label_create(lv_scr_act());
	lv_label_set_text(labelBarra7, " ]");
	lv_obj_center(labelBarra7);
	lv_obj_align(labelBarra7, LV_ALIGN_BOTTOM_LEFT, 295, -30);
	lv_obj_set_width(labelBarra7, 30);
	lv_obj_set_height(labelBarra7, 30);
}

/************************************************************************/
/* TASKS                                                                */
/************************************************************************/

static void task_lcd(void *pvParameters) {
	int px, py;

	lv_termostato();

	for (;;)  {
		lv_tick_inc(50);
		lv_task_handler();
		vTaskDelay(50);
	}
}

/************************************************************************/
/* configs                                                              */
/************************************************************************/

static void configure_lcd(void) {
	/**LCD pin configure on SPI*/
	pio_configure_pin(LCD_SPI_MISO_PIO, LCD_SPI_MISO_FLAGS);  //
	pio_configure_pin(LCD_SPI_MOSI_PIO, LCD_SPI_MOSI_FLAGS);
	pio_configure_pin(LCD_SPI_SPCK_PIO, LCD_SPI_SPCK_FLAGS);
	pio_configure_pin(LCD_SPI_NPCS_PIO, LCD_SPI_NPCS_FLAGS);
	pio_configure_pin(LCD_SPI_RESET_PIO, LCD_SPI_RESET_FLAGS);
	pio_configure_pin(LCD_SPI_CDS_PIO, LCD_SPI_CDS_FLAGS);
	
	ili9341_init();
	ili9341_backlight_on();
}

static void configure_console(void) {
	const usart_serial_options_t uart_serial_options = {
		.baudrate = USART_SERIAL_EXAMPLE_BAUDRATE,
		.charlength = USART_SERIAL_CHAR_LENGTH,
		.paritytype = USART_SERIAL_PARITY,
		.stopbits = USART_SERIAL_STOP_BIT,
	};

	/* Configure console UART. */
	stdio_serial_init(CONSOLE_UART, &uart_serial_options);

	/* Specify that stdout should not be buffered. */
	setbuf(stdout, NULL);
}

/************************************************************************/
/* port lvgl                                                            */
/************************************************************************/

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p) {
	ili9341_set_top_left_limit(area->x1, area->y1);   ili9341_set_bottom_right_limit(area->x2, area->y2);
	ili9341_copy_pixels_to_screen(color_p,  (area->x2 + 1 - area->x1) * (area->y2 + 1 - area->y1));
	
	/* IMPORTANT!!!
	* Inform the graphics library that you are ready with the flushing*/
	lv_disp_flush_ready(disp_drv);
}

void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data) {
	int px, py, pressed;
	
	if (readPoint(&px, &py))
		data->state = LV_INDEV_STATE_PRESSED;
	else
		data->state = LV_INDEV_STATE_RELEASED; 
	
	data->point.x = px;
	data->point.y = py;
}

void configure_lvgl(void) {
	lv_init();
	lv_disp_draw_buf_init(&disp_buf, buf_1, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);
	
	lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
	disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
	disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
	disp_drv.hor_res = LV_HOR_RES_MAX;      /*Set the horizontal resolution in pixels*/
	disp_drv.ver_res = LV_VER_RES_MAX;      /*Set the vertical resolution in pixels*/

	lv_disp_t * disp;
	disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
	
	/* Init input on LVGL */
	lv_indev_drv_init(&indev_drv);
	indev_drv.type = LV_INDEV_TYPE_POINTER;
	indev_drv.read_cb = my_input_read;
	lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv);
}

/************************************************************************/
/* main                                                                 */
/************************************************************************/
int main(void) {
	/* board and sys init */
	board_init();
	sysclk_init();
	configure_console();

	/* LCd, touch and lvgl init*/
	configure_lcd();
	configure_touch();
	configure_lvgl();

	/* Create task to control oled */
	if (xTaskCreate(task_lcd, "LCD", TASK_LCD_STACK_SIZE, NULL, TASK_LCD_STACK_PRIORITY, NULL) != pdPASS) {
		printf("Failed to create lcd task\r\n");
	}
	
	/* Start the scheduler. */
	vTaskStartScheduler();

	while(1){ }
}
