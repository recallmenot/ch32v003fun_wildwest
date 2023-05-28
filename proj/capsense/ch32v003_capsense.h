//######## necessities

// include guards
#ifndef CH32V003_CAPSENSE_H
#define CH32V003_CAPSENSE_H

// includes
#include<stdint.h>								//uintN_t support
#include "string.h"

#ifndef CAPSENSE_SENSE_N_LINES
#include"../../ch32v003fun/ch32v003fun.h"
#endif



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

to have capsense_enter() automatically restore the HSI speed to 48MHz after standby (else it will be 24MHz)
#define CAPSENSE_RESTORE_HSI_48MHz
*/



//######## references: use these

// port definitions for the capsense_gpio_assign functions
#define CAPSENSE_GPIO_PORT_A 0b00
#define CAPSENSE_GPIO_PORT_C 0b10
#define CAPSENSE_GPIO_PORT_D 0b11

// port definitions for the capsense_gpio_assign_pin functions
#define CAPSENSE_TRIGGER_DIRECTION_RISING	0b01
#define CAPSENSE_TRIGGER_DIRECTION_FALLING	0b10
#define CAPSENSE_TRIGGER_DIRECTION_BOTH		0b11



//######## function overview (declarations): use these!



//######## internal function declarations



//######## preprocessor #define requirements

#if CAPSENSE_SENSE_N_LINES < 1
#error "please #define CAPSENSE_SENSE_N_LINES to at least one"
#endif

#ifndef CAPSENSE_PUSHPULL_PORT
#error "please #define CAPSENSE_PUSHPULL_PORT to the port (use the CAPSENSE_GPIO_PORT_ macros)"
#endif

#ifndef CAPSENSE_PUSHPULL_PIN
#error "please #define CAPSENSE_PUSHPULL_PIN to the pin number of the desired pushpull pin"
#endif


//######## preprocessor macros

#define CAPSENSE_GPIO_REGISTER_BASE_I	1024
#define CAPSENSE_GPIO_CFGLR_OFFSET	0x00
#define CAPSENSE_GPIO_OUTDR_OFFSET	0x0C
#define CAPSENSE_GPIO_BSHR_OFFSET	0x10
#define CAPSENSE_GPIO_INDR_OFFSET	0x08
#define CAPSENSE_TRIGGER_RISING_OFFSET	0x08
#define CAPSENSE_TRIGGER_FALLING_OFFSET	0x0C

#ifndef CAPSENSE_SENSE_N_LINES
#define CAPSENSE_PUSHPULL_PORT GPIOC
#define CAPSENSE_PUSHPULL_PIN 7
#define CAPSENSE_SENSE_PORT_L0 GPIOD
#define CAPSENSE_SENSE_PORT_L1 GPIOD
#define CAPSENSE_SENSE_PORT_L2 GPIOD
#define CAPSENSE_SENSE_PORT_L3 GPIOD
#define CAPSENSE_SENSE_PORT_L4 GPIOD
#define CAPSENSE_SENSE_PORT_L5 GPIOD
#define CAPSENSE_SENSE_PORT_L6 GPIOD
#define CAPSENSE_SENSE_PORT_L7 GPIOD
#endif

#define CAPSENSE_SENSE_PORT(x) (CAPSENSE_SENSE_PORT_L##x) // concatenate number

#define CAPSENSE_CEILING 0xfe


//######## internal variables

uint32_t capsense_t_now;
/*
uint32_t capsense_t_reached[CAPSENSE_SENSE_N_LINES] = {0};
uint32_t capsense_t_result[CAPSENSE_SENSE_N_LINES] = {0};
*/
uint32_t capsense_t_pushpull = 0;
uint16_t* capsense_t_result[8] = {0};

uint8_t capsense_sense_reached_count = 0;

uint8_t capsense_sense_line_active = 0;


//######## small function definitions, static inline


