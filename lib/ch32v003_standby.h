//######## necessities

// include guards
#ifndef CH32V003_STANDBY_H
#define CH32V003_STANDBY_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description

This library makes sending the CH32V003 to sleep easy.
You can use the auto-wakeup timer, a pin or both, to exit standby and resume operation.

Expect around 10uA total power consumption in standby.
The LSI oscillator used for auto-wakeup uses 0.6uA.

*/



/*######## library usage and configuration

## auto-wakeup timer

The auto-wakeup timer takes two arguments to set the delay:
a clock prescaler divider and a count which, when reached, triggers the auto-wakeup event.

The autowakeup delay can be calculated by:

t = AWUWR / (fLSI / AWUPSC)

Where AWUWR (the count) can be 1 to 63, fLSI is always 128000 and AWUPSC (the prescaler divider), for practical purposes is 2048, 4096, 10240 or 61440, though lower values are possible.
The maximum auto-wakeup delay is 30s.

## wake by GPIO pins

When using GPIO pins to wake, there is a limitation on which can be set to this task simultaneously.
A pin can only be connected to its preassigned event channel:
PA1, PC1 and PD1 can only be connected to event channel 1, so you will have to pick one of the three.
Likewise, one of PA2, PC2 and PD2 can be connected to event channel 2, etc.

*/



/*######## features: use your desired before #including this library!

to have standby_enter() automatically restore the HSI speed to 48MHz after standby (else it will be 24MHz)
#define STANDBY_RESTORE_HSI_48MHz
*/



//######## references: use these

// port definitions for the standby_gpio_assign functions
#define STANDBY_GPIO_PORT_A 0b00
#define STANDBY_GPIO_PORT_C 0b10
#define STANDBY_GPIO_PORT_D 0b11

// port definitions for the standby_gpio_assign_pin functions
#define STANDBY_TRIGGER_DIRECTION_RISING	0b01
#define STANDBY_TRIGGER_DIRECTION_FALLING	0b10
#define STANDBY_TRIGGER_DIRECTION_BOTH		0b11



//######## function overview (declarations): use these!

// initialize standby with auto-wakeup timer
static inline void standby_autowakeup_init(const uint8_t window_count, const uint8_t PWR_AWU_Prescaler);

// initialize standby with GPIO wakeup
static inline void standby_gpio_init();

// configure GPIO pin for wakeup
static inline void standby_gpio_assign_pin(uint8_t standby_gpio_port, uint8_t pin_number, uint8_t standby_trigger_direction);
// configure GPIO pin with pull-up for wakeup
static inline void standby_gpio_assign_button(GPIO_TypeDef* gpio_port, uint8_t pin_number);

// enter standby mode
static inline void standby_enter();



//######## internal function declarations



//######## internal variables



//######## preprocessor macros



//######## preprocessor #define requirements

#ifndef STANDBY_RESTORE_HSI_48MHz
#pragma message "HSI will be 24MHz after waking from standby, is this intentional? Use #define STANDBY_RESTORE_HSI_48MHz to automatically revert to 48MHz HSI."
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



static inline void standby_gpio_assign_pin(uint8_t standby_gpio_port, uint8_t pin_number, uint8_t standby_trigger_direction) {
	// enable the correct GPIO port register
	RCC->APB2PCENR |= (1 << (2 + standby_gpio_port));

	// assign pin 2 interrupt from portD (0b11) to EXTI channel 2
	AFIO->EXTICR |= (uint32_t)(standby_gpio_port << (pin_number * 2));

	// enable line2 interrupt event
	EXTI->EVENR |= (1 << pin_number);

	// set trigger condition
	EXTI->RTENR |= (standby_trigger_direction & 0b01) * (1 << pin_number);
	EXTI->FTENR |= (standby_trigger_direction >> 1) * (1 << pin_number);
}



static inline void standby_gpio_assign_button(GPIO_TypeDef* gpio_port, uint8_t pin_number) {
	// configure button as input, pullup
	gpio_port->CFGLR &= ~(0xf << (pin_number * 4));
	gpio_port->CFGLR |= (GPIO_CNF_IN_PUPD) << (pin_number * 4);
	gpio_port->BSHR = (1 << pin_number);
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
