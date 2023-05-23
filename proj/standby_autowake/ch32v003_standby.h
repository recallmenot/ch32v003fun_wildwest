//######## necessities

// include guards
#ifndef CH32V003_STANDBY_H
#define CH32V003_STANDBY_H

// includes
#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"



/*######## library description


*/


/*######## library usage and configuration


*/


/*######## features: use your desired before #including this library!

to have standby_enter() automatically restore the HSI speed to 48MHz after standby (else it will be 24MHz)
#define STANDBY_RESTORE_HSI_48MHz
*/


//######## references: use these

#define STANDBY_GPIO_PORT_A 0b00
#define STANDBY_GPIO_PORT_C 0b10
#define STANDBY_GPIO_PORT_D 0b11

//######## function overview (declarations): use these!




//######## internal function declarations



//######## internal variables

#define STANDBY_GPIO_REGISTER_BASE_I 1024
#define STANDBY_GPIO_CFGLR_OFFSET 0x00
#define STANDBY_GPIO_OUTDR_OFFSET 0x0C
#define STANDBY_GPIO_BSHR_OFFSET 0x10



//######## preprocessor macros



//######## preprocessor #define requirements

#ifndef STANDBY_RESTORE_HSI_48MHz
#pragma message "HSI will be 24MHz after waking from standby, is this intentional? Use #define STANDBY_RESTORE_HSI_48MHz to change this."
#endif


//######## small function definitions, static inline



static inline void standby_autowakeup_init(const uint8_t window_count, const uint8_t PWR_AWU_Prescaler) {
	// enable power interface module clock
	RCC->APB1PCENR |= RCC_APB1Periph_PWR;

	// enable low speed oscillator (LSI)
	RCC->RSTSCKR |= RCC_LSION;
	while ((RCC->RSTSCKR & RCC_LSIRDY) == 0) {}

	// enable AutoWakeUp event
	EXTI->EVENR |= EXTI_Line9;
	EXTI->FTENR |= EXTI_Line9;

	// configure AWU prescaler
	PWR->AWUPSC |= PWR_AWU_Prescaler;

	// configure AWU window comparison value
	PWR->AWUWR &= ~0x3f;
	PWR->AWUWR |= window_count;

	// enable AWU
	PWR->AWUCSR |= (1 << 1);

	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;

	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);
}

static inline void standby_gpio_init() {
	// enable alternate IO function module clock
	RCC->APB2PCENR |= RCC_AFIOEN;

	// select standby on power-down
	PWR->CTLR |= PWR_CTLR_PDDS;

	// peripheral interrupt controller send to deep sleep
	PFIC->SCTLR |= (1 << 2);
}

static inline void standby_gpio_assign_pin(uint8_t standby_gpio_port, uint8_t pin_number) {
	// enable the correct GPIO port register
	RCC->APB2PCENR |= (1 << (2 + standby_gpio_port));

	// configure button as input, pullup
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (STANDBY_GPIO_REGISTER_BASE_I *standby_gpio_port) + STANDBY_GPIO_CFGLR_OFFSET) &= ~(0xf << (pin_number * 4));
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (STANDBY_GPIO_REGISTER_BASE_I *standby_gpio_port) + STANDBY_GPIO_CFGLR_OFFSET) |= (GPIO_CNF_IN_PUPD) << (pin_number * 4);
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (STANDBY_GPIO_REGISTER_BASE_I * standby_gpio_port) + STANDBY_GPIO_BSHR_OFFSET) = (1 << pin_number);

	// assign pin 2 interrupt from portD (0b11) to EXTI channel 2
	AFIO->EXTICR |= (uint32_t)(standby_gpio_port << (pin_number * 2));

	// enable line2 interrupt event
	EXTI->EVENR |= (1 << pin_number);
	EXTI->FTENR |= (1 << pin_number);
}

static inline void standby_enter() {
	__WFE();
	// after standby, core clock will be 24MHz HSI
	#ifdef STANDBY_RESTORE_HSI_48MHz
	// restore clock to full speed
	SystemInit48HSI();
	#endif
}





//######## small internal function definitions, static inline



//######## implementation block
//#define CH32V003_LIBRARYNAME_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_LIBRARYNAME_IMPLEMENTATION)



#endif // CH32V003_LIBRARYNAME_IMPLEMENTATION
#endif // CH32V003_LIBRARYNAME_H
