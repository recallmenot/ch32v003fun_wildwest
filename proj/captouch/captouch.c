// DEMO CONFIGURATION

// use the sensing surfaces either as 3 buttons or a slider (set 1 and 0 to select)
#define DEMO_SELECT_BUTTONS	0
#define DEMO_SELECT_SLIDER	1

// here you can flip the timer polarity to suit your LED connection
// set to 1 if PD4 is connected to LED cathode (-), 0 if PD4 is connected to LED anode (+)
#define LED_CATHODE_ON_PIN	0



#if ((DEMO_SELECT_BUTTONS < 1) && (DEMO_SELECT_SLIDER < 1))
#error "please enable one of the DEMOS"
#endif



#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"

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



void t2pwm_init( void )
{
	// Enable GPIOD and TIM2
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB1PCENR |= RCC_APB1Periph_TIM2;

	// PD4 is T2CH1, 10MHz Output alt func, push-pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP_AF)<<(4*4);
	
	// Reset TIM2 to init all regs
	RCC->APB1PRSTR |= RCC_APB1Periph_TIM2;
	RCC->APB1PRSTR &= ~RCC_APB1Periph_TIM2;
	
	// SMCFGR: default clk input is CK_INT
	// set TIM2 clock prescaler divider 
	TIM2->PSC = 0x0000;
	// set PWM total cycle width
	TIM2->ATRLR = 255;
	
	// for channel 1 and 2, let CCxS stay 00 (output), set OCxM to 110 (PWM I)
	// enabling preload causes the new pulse width in compare capture register only to come into effect when UG bit in SWEVGR is set (= initiate update) (auto-clears)
	TIM2->CHCTLR1 |= TIM_OC1M_2 | TIM_OC1M_1 | TIM_OC1PE;

	// CTLR1: default is up, events generated, edge align
	// enable auto-reload of preload
	TIM2->CTLR1 |= TIM_ARPE;

	// Enable CH1 output, positive pol
	#if LED_CATHODE_ON_PIN == 1
	TIM2->CCER |= TIM_CC1E | TIM_CC1P;
	#else
	TIM2->CCER |= TIM_CC1E;
	#endif

	// initialize counter
	TIM2->SWEVGR |= TIM_UG;

	// Enable TIM2
	TIM2->CTLR1 |= TIM_CEN;
}


/*
 * set timer channel PW
 */
void t2pwm_setpw(uint8_t chl, uint16_t width)
{
	switch(chl&3)
	{
		case 0: TIM2->CH1CVR = width; break;
		case 1: TIM2->CH2CVR = width; break;
		case 2: TIM2->CH3CVR = width; break;
		case 3: TIM2->CH4CVR = width; break;
	}
	TIM2->SWEVGR |= TIM_UG; // load new value in compare capture register
}

int main()
{
	SystemInit48HSI();
	#if LOG_LVL >= 1
	SetupUART( UART_BRR );
	#endif

	printf("\r\n\r\ncapTouch TM (C) patent pending rights reserved\r\n\r\n");

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

	// GPIO D4 timer2 PWM for LED
	t2pwm_init();

	// GPIO C4 Push-Pull
	GPIOC->CFGLR &= ~(0xf<<(4*4));
	GPIOC->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	GPIOC->OUTDR |= (1 << 3);

	captouch_assign_sense();

	// give the user time to open the terminal connection
	Delay_Ms(2000);


	GPIOC->BSHR = (1 << 4);
	printf("EXTI7_0 priotirty is %x\r\n", NVIC->IPRIOR[EXTI7_0_IRQn]);


	//uint16_t counter = 0;

	uint8_t result_line6;
	uint8_t result_line5;
	uint8_t result_line2;

	LOG("CAL in 2000ms\r\n");
	uint8_t cal_line6 = captouch_sense(CAPTOUCH_SENSE_PORT_L6, 6);
	Delay_Ms(1);
	uint8_t cal_line5 = captouch_sense(CAPTOUCH_SENSE_PORT_L5, 5);
	Delay_Ms(1);
	uint8_t cal_line2 = captouch_sense(CAPTOUCH_SENSE_PORT_L2, 2);

	LOG("%03u %03u %03u cal\r\n", cal_line6, cal_line5, cal_line2);
	Delay_Ms(2000);
	GPIOC->BSHR = (1 << (16 + 4));

	uint8_t slider_output = 0;
	int16_t slider_memory;

	uint8_t schd_sense_step = 0;

	uint32_t schd_sense_t = 0;
	uint32_t schd_log_t = 0;
	LOG("entering capTouch TM (C) patent pending rights reserved loop\r\n");
	for (;;) {
		if (SysTick->CNT - schd_sense_t > SCHD_sense_i) {
			#if DEMO_SELECT_BUTTONS == 1
			switch (schd_sense_step) {
				case 0:
					//result_line6 = captouch_sense_pin(6);
					result_line6 = captouch_value_clean(captouch_sense_pin(6), cal_line6, 8);
					//captouch_filter(&result_line6, captouch_value_clean(captouch_sense_pin(6), cal_line6, 3));
					schd_sense_step++;
					break;
				case 1:
					//result_line5 = captouch_sense_pin(5);
					result_line5 = captouch_value_clean(captouch_sense_pin(5), cal_line5, 8);
					//captouch_filter(&result_line5, captouch_value_clean(captouch_sense_pin(5), cal_line5, 3));
					schd_sense_step++;
					break;
				case 2:
					captouch_slider2_scroll(result_line6, result_line5, 50, &slider_output, &slider_memory, 16, 8);
					//slider_output = captouch_discretize(slider_output, 3);
					schd_sense_step++;
					break;
				case 3:
					led_ctrl_t0 = (slider_output >> 2) * DELAY_US_TIME;		// on-time
					led_ctrl_t1 = ((255 - slider_output) >> 2) * DELAY_US_TIME;	// off-time
					schd_sense_step = 0;
					break;
			}
			#endif
			#if DEMO_SELECT_SLIDER == 1
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
				t2pwm_setpw(0, slider_output);
				schd_sense_step = 0;
			}
			#endif
			schd_sense_t = SysTick->CNT;
		}
		if (SysTick->CNT - schd_log_t > SCHD_log_i) {
			#if DEMO_SELECT_BUTTONS == 1
			LOG("%03u %03u %03u\r\n", result_line6, result_line5, result_line2);
			#endif
			#if DEMO_SELECT_SLIDER == 1
			LOG("%03u %03u %03u %03u\r\n", result_line6, result_line5, result_line2, slider_output);
			#endif
			GPIOD->OUTDR ^= (1 << 4);
			schd_log_t = SysTick->CNT;
		}
	}
}
