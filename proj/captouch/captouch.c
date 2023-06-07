// DEMO CONFIGURATION

#define DEMO_SELECT_BUTTONS	0
#define DEMO_SELECT_SLIDER	1

#if (((DEMO_SELECT_BUTTONS + DEMO_SELECT_SLIDER) < 1) || ((DEMO_SELECT_BUTTONS + DEMO_SELECT_SLIDER) > 1))
#error "please enable one of the demos by setting it to 1 and the other to 0"
#endif



#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"

#include "../../lib/ch32v003_GPIO_branchless.h"

#define BTNi_i_DEBOUNCE		Ticks_from_Ms(10)
#define BTNi_i_HOLD		Ticks_from_Ms(1000)
#define BTNi_i_HOLD_REPEAT	Ticks_from_Ms(250)
#include "../BTN/BTN_interpret.h"

#include <stdio.h>

#define LOG_LVL 1
#if LOG_LVL >= 1
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define CAPTOUCH_SENSE_PORT_L2 GPIOD
#define CAPTOUCH_SENSE_PORT_L5 GPIOC
#define CAPTOUCH_SENSE_PORT_L6 GPIOC
#define CAPTOUCH_SENSE_THRESHOLD 5
#define CAPTOUCH_BTN_CONTACT_THRESHOLD 30
#define CAPTOUCH_SLIDER_CONTACT_THRESHOLD 30
#include "ch32v003_captouch.h"



//#define SCHD_sense_i 967 * DELAY_US_TIME
//#define SCHD_sense_i 2141 * DELAY_US_TIME
#define SCHD_sense_i 373 * DELAY_US_TIME
#define SCHD_log_i 73 * DELAY_MS_TIME



