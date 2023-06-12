// 2023-06-12 recallmenot
#define DEMO_brightness_8bit			0		// 
#define DEMO_brightness_10bit			0
#define DEMO_colorwheel_8bit			0		// (1024 * 6) hues
#define DEMO_colorwheel_10bit			0		// (1024 * 6) hues
#define DEMO_spectrum_8bit			0		// 256 hues
#define DEMO_spectrum_10bit			0		// 1024 hues
#define DEMO_rainbow_8bit			0		// 256 hues
#define DEMO_rainbow_10bit			1		// 1024 hues

// perceived brightness does not translate linearly to PWM values!
// for low values, +1 is a big increase, for high values +1 becomes imperceivable
// this also affects color mixing of RGB values
// turn off to read out raw RGB, turn on to view PWM values on the LED
// this is called "gamma" on displays
#define DEMO_ENABLE_RGB_to_PWM			1

#include"colortargets.h"
// only display one certain color
#define DEMO_ONLY_testcolor			0
#define testcolor_colorwheel			colort_c10_magneta
#define testcolor_spectrum			colort_s10_magneta
#define testcolor_rainbow			colort_r10_magneta

#if ((DEMO_brightness_8bit + DEMO_brightness_10bit + DEMO_colorwheel_8bit + DEMO_colorwheel_10bit + DEMO_spectrum_8bit + DEMO_spectrum_10bit + DEMO_rainbow_8bit + DEMO_rainbow_10bit) > 1 \
  || (DEMO_brightness_8bit + DEMO_brightness_10bit + DEMO_colorwheel_8bit + DEMO_colorwheel_10bit + DEMO_spectrum_8bit + DEMO_spectrum_10bit + DEMO_rainbow_8bit + DEMO_rainbow_10bit) < 1)
#error "please enable ONE of the demos by setting it to 1 and the others to 0"
#endif



#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "ch32v003fun.h"

#include "ch32v003_GPIO_branchless.h"
#define COLORMATH_IMPLEMENTATION
#include "colormath.h"

#include <stdio.h>



#define LOG_EN					0
#if LOG_EN == 1
#define LOG(...)	printf(__VA_ARGS__)
#define LOG_ratios(R, G, B)	print_ratios(R, G, B)
#else
#define LOG(...)
#define LOG_ratios(R, G, B)
#endif

uint16_t get_ratio(uint32_t A, uint32_t B) {
	return ((A * 1000) / (A + B));
}

void print_ratios(uint16_t R, uint16_t G, uint16_t B) {
	LOG("RG%04u ", get_ratio(R, G));
	LOG("GB%04u ", get_ratio(G, B));
	LOG("BR%04u   ", get_ratio(B, R));
}



