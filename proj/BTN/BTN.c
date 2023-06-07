// blink, but with arduino-like HAL
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "../../ch32v003fun/ch32v003fun.h"

#define CH32V003_GPIO_ITER_IMPLEMENTATION
#include "../../lib/ch32v003_GPIO_branchless.h"
#define BTNi_i_DEBOUNCE		Ticks_from_Ms(10)
#define BTNi_i_HOLD		Ticks_from_Ms(1000)
#define BTNi_i_HOLD_REPEAT	Ticks_from_Ms(250)
#include "BTN_interpret.h"

#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

uint32_t count;

int main() {
	SystemInit48HSI();
	GPIO_portEnable(GPIO_port_D);
	GPIO_portEnable(GPIO_port_C);

	GPIO_pinMode(GPIO_port_D, 3, GPIO_pinMode_I_pullUp, GPIO_SPEED_IN);

	for (int i = 0; i <= 7; i++) {
		GPIO_pinMode(GPIO_port_C, i, GPIO_pinMode_O_pushPull, GPIO_Speed_10MHz);
	}
	uint32_t btn_time_memory;
	uint8_t btn_state_memory;
	while (1) {
		BTNi_interpret_state(!GPIO_digitalRead(GPIO_port_D, 3), &btn_time_memory, &btn_state_memory);
		for (uint8_t i = 0; i <= 7; i++) {
			if (btn_state_memory == i) {
				GPIO_digitalWrite_hi(GPIO_port_C, i);
			}
			else {
				GPIO_digitalWrite_lo(GPIO_port_C, i);
			}
		}
		Delay_Ms(10);
	}
}
