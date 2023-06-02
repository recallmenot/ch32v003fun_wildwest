//######## necessities

// include guards
#ifndef CH32V003_CAPTOUCH_H
#define CH32V003_CAPTOUCH_H

// includes
#include<stdint.h>								//uintN_t support

// maintenance includes
#ifndef CAPTOUCH_PUSHPULL_PORT
#define SYSTEM_CORE_CLOCK 480000000
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

void captouch_delay_systick_interruptible(uint32_t n, uint8_t* event) {
	uint32_t targend = SysTick->CNT + n;
	while( ((int32_t)( SysTick->CNT - targend )) < 0 ) {
		if (*event) {
			break;
		}
	}
	*event = 0;
}

/*
void DelaySysTick( uint32_t n )
{
	uint32_t targend = SysTick->CNT + n;
	while( ((int32_t)( SysTick->CNT - targend )) < 0 );
}
*/

#define captouch_Delay_Us(n, event) captouch_delay_systick_interruptible( ((n) * DELAY_US_TIME), event)
#define captouch_Delay_Ms(n, event) captouch_delay_systick_interruptible( ((n) * DELAY_MS_TIME), event)

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

volatile uint32_t captouch_t_discharged;
volatile uint8_t captouch_pcint_fired = 0;



//######## small function definitions, static inline



static inline void captouch_assign_pushpull() {
	//pin mode output push pull
	CAPTOUCH_PUSHPULL_PORT->CFGLR &= ~(0xf << (CAPTOUCH_PUSHPULL_PIN * 4));
	CAPTOUCH_PUSHPULL_PORT->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (CAPTOUCH_PUSHPULL_PIN * 4);
	CAPTOUCH_PUSHPULL_PORT->BSHR = (1 << (CAPTOUCH_PUSHPULL_PIN + 16));
}


// ternary expressions can evaluate pointers unlike _Generic and *should* also be evaluated at compile-time
#define port_to_portnumber(port)	(((port) == GPIOA) ? 0b00 : \
					(((port) == GPIOC) ? 0b10 : 0b11))

#define portnumber_to_port(port) _Generic((port), \
	0b00: GPIOA, \
	0b10: GPIOB, \
	0b11: GPIOC \
)


#define captouch_cfg_pc_interrupt(port, pin) ({ \
	AFIO->EXTICR |= port_to_portnumber(port) << (pin * 2); \
	EXTI->FTENR |= 1 << pin; \
})

// sense pin is charged to VDD by default
#define captouch_cfg_pin_charge(port, pin) ({ \
	port->CFGLR &= ~(0xf << (pin * 4)); \
	port->CFGLR |= (GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (pin * 4); \
	port->BSHR = (1 << (pin)); \
})




static inline void captouch_assign_sense() {
	// enable AFIO to be able to route pins to EXTI of PFIC
	RCC->APB2PCENR |= RCC_AFIOEN;
	#ifdef CAPTOUCH_SENSE_PORT_L0
	// enable the correct GPIO port register
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L0, 0);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L0, 0);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L1
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L1, 1);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L1, 1);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L2
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L2, 2);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L2, 2);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L3
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L3, 3);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L3, 3);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L4
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L4, 4);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L4, 4);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L5
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L5, 5);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L5, 5);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L6
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L6, 6);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L6, 6);
	#endif
	#ifdef CAPTOUCH_SENSE_PORT_L7
	captouch_cfg_pin_charge(CAPTOUCH_SENSE_PORT_L7, 7);
	captouch_cfg_pc_interrupt(CAPTOUCH_SENSE_PORT_L7, 7);
	#endif
}

// bit		0 | 1 | 2  |  3  | 4  |  5  | 6  .. 22 | 23 .. 28
// IRQn		2 | 3 | 12 | res | 14 | res | 16 .. 31 | 32 .. 38
RV_STATIC_INLINE uint32_t NVIC_get_enabled_IRQs()
{
	return ( ((NVIC->ISR[0] >> 2) & 0b11) | ((NVIC->ISR[0] >> 12) << 2) | ((NVIC->ISR[1] & 0b1111111) << 23) );
}

RV_STATIC_INLINE void NVIC_clear_all_IRQs()
{
	NVIC->IRER[0] = ~0;
	NVIC->IRER[1] = ~0;
}

RV_STATIC_INLINE void NVIC_restore_IRQs(uint32_t old_state)
{
	NVIC->IENR[0] = (old_state >> 2) << 12;
	NVIC->IENR[1] = old_state >> 23;
}

// special delay function that can end either when target SysTick is reached or event becomes 1 (by interrupt)
void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
	captouch_t_discharged = SysTick->CNT;
	// notify delay_systick_interruptible it can stop
	captouch_pcint_fired = 1;
	// obligatory interrupt acknowledge
	EXTI->INTFR = ~0;
}

uint16_t captouch_sense(GPIO_TypeDef* port, uint8_t pin) {
	volatile uint32_t t_pull;
	// disable all IRQs besides EXTI7_0
	uint32_t IRQ_backup = NVIC_get_enabled_IRQs();
	NVIC_clear_all_IRQs();
	NVIC_EnableIRQ(EXTI7_0_IRQn);
	// elevate EXT7_0 interrupt priority
	NVIC_SetPriority(EXTI7_0_IRQn, ((1<<7) | (0<<6) | (1<<4)));
	// register pin for pin change interrupt
	EXTI->INTENR |= 1 << pin;
	// let external pulldown resistor slowly discharge pin by turning it into a Hi-Z input
	port->CFGLR &= ~(0xf << (pin * 4));
	port->CFGLR |= ((GPIO_CNF_IN_FLOATING) << (pin * 4));
	// store when the pin discharge started
	t_pull = SysTick->CNT;
	// wait 255 Ticks (42.5ns @ core clock / 8), 
	DelaySysTick(255);
	// EXTI7_0 will surely have fired and recorded the time to discharge the pin by now
	// restore old IRQ config from backup
	NVIC_restore_IRQs(IRQ_backup);
	// register pin for pin change interrupt
	EXTI->INTENR &= ~(1 << pin);
	// restore EXT7_0 interrupt priority to normal level
	NVIC_SetPriority(EXTI7_0_IRQn, 0);
	// recharge pin for next time
	captouch_cfg_pin_charge(port, pin);
	// calculate time to discharge
	;
	return t_pull = captouch_t_discharged - t_pull;
}


/*
static inline uint8_t captouch_sense(GPIO_TypeDef* port, uint8_t pin) {
	volatile uint32_t t_pull;
	__disable_irq();
	port->CFGLR &= ~(0xf << (pin * 4));
	port->CFGLR |= ((GPIO_CNF_IN_FLOATING) << (pin * 4));
	t_pull = SysTick->CNT;
	while ((port->INDR & (1 << pin))) {
		//__asm__("nop");
	}
	t_pull = SysTick->CNT - t_pull;
	__enable_irq();
	port->CFGLR &= ~(0xf << (pin * 4));
	port->CFGLR |= ((GPIO_Speed_50MHz | GPIO_CNF_OUT_PP) << (pin * 4));
	port->BSHR = (1 << (pin));
	return (t_pull < CAPTOUCH_CEILING ? t_pull : CAPTOUCH_CEILING);
}
*/

static inline uint8_t captouch_value_clean(uint16_t value, uint8_t cal, uint8_t threshold) {
	if (value > cal) {
		uint8_t result = value - cal;
		return (result > threshold ?
			(result < CAPTOUCH_CEILING ? result : CAPTOUCH_CEILING)
	  		: 0);
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
