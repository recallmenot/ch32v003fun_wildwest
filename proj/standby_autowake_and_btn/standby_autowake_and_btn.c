// based on https://paste.sr.ht/blob/b9b4fb45cbc70f2db7e31a77a6ef7dd2a7f220fb
// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000

#include "../../ch32v003fun/ch32v003fun.h"

#define STANDBY_RESTORE_HSI_48MHz
#include "../../lib/ch32v003_standby.h"

#include <stdio.h>

#define APB_CLOCK SYSTEM_CORE_CLOCK

/* somehow this ISR won't get called??
void AWU_IRQHandler( void ) __attribute__((interrupt));
void AWU_IRQHandler( void ) {
	GPIOD->OUTDR ^= (1 << 4);
}
*/

int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	printf("\r\n\r\nlow power example\r\n\r\n");

	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);
	GPIOD->OUTDR |= (1 << 4);

	// give the user time to open the terminal connection
	//Delay_Ms(5000);
	//printf("5000ms wait over\r\n");

	standby_autowakeup_init(63, PWR_AWU_Prescaler_10240);

	standby_gpio_init();
	standby_gpio_assign_button(GPIOD, 2);
	standby_gpio_assign_pin(STANDBY_GPIO_PORT_D, 2, STANDBY_TRIGGER_DIRECTION_FALLING);


	uint16_t counter = 0;
	printf("entering sleep loop\r\n");

	for (;;) {
		standby_enter();
		printf("\r\nawake, %u\r\n", counter++);
		GPIOD->OUTDR ^= (1 << 4);
	}
}