int main() {
	SystemInit48HSI();
#if LOG_EN == 1
	SetupUART( UART_BRR );
#endif
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
#if DEMO_brightness_8bit == 1
#elif DEMO_colorwheel == 1
#elif DEMO_spectrum_8bit == 1
#endif
	


	while (1) {
#if DEMO_brightness_8bit == 1
		#define NEXT_DELAY	63
		static uint16_t led_i = 0;
	#if DEMO_ENABLE_RGB_to_PWM == 1
		uint8_t led_b = colorm_brightness_to_pwm(led_i, 8);
	#else
		uint8_t led_b = led_i;
	#endif
		GPIO_tim2_analogWrite(1, ((uint16_t)led_b) << 2);
		GPIO_tim2_analogWrite(2, ((uint16_t)led_b) << 2);
		GPIO_tim2_analogWrite(3, ((uint16_t)led_b) << 2);

		// led_b = led_b % bit_top(8,1); // less code but 128 bytes larger
		if (led_i < bit_top(8,1)) {
			led_i++;
		}
		else {
			led_i = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_brightness_10bit == 1
		#define NEXT_DELAY	16
		static uint16_t led_i = 0;
	#if DEMO_ENABLE_RGB_to_PWM == 1
		uint16_t led_b = colorm_brightness_to_pwm(led_i, 10);
	#else
		uint16_t led_b = led_i;
	#endif
		GPIO_tim2_analogWrite(1, led_b);
		GPIO_tim2_analogWrite(2, led_b);
		GPIO_tim2_analogWrite(3, led_b);

		// led_b = led_b % bit_top(8,1); // less code but 128 bytes larger
		if (led_i < bit_top(10,1)) {
			led_i++;
		}
		else {
			led_i = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_colorwheel_8bit == 1
		#define NEXT_DELAY	11
		uint8_t led1_RGB[3] = {};
		static uint16_t led1_h = 0;
	#if DEMO_ONLY_testcolor == 1
		colorm_colorwheel_8bit(testcolor_colorwheel, led1_RGB);
	#else
		colorm_colorwheel_8bit(led1_h, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 8);
	#endif
		GPIO_tim2_analogWrite(1, ((uint16_t)led1_RGB[0]) << 2);
		GPIO_tim2_analogWrite(2, ((uint16_t)led1_RGB[1]) << 2);
		GPIO_tim2_analogWrite(3, ((uint16_t)led1_RGB[2]) << 2);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%03u G%03u B%03u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		if (led1_h < (bit_top(8, 6))) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_colorwheel_10bit == 1
		#define NEXT_DELAY	3
		uint16_t led1_RGB[3] = {};
		static uint16_t led1_h = 0;
	#if DEMO_ONLY_testcolor == 1
		colorm_colorwheel_10bit(testcolor_colorwheel, led1_RGB);
	#else
		colorm_colorwheel_10bit(led1_h, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 10);
	#endif
		GPIO_tim2_analogWrite(1, led1_RGB[0]);
		GPIO_tim2_analogWrite(2, led1_RGB[1]);
		GPIO_tim2_analogWrite(3, led1_RGB[2]);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%04u G%04u B%04u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		if (led1_h < (bit_top(10, 6))) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_spectrum_8bit == 1
		#define NEXT_DELAY	63
		uint8_t led1_RGB[3] = {};
		static uint8_t led1_h = 0;
		//hsl_to_rgb_8bit(170, 255, 128, led1_RGB);
	#if DEMO_ONLY_testcolor == 1
		colorm_spectrum_8bit(testcolor_spectrum, 255, 255, led1_RGB);
	#else
		colorm_spectrum_8bit(led1_h, 255, 255, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 8);
	#endif
		GPIO_tim2_analogWrite(1, ((uint16_t)led1_RGB[0]) << 2);
		GPIO_tim2_analogWrite(2, ((uint16_t)led1_RGB[1]) << 2);
		GPIO_tim2_analogWrite(3, ((uint16_t)led1_RGB[2]) << 2);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%03u G%03u B%03u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		led1_h++;
		/*
		if (led1_h < (256 - 1)) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		*/
		Delay_Ms(NEXT_DELAY);
#elif DEMO_spectrum_10bit == 1
		#define NEXT_DELAY	16
		uint16_t led1_RGB[3] = {};
		static uint16_t led1_h = 0;
		//hsl_to_rgb_8bit(170, 255, 128, led1_RGB);
	#if DEMO_ONLY_testcolor == 1
		colorm_spectrum_10bit(testcolor_spectrum, 1023, 1023, led1_RGB);
	#else
		colorm_spectrum_10bit(led1_h, 1023, 1023, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 10);
	#endif
		//color_RGB_to_PWM(led1_RGB, 10);
		GPIO_tim2_analogWrite(1, led1_RGB[0]);
		GPIO_tim2_analogWrite(2, led1_RGB[1]);
		GPIO_tim2_analogWrite(3, led1_RGB[2]);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%04u G%04u B%04u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		if (led1_h < (1024 - 1)) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_rainbow_8bit == 1
		#define NEXT_DELAY	63
		uint8_t led1_RGB[3] = {};
		static uint8_t led1_h = 0;
		//hsl_to_rgb_8bit(170, 255, 128, led1_RGB);
	#if DEMO_ONLY_testcolor == 1
		colorm_rainbow_8bit(testcolor_rainbow, 255, 255, led1_RGB);
	#else
		colorm_rainbow_8bit(led1_h, 255, 255, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 8);
	#endif
		GPIO_tim2_analogWrite(1, ((uint16_t)led1_RGB[0]) << 2);
		GPIO_tim2_analogWrite(2, ((uint16_t)led1_RGB[1]) << 2);
		GPIO_tim2_analogWrite(3, ((uint16_t)led1_RGB[2]) << 2);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%03u G%03u B%03u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		if (led1_h < (256 - 1)) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		Delay_Ms(NEXT_DELAY);
#elif DEMO_rainbow_10bit == 1
		#define NEXT_DELAY	16
		uint16_t led1_RGB[3] = {};
		static uint16_t led1_h = 0;
	#if DEMO_ONLY_testcolor == 1
		colorm_rainbow_10bit(testcolor_rainbow, 1023, 1023, led1_RGB);
	#else
		colorm_rainbow_10bit(led1_h, 1023, 1023, led1_RGB);
	#endif
	#if DEMO_ENABLE_RGB_to_PWM == 1
		colorm_RGB_to_PWM(led1_RGB, 10);
	#endif
		GPIO_tim2_analogWrite(1, led1_RGB[0]);
		GPIO_tim2_analogWrite(2, led1_RGB[1]);
		GPIO_tim2_analogWrite(3, led1_RGB[2]);
		LOG_ratios(led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		LOG("h%04u  R%04u G%04u B%04u\r\n", led1_h, led1_RGB[0], led1_RGB[1], led1_RGB[2]);
		if (led1_h < (1024 - 1)) {
			led1_h++;
		}
		else {
			led1_h = 0;
		}
		Delay_Ms(NEXT_DELAY);
#endif
	}
}
