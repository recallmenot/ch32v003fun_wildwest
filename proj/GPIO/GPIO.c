// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"

#include "../../lib/ch32v003_GPIO_branchless.h"

#include <stdio.h>


uint32_t count;

int main() {
	SystemInit48HSI();

	// Enable GPIO ports
	GPIO_portEnable(GPIO_port_C);
	GPIO_portEnable(GPIO_port_D);

	// GPIO C0 - C7 Push-Pull
	for (int i = 0; i <= 7; i++) {
		GPIO_pinMode(GPIO_port_C, i, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	}

	// GPIO D4 Push-Pull
	GPIO_pinMode(GPIO_port_D, 4, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);

	while (1) {
		// Turn on pins
		GPIO_digitalWrite(GPIO_port_D, 4, high);
		Delay_Ms(1000);
		// Turn off pins
		GPIO_digitalWrite(GPIO_port_D, 4, low);
		Delay_Ms(250);
		for (int i = 0; i <= 7; i++) {
			GPIO_digitalWrite_hi(GPIO_port_D, i);
			Delay_Ms(50);
		}
		for (int i = 7; i >= 0; i--) {
			GPIO_digitalWrite_lo(GPIO_port_D, i);
			Delay_Ms(50);
		}
		Delay_Ms(1000);
	}
}