/*

void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) {
	// find line that raised interrupt (=pin)
	capsense_sense_line_active = __builtin_ctz(EXTI->INTFR);
	capsense_t_result[capsense_sense_line_active] = SysTick->CNT - capsense_t_pushpull;
	capsense_sense_reached_count++;
	// clear interrupt flag from line
	EXTI->INTFR |= (1 << capsense_sense_line_active);
	// when all 4 have reached trigger threshold
	if (capsense_sense_reached_count >= CAPSENSE_SENSE_N_LINES) {
		capsense_sense_reached_count = 0;
		capsense_t_pushpull = SysTick->CNT;
		// invert pushpull pin state
		*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * CAPSENSE_PUSHPULL_PORT) + CAPSENSE_GPIO_OUTDR_OFFSET) ^= (1 << CAPSENSE_PUSHPULL_PIN);
	}
	GPIOD->OUTDR ^= (1 << 4);
}
static inline void capsense_assign_pushpull() {
	//pin mode output push pull
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * CAPSENSE_PUSHPULL_PORT) + CAPSENSE_GPIO_CFGLR_OFFSET) &= ~(0xf << (CAPSENSE_PUSHPULL_PIN * 4));
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * CAPSENSE_PUSHPULL_PORT) + CAPSENSE_GPIO_CFGLR_OFFSET) |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (CAPSENSE_PUSHPULL_PIN * 4);
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * CAPSENSE_PUSHPULL_PORT) + CAPSENSE_GPIO_BSHR_OFFSET) = (1 << CAPSENSE_PUSHPULL_PIN);
}
static inline void capsense_assign_sense(uint8_t capsense_gpio_port, uint8_t pin_number) {

	// enable the correct GPIO port register
	RCC->APB2PCENR |= (1 << (2 + capsense_gpio_port));

	// sense as input, floating
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * capsense_gpio_port) + CAPSENSE_GPIO_CFGLR_OFFSET) &= ~(0xf << (pin_number * 4));
	*(uint32_t*)(uintptr_t)(GPIOA_BASE + (CAPSENSE_GPIO_REGISTER_BASE_I * capsense_gpio_port) + CAPSENSE_GPIO_CFGLR_OFFSET) |= (GPIO_CNF_IN_FLOATING) << (pin_number * 4);

	// enable alternate IO function module clock
	RCC->APB2PCENR |= RCC_AFIOEN;

	// assign pin 2 interrupt from portD (0b11) to EXTI channel 2
	AFIO->EXTICR |= (uint32_t)(capsense_gpio_port << (pin_number * 2));

	// enable line interrupt
	EXTI->INTENR |= (1 << pin_number);

	NVIC_EnableIRQ( EXTI7_0_IRQn );

	// set to trigger at rising and falling edge
	*(uint32_t*)(uintptr_t)(EXTI_BASE + CAPSENSE_TRIGGER_RISING_OFFSET) |= (1 << pin_number);
	*(uint32_t*)(uintptr_t)(EXTI_BASE + CAPSENSE_TRIGGER_FALLING_OFFSET) |= (1 << pin_number);
}

*/




static inline void capsense_assign_pushpull() {
	//pin mode output push pull
	CAPSENSE_PUSHPULL_PORT->CFGLR &= ~(0xf << (CAPSENSE_PUSHPULL_PIN * 4));
	CAPSENSE_PUSHPULL_PORT->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (CAPSENSE_PUSHPULL_PIN * 4);
	CAPSENSE_PUSHPULL_PORT->BSHR = (1 << (CAPSENSE_PUSHPULL_PIN + 16));
}

