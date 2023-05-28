//######## necessities

// include guards
#ifndef CH32V003_CAPTOUCH_H
#define CH32V003_CAPTOUCH_H

// includes
#include<stdint.h>								//uintN_t support

// maintenance includes
#ifndef CAPTOUCH_PUSHPULL_PORT
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

to have captouch_enter() automatically restore the HSI speed to 48MHz after standby (else it will be 24MHz)
#define CAPTOUCH_RESTORE_HSI_48MHz
*/



//######## references: use these

// port definitions for the captouch_gpio_assign functions
#define CAPTOUCH_GPIO_PORT_A 0b00
#define CAPTOUCH_GPIO_PORT_C 0b10
#define CAPTOUCH_GPIO_PORT_D 0b11

// port definitions for the captouch_gpio_assign_pin functions
#define CAPTOUCH_TRIGGER_DIRECTION_RISING	0b01
#define CAPTOUCH_TRIGGER_DIRECTION_FALLING	0b10
#define CAPTOUCH_TRIGGER_DIRECTION_BOTH		0b11



//######## function overview (declarations): use these!



//######## internal function declarations



//######## preprocessor #define requirements

#ifndef CAPTOUCH_PUSHPULL_PORT
#error "please #define CAPTOUCH_PUSHPULL_PORT to the port (use the CAPTOUCH_GPIO_PORT_ macros)"
#endif

#ifndef CAPTOUCH_PUSHPULL_PIN
#error "please #define CAPTOUCH_PUSHPULL_PIN to the pin number of the desired pushpull pin"
#endif


//######## preprocessor macros

// maintenance defines
#ifndef CAPTOUCH_PUSHPULL_PORT
#define CAPTOUCH_PUSHPULL_PORT GPIOC
#define CAPTOUCH_PUSHPULL_PIN 7
#define CAPTOUCH_SENSE_PORT_L0 GPIOD
#define CAPTOUCH_SENSE_PORT_L1 GPIOD
#define CAPTOUCH_SENSE_PORT_L2 GPIOD
#define CAPTOUCH_SENSE_PORT_L3 GPIOD
#define CAPTOUCH_SENSE_PORT_L4 GPIOD
#define CAPTOUCH_SENSE_PORT_L5 GPIOD
#define CAPTOUCH_SENSE_PORT_L6 GPIOD
#define CAPTOUCH_SENSE_PORT_L7 GPIOD
#endif

#define CAPTOUCH_CEILING 0xfe


//######## internal variables




//######## small function definitions, static inline



static inline void captouch_assign_pushpull() {
	//pin mode output push pull
	CAPTOUCH_PUSHPULL_PORT->CFGLR &= ~(0xf << (CAPTOUCH_PUSHPULL_PIN * 4));
	CAPTOUCH_PUSHPULL_PORT->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (CAPTOUCH_PUSHPULL_PIN * 4);
	CAPTOUCH_PUSHPULL_PORT->BSHR = (1 << (CAPTOUCH_PUSHPULL_PIN + 16));
}

static inline void captouch_assign_sense() {
	// enable the correct GPIO port register
	// sense as input, floating
	#ifdef CAPTOUCH_SENSE_PORT_L0
	CAPTOUCH_SENSE_PORT_L0->CFGLR &= ~(0xf << (0 * 4));
	CAPTOUCH_SENSE_PORT_L0->CFGLR |= (GPIO_CNF_IN_FLOATING) << (0 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L1
	CAPTOUCH_SENSE_PORT_L1->CFGLR &= ~(0xf << (1 * 4));
	CAPTOUCH_SENSE_PORT_L1->CFGLR |= (GPIO_CNF_IN_FLOATING) << (1 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L2
	CAPTOUCH_SENSE_PORT_L2->CFGLR &= ~(0xf << (2 * 4));
	CAPTOUCH_SENSE_PORT_L2->CFGLR |= (GPIO_CNF_IN_FLOATING) << (2 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L3
	CAPTOUCH_SENSE_PORT_L3->CFGLR &= ~(0xf << (3 * 4));
	CAPTOUCH_SENSE_PORT_L3->CFGLR |= (GPIO_CNF_IN_FLOATING) << (3 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L4
	CAPTOUCH_SENSE_PORT_L4->CFGLR &= ~(0xf << (4 * 4));
	CAPTOUCH_SENSE_PORT_L4->CFGLR |= (GPIO_CNF_IN_FLOATING) << (4 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L5
	CAPTOUCH_SENSE_PORT_L5->CFGLR &= ~(0xf << (5 * 4));
	CAPTOUCH_SENSE_PORT_L5->CFGLR |= (GPIO_CNF_IN_FLOATING) << (5 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L6
	CAPTOUCH_SENSE_PORT_L6->CFGLR &= ~(0xf << (6 * 4));
	CAPTOUCH_SENSE_PORT_L6->CFGLR |= (GPIO_CNF_IN_FLOATING) << (6 * 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L7
	CAPTOUCH_SENSE_PORT_L7->CFGLR &= ~(0xf << (7 * 4));
	CAPTOUCH_SENSE_PORT_L7->CFGLR |= (GPIO_CNF_IN_FLOATING) << (7 * 4);
	#endif
}



static inline uint8_t captouch_sense(GPIO_TypeDef* port, uint8_t pin) {
	CAPTOUCH_PUSHPULL_PORT->BSHR = 1 << (CAPTOUCH_PUSHPULL_PIN + 16);
	while (port->INDR & (1 << pin)) {
		//__asm__("nop");
	}
	CAPTOUCH_PUSHPULL_PORT->BSHR = 1 << CAPTOUCH_PUSHPULL_PIN;
	volatile uint32_t t_pushpull = SysTick->CNT;
	while (!(port->INDR & (1 << pin))) {
		//__asm__("nop");
	}
	t_pushpull = SysTick->CNT - t_pushpull;
	CAPTOUCH_PUSHPULL_PORT->BSHR = 1 << (CAPTOUCH_PUSHPULL_PIN + 16);
	return (t_pushpull < CAPTOUCH_CEILING ? t_pushpull : CAPTOUCH_CEILING);
}



static inline uint8_t captouch_offset(uint8_t value, uint8_t cal, uint8_t threshold) {
	if (value > cal) {
		uint8_t result = value - cal;
		return (result > threshold ? result : 0);
	}
	else {
		return 0;
	}
}



static inline uint8_t captouch_discretize(uint8_t value, uint8_t discard_bits) {
	return ((value >> discard_bits) << discard_bits);
}


static inline uint8_t captouch_slider2(uint16_t value0, uint16_t value1, uint8_t threshold_contact) {
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

static inline void captouch_slider2_scroll(uint16_t value0, uint16_t value1, uint8_t threshold_contact, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
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

static inline uint8_t captouch_slider3(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t threshold_contact) {
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

static inline void captouch_slider3_scroll(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t threshold_contact, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
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



static inline void captouch_filter(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + input;
}
static inline void captouch_filter_2ndorder(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + (*output - (*output >> 6)) + input;
}


//######## small internal function definitions, static inline



//######## implementation block
//#define CH32V003_LIBRARYNAME_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_LIBRARYNAME_IMPLEMENTATION)



#endif // CH32V003_LIBRARYNAME_IMPLEMENTATION
#endif // CH32V003_LIBRARYNAME_H
