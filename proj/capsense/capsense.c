// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "../../ch32v003fun/ch32v003fun.h"

#define SELECT_BUTTONS 0
#define SELECT_SLIDER2 0
#define SELECT_SLIDER3 1

#if (((SELECT_BUTTONS < 1) && (SELECT_SLIDER2 < 1) && (SELECT_SLIDER3 < 1)) || ((SELECT_BUTTONS + SELECT_SLIDER2 + SELECT_SLIDER3) > 1))
#error "please enable one of the demos"
#endif

#define CAPSENSE_PUSHPULL_PORT GPIOC
#define CAPSENSE_PUSHPULL_PIN 7
#define CAPSENSE_SENSE_N_LINES 3
#define CAPSENSE_SENSE_PORT_L2 GPIOD
#define CAPSENSE_SENSE_PORT_L5 GPIOC
#define CAPSENSE_SENSE_PORT_L6 GPIOC
#include "ch32v003_capsense.h"

#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

#define LOG_LVL 1
#if LOG_LVL >= 1
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define SCHD_sense_i 967 * DELAY_US_TIME
#define SCHD_led_i 809 * DELAY_US_TIME
#define SCHD_log_i 73 * DELAY_MS_TIME

/* somehow this ISR won't get called??
*/

int main()
{
	SystemInit48HSI();
	#if LOG_LVL >= 1
	SetupUART( UART_BRR );
	#endif

	//printf("\r\n\r\nlow power example\r\n\r\n");

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// GPIO C3 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(3*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(3*4);
	GPIOC->OUTDR |= (1 << 3);

	// GPIO C4 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
	GPIOC->OUTDR |= (1 << 4);

	// give the user time to open the terminal connection
	Delay_Ms(3000);
	LOG("3000ms wait over\r\n");

	capsense_assign_pushpull();
	capsense_assign_sense();

	GPIOC->OUTDR |= (1 << 3);

	//uint16_t counter = 0;
	LOG("entering capsense loop\r\n");

	uint8_t result_line6;
	uint8_t result_line5;
	uint8_t result_line2;

	uint8_t cal_line6 = capsense_sense(CAPSENSE_SENSE_PORT_L6, 6);
	Delay_Ms(1);
	uint8_t cal_line5 = capsense_sense(CAPSENSE_SENSE_PORT_L5, 5);
	Delay_Ms(1);
	uint8_t cal_line2 = capsense_sense(CAPSENSE_SENSE_PORT_L2, 2);

	uint8_t slider_output = 0;
	uint8_t slider_pre_output = 0;
	int16_t slider_memory;

	uint8_t schd_sense_step = 0;
	uint8_t schd_led_step = 0;

	uint32_t schd_led_i = SCHD_led_i;

	uint32_t schd_sense_t = 0;
	uint32_t schd_led_t = 0;
	uint32_t schd_log_t = 0;
	uint32_t led_ctrl_t0 = 0;
	uint32_t led_ctrl_t1 = 0;
	for (;;) {
		if (SysTick->CNT - schd_sense_t > SCHD_sense_i) {
		#if SELECT_SLIDER2 == 1
			switch (schd_sense_step) {
				case 0:
					//result_line6 = capsense_sense_pin(6);
					result_line6 = capsense_offset(capsense_sense_pin(6), cal_line6, 8);
					//capsense_filter(&result_line6, capsense_offset(capsense_sense_pin(6), cal_line6, 3));
					schd_sense_step++;
					break;
				case 1:
					//result_line5 = capsense_sense_pin(5);
					result_line5 = capsense_offset(capsense_sense_pin(5), cal_line5, 8);
					//capsense_filter(&result_line5, capsense_offset(capsense_sense_pin(5), cal_line5, 3));
					schd_sense_step++;
					break;
				case 2:
					capsense_slider2_scroll(result_line6, result_line5, 50, &slider_output, &slider_memory, 16, 8);
					//slider_output = capsense_discretize(slider_output, 3);
					schd_sense_step++;
					break;
				case 3:
					led_ctrl_t0 = (slider_output >> 2) * DELAY_US_TIME;		// on-time
					led_ctrl_t1 = ((255 - slider_output) >> 2) * DELAY_US_TIME;	// off-time
					schd_sense_step = 0;
					break;
			}
			#endif
			#if SELECT_SLIDER3 == 1
			/*
			switch (schd_sense_step) {
				case 0:
					//result_line6 = capsense_sense_pin(6);
					result_line6 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L6, 6), cal_line6, 8);
					//capsense_filter(&result_line6, capsense_offset(capsense_sense_pin(6), cal_line6, 3));
					schd_sense_step++;
					break;
				case 1:
					result_line5 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L5, 5), cal_line5, 8);
					schd_sense_step++;
					break;
				case 2:
					//result_line2 = capsense_sense_pin(2);
					result_line2 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L2, 2), cal_line2, 8);
					//capsense_filter(&result_line2, capsense_offset(capsense_sense_pin(2), cal_line2, 3));
					schd_sense_step++;
					break;
				case 3:
					slider_output = capsense_slider3(result_line6, result_line5, result_line2, 30);
					//slider_output = capsense_discretize(slider_output, 3);
					schd_sense_step++;
					break;
				case 4:
					led_ctrl_t0 = (slider_output >> 2) * DELAY_US_TIME;		// on-time
					led_ctrl_t1 = ((255 - slider_output) >> 2) * DELAY_US_TIME;	// off-time
					schd_sense_step = 0;
					break;
				default:
					break;
			}
			*/
			if (schd_sense_step == 0) {
				//result_line6 = capsense_sense_pin(6);
				result_line6 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L6, 6), cal_line6, 8);
				//capsense_filter(&result_line6, capsense_offset(capsense_sense_pin(6), cal_line6, 3));
				schd_sense_step++;

			}
			else if (schd_sense_step == 1) {
				result_line5 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L5, 5), cal_line5, 8);
				schd_sense_step++;

			}
			else if (schd_sense_step == 2) {
				//result_line2 = capsense_sense_pin(2);
				result_line2 = capsense_offset(capsense_sense(CAPSENSE_SENSE_PORT_L2, 2), cal_line2, 8);
				//capsense_filter(&result_line2, capsense_offset(capsense_sense_pin(2), cal_line2, 3));
				schd_sense_step++;

			}
			else if (schd_sense_step == 3) {
				//slider_output = capsense_slider3(result_line6, result_line5, result_line2, 30);
				capsense_slider3_scroll(result_line6, result_line5, result_line2, 30, &slider_output, &slider_memory, 4, 4);
				//slider_output = capsense_discretize(slider_output, 3);
				
				schd_sense_step++;

			}
				/*
			else if (schd_sense_step == 4) {
				capsense_filter(&slider_output, slider_pre_output);
				schd_sense_step++;
			}
				*/
			else if (schd_sense_step >= 4) {
				//capsense_filter(&slider_output, slider_pre_output);
				led_ctrl_t0 = (slider_output >> 2) * DELAY_US_TIME;		// on-time
				led_ctrl_t1 = ((255 - slider_output) >> 2) * DELAY_US_TIME;	// off-time
				schd_sense_step = 0;
			}
			#endif
			schd_sense_t = SysTick->CNT;
		}
		if (SysTick->CNT - schd_led_t > schd_led_i) {
			switch (schd_led_step) {
				case 0:
					if (led_ctrl_t0) {
						GPIOC->BSHR = 1 << 3;
					}
					schd_led_i = led_ctrl_t0;
					schd_led_step++;
					break;
				case 1:
					GPIOC->BSHR = 1 << (3 + 16);
					schd_led_i = led_ctrl_t1;
					schd_led_step = 0;
					break;

			}
			schd_led_t = SysTick->CNT;
		}
		if (SysTick->CNT - schd_log_t > SCHD_log_i) {
			#if SELECT_SLIDER2 == 1
			LOG("%03u %03u %03u\r\n", result_line6, result_line5, slider_output);
			#endif
			#if SELECT_SLIDER3 == 1
			LOG("%03u %03u %03u %03u\r\n", result_line6, result_line5, result_line2, slider_output);
			#endif
			GPIOD->OUTDR ^= (1 << 4);
			schd_log_t = SysTick->CNT;
		}
	}
}