static inline void capsense_assign_sense() {
	// enable the correct GPIO port register
	// sense as input, floating
	#ifdef CAPSENSE_SENSE_PORT_L0
	CAPSENSE_SENSE_PORT_L0->CFGLR &= ~(0xf << (0 * 4));
	CAPSENSE_SENSE_PORT_L0->CFGLR |= (GPIO_CNF_IN_FLOATING) << (0 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L1
	CAPSENSE_SENSE_PORT_L1->CFGLR &= ~(0xf << (1 * 4));
	CAPSENSE_SENSE_PORT_L1->CFGLR |= (GPIO_CNF_IN_FLOATING) << (1 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L2
	CAPSENSE_SENSE_PORT_L2->CFGLR &= ~(0xf << (2 * 4));
	CAPSENSE_SENSE_PORT_L2->CFGLR |= (GPIO_CNF_IN_FLOATING) << (2 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L3
	CAPSENSE_SENSE_PORT_L3->CFGLR &= ~(0xf << (3 * 4));
	CAPSENSE_SENSE_PORT_L3->CFGLR |= (GPIO_CNF_IN_FLOATING) << (3 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L4
	CAPSENSE_SENSE_PORT_L4->CFGLR &= ~(0xf << (4 * 4));
	CAPSENSE_SENSE_PORT_L4->CFGLR |= (GPIO_CNF_IN_FLOATING) << (4 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L5
	CAPSENSE_SENSE_PORT_L5->CFGLR &= ~(0xf << (5 * 4));
	CAPSENSE_SENSE_PORT_L5->CFGLR |= (GPIO_CNF_IN_FLOATING) << (5 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L6
	CAPSENSE_SENSE_PORT_L6->CFGLR &= ~(0xf << (6 * 4));
	CAPSENSE_SENSE_PORT_L6->CFGLR |= (GPIO_CNF_IN_FLOATING) << (6 * 4);
	#endif
	#ifdef CAPSENSE_SENSE_PORT_L7
	CAPSENSE_SENSE_PORT_L7->CFGLR &= ~(0xf << (7 * 4));
	CAPSENSE_SENSE_PORT_L7->CFGLR |= (GPIO_CNF_IN_FLOATING) << (7 * 4);
	#endif
}

static inline GPIO_TypeDef* capsense_sense_port(int index) __attribute__((always_inline));
static inline GPIO_TypeDef* capsense_sense_port(int index) {
	#if defined(CAPSENSE_SENSE_PORT_L0)
	if (index == 0) {
		return CAPSENSE_SENSE_PORT_L0;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L1)
	if (index == 1) {
		return CAPSENSE_SENSE_PORT_L1;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L2)
	if (index == 2) {
		return CAPSENSE_SENSE_PORT_L2;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L3)
	if (index == 3) {
		return CAPSENSE_SENSE_PORT_L3;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L4)
	if (index == 4) {
		return CAPSENSE_SENSE_PORT_L4;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L5)
	if (index == 5) {
	return CAPSENSE_SENSE_PORT_L5;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L6)
	if (index == 6) {
		return CAPSENSE_SENSE_PORT_L6;
	}
	#endif
	#if defined(CAPSENSE_SENSE_PORT_L7)
	if (index == 7) {
		return CAPSENSE_SENSE_PORT_L7;
	}
	#endif
	return 0;
}

static inline uint8_t capsense_sense(GPIO_TypeDef* port, uint8_t pin) {
	CAPSENSE_PUSHPULL_PORT->BSHR = 1 << (CAPSENSE_PUSHPULL_PIN + 16);
	while (port->INDR & (1 << pin)) {
		//__asm__("nop");
	}
	CAPSENSE_PUSHPULL_PORT->BSHR = 1 << CAPSENSE_PUSHPULL_PIN;
	volatile uint32_t t_pushpull = SysTick->CNT;
	while (!(port->INDR & (1 << pin))) {
		//__asm__("nop");
	}
	t_pushpull = SysTick->CNT - t_pushpull;
	CAPSENSE_PUSHPULL_PORT->BSHR = 1 << (CAPSENSE_PUSHPULL_PIN + 16);
	return (t_pushpull < CAPSENSE_CEILING ? t_pushpull : CAPSENSE_CEILING);
}



static inline uint8_t capsense_offset(uint8_t value, uint8_t cal, uint8_t threshold) {
	if (value > cal) {
		uint8_t result = value - cal;
		return (result > threshold ? result : 0);
	}
	else {
		return 0;
	}
}


static inline uint8_t capsense_discretize(uint8_t value, uint8_t discard_bits) {
	return ((value >> discard_bits) << discard_bits);
}


static inline uint8_t capsense_slider2(uint16_t value0, uint16_t value1, uint8_t threshold_contact) {
	uint16_t ratio;
	uint16_t value_sum = value0 + value1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > threshold_contact) {
		// ratio in integer space requires * 256
		ratio = ((value1 << 8) / value_sum);
		return (ratio > 255) ? 255 : ratio;
	}
	return 0;
}

static inline void capsense_slider2_scroll(uint16_t value0, uint16_t value1, uint8_t threshold_contact, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
	uint16_t value_sum = value0 + value1;
	uint16_t ratio;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > threshold_contact) {
		// ratio in integer space requires * 256, discretized for stability ( << 5 = * 64 so resolution will be 8)
		ratio = (((value1 << 5) / value_sum) << 3);
		if (ratio - *memory > threshold_change) {
			if (*output <= 255 - increment) {
				*output = *output + increment;
			}
			*memory = ratio;
		}
		else if (*memory - ratio > threshold_change) {
			if (*output >= increment) {
				*output = *output - increment;
			}
			*memory = ratio;
		}
	}
}

static inline uint8_t capsense_slider3(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t threshold_contact) {
	uint16_t value_sum = value0 + value1 + value2;
	uint16_t ratio;
	uint16_t sum0;
	uint16_t sum1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > threshold_contact) {
		sum0 = value0 + value1;
		sum1 = value1 + value2;
		// ratio in integer space requires * 256
		ratio = ((sum1 << 8) / (sum0 + sum1));
		return (ratio > 0xff) ? 0xff : (uint8_t)ratio;
	}
	return 0;
}

static inline void capsense_slider3_scroll(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t threshold_contact, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
	uint16_t value_sum = value0 + value1 + value2;
	uint16_t ratio;
	uint16_t sum0;
	uint16_t sum1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > threshold_contact) {
		sum0 = value0 + value1;
		sum1 = value1 + value2;
		// ratio in integer space requires * 256
		ratio = ((sum1 << 8) / (sum0 + sum1));
		if (ratio - *memory > threshold_change) {
			if (*output <= 255 - increment) {
				*output = *output + increment;
			}
			*memory = ratio;
		}
		else if (*memory - ratio > threshold_change) {
			if (*output >= increment) {
				*output = *output - increment;
			}
			*memory = ratio;
		}
	}
}



static inline void capsense_filter(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + input;
}
static inline void capsense_filter_2ndorder(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + (*output - (*output >> 6)) + input;
}


//######## small internal function definitions, static inline



//######## implementation block
//#define CH32V003_LIBRARYNAME_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_LIBRARYNAME_IMPLEMENTATION)



#endif // CH32V003_LIBRARYNAME_IMPLEMENTATION
#endif // CH32V003_LIBRARYNAME_H