int main()
{
	SystemInit48HSI();
	#if LOG_LVL >= 1
	SetupUART( UART_BRR );
	#endif

	LOG("\r\n\r\ncapTouch TM (C) patent pending rights reserved\r\n\r\n");

	GPIO_portEnable(GPIO_port_C);
	GPIO_portEnable(GPIO_port_D);

	GPIO_pinMode(GPIO_port_C, 4, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);

	GPIO_pinMode(GPIO_port_D, 4, GPIO_pinMode_O_pushPullMux, GPIO_Speed_50MHz);
	GPIO_tim2_map(GPIO_tim2_output_set_0__D4_D3_C0_D7);
	GPIO_tim2_init();
	GPIO_tim2_enableCH(1);

	captouch_assign_sense();

	// give the user time to open the terminal connection
	Delay_Ms(2000);
	LOG("preparing to calibrate\r\n");

	GPIOC->BSHR = (1 << 4);
	LOG("EXTI7_0 priotirty is %x\r\n", NVIC->IPRIOR[EXTI7_0_IRQn]);


	//uint16_t counter = 0;

	uint8_t result_line6;
	uint8_t result_line5;
	uint8_t result_line2;


#if DEMO_SELECT_BUTTONS == 1
	uint32_t btn_C6_time_memory = 0;
	uint32_t btn_C5_time_memory = 0;
	uint32_t btn_D2_time_memory = 0;
	uint8_t btn_C6_state_memory = 0;
	uint8_t btn_C5_state_memory = 0;
	uint8_t btn_D2_state_memory = 0;
	uint16_t btn_output = 0;
	#define led_step_smol	4
	#define led_step_big	64
#elif DEMO_SELECT_SLIDER == 1
	uint8_t slider_output = 0;
	int16_t slider_memory;
#endif

	LOG("CAL in 2000ms\r\n");
	uint8_t cal_line6 = captouch_sense(CAPTOUCH_SENSE_PORT_L6, 6);
	Delay_Ms(1);
	uint8_t cal_line5 = captouch_sense(CAPTOUCH_SENSE_PORT_L5, 5);
	Delay_Ms(1);
	uint8_t cal_line2 = captouch_sense(CAPTOUCH_SENSE_PORT_L2, 2);

	LOG("calibration %03u %03u %03u\r\n", cal_line6, cal_line5, cal_line2);
	Delay_Ms(2000);
	GPIOC->BSHR = (1 << (16 + 4));


	uint8_t schd_sense_step = 0;

	uint32_t schd_sense_t = 0;
	uint32_t schd_log_t = 0;
	LOG("entering capTouch TM (C) patent pending rights reserved loop\r\n");
	for (;;) {
		if (SysTick->CNT - schd_sense_t > SCHD_sense_i) {
#if DEMO_SELECT_BUTTONS == 1

			switch (schd_sense_step) {
				case 0:
					result_line6 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L6, 6), cal_line6, 4);
					BTNi_interpret_state(BTNi_analog_threshold(result_line6, 30), &btn_C6_time_memory, &btn_C6_state_memory);
					schd_sense_step++;
					break;
				case 1:
					result_line5 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L5, 5), cal_line5, 4);
					BTNi_interpret_state(BTNi_analog_threshold(result_line5, 30), &btn_C5_time_memory, &btn_C5_state_memory);
					schd_sense_step++;
					break;
				case 2:
					result_line2 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L2, 2), cal_line2, 4);
					BTNi_interpret_state(BTNi_analog_threshold(result_line2, 30), &btn_D2_time_memory, &btn_D2_state_memory);
					schd_sense_step++;
					break;
				case 3:
					switch (btn_C6_state_memory) {
						case BTNi_state_pushed:
							btn_output = (btn_output >= led_step_smol) ? (btn_output - led_step_smol) : 0;
							break;
						case BTNi_state_held:
							btn_output = (btn_output >= led_step_big) ? (btn_output - led_step_big) : 0;
							break;
					}
					switch (btn_C5_state_memory) {
						case BTNi_state_held:
							btn_output = 0;
							break;
					}
					switch (btn_D2_state_memory) {
						case BTNi_state_pushed:
							btn_output = (btn_output <= (1023 - led_step_smol)) ? (btn_output + led_step_smol) : 1023;
							break;
						case BTNi_state_held:
							btn_output = (btn_output <= (1023 - led_step_big)) ? (btn_output + led_step_big) : 1023;
							break;
					}
					schd_sense_step++;
					break;
				case 4:
					GPIO_tim2_analogWrite(1, btn_output);
					schd_sense_step = 0;
					break;
			}
#elif DEMO_SELECT_SLIDER == 1
			// with switch case here, 5 statements = 2600 bytes, 6 statements = 18000 bytes! maybe gcc12 bug? -> replaced with if else (works)
			if (schd_sense_step == 0) {
				result_line6 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L6, 6), cal_line6, 4);
				schd_sense_step++;

			}
			else if (schd_sense_step == 1) {
				result_line5 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L5, 5), cal_line5, 4);
				schd_sense_step++;

			}
			else if (schd_sense_step == 2) {
				result_line2 = captouch_value_clean(captouch_sense(CAPTOUCH_SENSE_PORT_L2, 2), cal_line2, 4);
				schd_sense_step++;

			}
			else if (schd_sense_step == 3) {
				captouch_slider3_scroll(result_line6, result_line5, result_line2, &slider_output, &slider_memory, 8, 1);
				schd_sense_step++;

			}
			else if (schd_sense_step >= 4) {
				GPIO_tim2_analogWrite(1, ((uint16_t)(slider_output) << 2));
				schd_sense_step = 0;
			}
#endif
			schd_sense_t = SysTick->CNT;
		}
		if (SysTick->CNT - schd_log_t > SCHD_log_i) {
#if DEMO_SELECT_BUTTONS == 1
			LOG("%03u %03u %03u %04u\r\n", result_line6, result_line5, result_line2, btn_output);
#elif DEMO_SELECT_SLIDER == 1
			LOG("%03u %03u %03u %03u\r\n", result_line6, result_line5, result_line2, slider_output);
#endif
			GPIOD->OUTDR ^= (1 << 4);
			schd_log_t = SysTick->CNT;
		}
	}
}
