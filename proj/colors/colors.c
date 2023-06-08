// 2023-06-07 recallmenot

#define DEMO_colorwheel				1

#if ((DEMO_GPIO_blink + DEMO_GPIO_out + DEMO_GPIO_in_btn + DEMO_ADC_bragraph + DEMO_colorwheel) > 1 \
  || (DEMO_GPIO_blink + DEMO_GPIO_out + DEMO_GPIO_in_btn + DEMO_ADC_bragraph + DEMO_colorwheel) < 1)
#error "please enable ONE of the demos by setting it to 1 and the others to 0"
#endif


#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"

#include "ch32v003_GPIO_branchless.h"
#define COLORMATH_IMPLEMENTATION
#include "colormath.h"

#include <stdio.h>



int main() {
	SystemInit48HSI();
#if DEMO_colorwheel == 1
	//SetupUART( UART_BRR );
	GPIO_portEnable(GPIO_port_C);
	GPIO_portEnable(GPIO_port_D);
	GPIO_tim2_map(GPIO_tim2_output_set_0__D4_D3_C0_D7);
	GPIO_pinMode(GPIO_port_D, 4, GPIO_pinMode_O_pushPullMux, GPIO_Speed_50MHz);
	GPIO_pinMode(GPIO_port_D, 3, GPIO_pinMode_O_pushPullMux, GPIO_Speed_50MHz);
	GPIO_pinMode(GPIO_port_C, 0, GPIO_pinMode_O_pushPullMux, GPIO_Speed_50MHz);
	GPIO_tim2_init();
	GPIO_tim2_enableCH(1);
	GPIO_tim2_enableCH(2);
	GPIO_tim2_enableCH(3);
	/*
	GPIO_tim1_map(GPIO_tim1_output_set_0__D2_A1_C3_C4__D0_A2_D1);
	GPIO_tim1_init();
	GPIO_tim1_enableCH(3);
	GPIO_tim1_enableCH(4);
	*/
#endif
	


	while (1) {
#if DEMO_colorwheel == 1
		#define COLOR_NEXT_DELAY	5
		uint16_t led_1_channels[3];
		static uint16_t led1_h = 0;
		colorwheel_8(led1_h, &led_1_channels[0], &led_1_channels[1], &led_1_channels[2]);
		GPIO_tim2_analogWrite(2, led_1_channels[0]);
		GPIO_tim2_analogWrite(3, led_1_channels[1]);
		GPIO_tim2_analogWrite(4, led_1_channels[2]);
		//GPIO_tim1_analogWrite(1, led_PW[4]);
		GPIO_digitalWrite(GPIO_port_D, 4, low);
		led1_h = (led1_h < bit_top(8, 6)) ? (led1_h + 1) : 0;
		Delay_Us(COLOR_NEXT_DELAY);
#endif
	}
}
