//######## necessities

// include guards
#ifndef CH32V003_CAPTOUCH_H
#define CH32V003_CAPTOUCH_H

// includes
#include<stdint.h>								//uintN_t support

// maintenance includes
#ifndef CAPTOUCH_SENSE_THRESHOLD
#define SYSTEM_CORE_CLOCK 480000000
#include"../../ch32v003fun/ch32v003fun.h"
#endif





/*######## library description

This library provides basic capacitive touch sensing functionality on CH32V003.
You can use it for buttons, sliders or both.

Expect around 10uA total power consumption in standby.
The LSI oscillator used for auto-wakeup uses 0.6uA.
*/

/*######## working principle

A floating input pin has a high impedance, which means it can hold a tiny ammount of charge as it forms a capacitor against the environment.  

A sense pin can pull itself up (regular pushpull output mode) to VDD and then go into input mode (high impedance).  
Now, a large resistor (>=1 megaohm) connecting it to ground will pull it down at a controlled rate.  

Bringing a finger close to the sense pad increases its capacitance (ability to hold more charge), thus it takes longer to charge, thus we see a change in time, which we can use as we see fit.  
Since we're trying to measure a change in capacitance, any extra cable length between the sense pins and the capacitive surfaces adds parasitic capacitance.  

It's a bit like a motor glider pulling himself up into the sky using the engine, then turning it off, starting the stopwatch and stopping it on touchdown.  
In that analogy, the extra capacitance from the finger is like steadily rising air slowing the planes descent.  

*/

/*######## usage and configuration

Per pin number (called line in this library), you may use a pin from one port as a sense pin, meaning of A1, C1 and D1 you may choose one to become sense pin, likewise for A2, C2 and D2 etc.

For good responsiveness, especially with sliders, a poll-rate of 1-2ms is advised, you will have to schedule this yourself.

At the beginning of your programm, you may want to record no-touch offsets as calibration values and then use them for the captouch_value_clean function.
Together with a threshold suited to your application (try 5), this should result in readings of 0 when there is no finger near the sense pad.

*/

/*######## requirements: define before including this library!

For each sense pin you wish to use, you need to define its port (GPIOA, GPIOC or GPIOD) like this:
#define CAPTOUCH_SENSE_PORT_L2 GPIOD
#define CAPTOUCH_SENSE_PORT_L5 GPIOC
#define CAPTOUCH_SENSE_PORT_L6 GPIOC
This translates to D2, C5 and C6 being configured as sense pins.

to control when a touch will start registering
#define CAPTOUCH_SENSE_THRESHOLD 5

*/

/*######## features: define before including this library!
You can pick whether you want to use buttons, sliders or both.

for buttons:
#define CAPTOUCH_BTN_CONTACT_THRESHOLD 30

for sliders:
#define CAPTOUCH_SLIDER_CONTACT_THRESHOLD 30

if you wish to increase resolution and sensitivity, you may want to run SysTick at full core clock speed:
https://github.com/cnlohr/ch32v003fun/wiki/Time
Though note, that this will likely not improve the stability of the measured values.
*/


/*######## electrical connections

Each sensing surface is directly connected to a sense pin and needs to be pulled down to ground through its own >= 1 megaohm resistor.

parasitic capacitance is introduced by
	long cables / traces
	no clearance to other cables / traces, especially ground
	too large sensing surface (>12mm), also introduces proximity effect


Symptoms of bad contact:
 * no readings with <1 mm proximity but not touching
 * jumpy output while gently pressing the finger on it in one spot
 * high sensitivity while moving the finger slowly but low sensitivity while moving the finger quickly (though this is also affected by a slow poll rate)
*/

/*######## implementation details

The library relies on SysTick and provides an EXTI0_7 ISR.
For measurement stability, temproraily all other programmable interrupts will be halted and interrupt priority will be temporarily elevated.
A `while(pin_is_high)`-loop produces far greater output value jitter, even in combination with `__disable_irq()`.
__WFI into sleep (core clock halted) takes far too long to wake up.
Total measurement time is only 100 ticks without finger and 240 with finger ( 16.67 .. 40 µs).
A delay function that has the ability to exit early when the interrupt raises a flag was tested but too slow.

*/


//######## references: use these





//######## function overview (declarations): use these!

// configure all sense pins defined as CAPTOUCH_SENSE_PORT_Lx
static inline void captouch_assign_sense();

// perform a capacitive measurement on a sense pin
static inline uint16_t captouch_sense(GPIO_TypeDef* port, uint8_t pin);

// remove offset from measurement and apply threshold
static inline uint8_t captouch_value_clean(uint16_t value, uint8_t cal, uint8_t threshold);

// divide value into steps by discarding bits (optional, use if you need)
static inline uint8_t captouch_discretize(uint8_t value, uint8_t discard_bits);

// slider with 2 pins: lower accuracy
// get momentary finger position
static inline uint8_t captouch_slider2(uint16_t value0, uint16_t value1);
// get scrollable value
static inline void captouch_slider2_scroll(uint16_t value0, uint16_t value1, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment);

// slider with 3 pins: higher accuracy
// get momentary finger position
static inline uint8_t captouch_slider3(uint16_t value0, uint16_t value1, uint16_t value2);
// get scrollable value
static inline void captouch_slider3_scroll(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment);



//######## internal function declarations

RV_STATIC_INLINE uint32_t NVIC_get_enabled_IRQs();
RV_STATIC_INLINE void NVIC_clear_all_IRQs_except(uint8_t IRQn_to_keep);
RV_STATIC_INLINE void NVIC_restore_IRQs(uint32_t old_state);



//######## preprocessor #define requirements

#if !defined(CAPTOUCH_SENSE_THRESHOLD)
#error "please #define a CAPTOUCH_SENSE_THRESHOLD"
#error "consult the 'requirements` section for more information."
#endif

#if !defined(CAPTOUCH_BTN_CONTACT_THRESHOLD) && !defined(CAPTOUCH_SLIDER_CONTACT_THRESHOLD)
#warning "you define neither CAPTOUCH_BTN_CONTACT_THRESHOLD nor CAPTOUCH_SLIDER_CONTACT_THRESHOLD. is this intentional?"
#warning "consult the 'features` section for more information."
#endif



//######## preprocessor macros

#define CAPTOUCH_CEILING 0xfe

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



// maintenance defines
#ifndef CAPTOUCH_SENSE_THRESHOLD
#define CAPTOUCH_SENSE_PORT_L0 GPIOD
#define CAPTOUCH_SENSE_PORT_L1 GPIOD
#define CAPTOUCH_SENSE_PORT_L2 GPIOD
#define CAPTOUCH_SENSE_PORT_L3 GPIOD
#define CAPTOUCH_SENSE_PORT_L4 GPIOD
#define CAPTOUCH_SENSE_PORT_L5 GPIOD
#define CAPTOUCH_SENSE_PORT_L6 GPIOD
#define CAPTOUCH_SENSE_PORT_L7 GPIOD
#define CAPTOUCH_SENSE_THRESHOLD 5
#define CAPTOUCH_BTN_CONTACT_THRESHOLD 30
#define CAPTOUCH_SLIDER_CONTACT_THRESHOLD 30
#endif



//######## internal variables

volatile uint32_t captouch_t_discharged;



//######## small function definitions, static inline

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


// special delay function that can end either when target SysTick is reached or event becomes 1 (by interrupt)
void EXTI7_0_IRQHandler( void ) __attribute__((interrupt));
void EXTI7_0_IRQHandler( void ) 
{
	captouch_t_discharged = SysTick->CNT;
	// obligatory interrupt acknowledge
	EXTI->INTFR = ~0;
}

static inline uint16_t captouch_sense(GPIO_TypeDef* port, uint8_t pin) {
	volatile uint32_t t_pull;
	// disable all IRQs besides EXTI7_0
	uint32_t IRQ_backup = NVIC_get_enabled_IRQs();
	NVIC_clear_all_IRQs_except(EXTI7_0_IRQn);
	// elevate EXT7_0 interrupt priority
	//NVIC_SetPriority(EXTI7_0_IRQn, ((1<<7) | (0<<6) | (1<<4)));
	// register pin for pin change interrupt
	EXTI->INTENR |= 1 << pin;
	// let external pulldown resistor slowly discharge pin by turning it into a Hi-Z input
	port->CFGLR &= ~(0xf << (pin * 4));
	port->CFGLR |= ((GPIO_CNF_IN_FLOATING) << (pin * 4));
	// store when the pin discharge started
	t_pull = SysTick->CNT;
	// wait 255 Ticks (42.5ns @ core clock / 8), 
	DelaySysTick(255);
	// by now, EXTI7_0 will surely have fired and recorded the time it took to discharge the pin
	// restore old IRQ config from backup
	NVIC_restore_IRQs(IRQ_backup);
	// register pin for pin change interrupt
	EXTI->INTENR &= ~(1 << pin);
	// restore EXT7_0 interrupt priority to normal level
	//NVIC_SetPriority(EXTI7_0_IRQn, 0);
	// recharge pin for next time
	captouch_cfg_pin_charge(port, pin);
	// calculate time to discharge
	;
	return t_pull = captouch_t_discharged - t_pull;
}



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



static inline uint8_t captouch_slider2(uint16_t value0, uint16_t value1) {
	uint16_t ratio;
	uint16_t value_sum = value0 + value1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > CAPTOUCH_SLIDER_CONTACT_THRESHOLD) {
		// ratio in integer space requires * 256
		ratio = ((value1 << 8) / value_sum);
		return (ratio > 255) ? 255 : ratio;
	}
	return 0;
}

static inline void captouch_slider2_scroll(uint16_t value0, uint16_t value1, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
	uint16_t value_sum = value0 + value1;
	uint16_t ratio;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > CAPTOUCH_SLIDER_CONTACT_THRESHOLD) {
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



static inline uint8_t captouch_slider3(uint16_t value0, uint16_t value1, uint16_t value2) {
	uint16_t value_sum = value0 + value1 + value2;
	uint16_t ratio;
	uint16_t sum0;
	uint16_t sum1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > CAPTOUCH_SLIDER_CONTACT_THRESHOLD) {
		sum0 = value0 + value1;
		sum1 = value1 + value2;
		// ratio in integer space requires * 256
		ratio = ((sum1 << 8) / (sum0 + sum1));
		return (ratio > 0xff) ? 0xff : (uint8_t)ratio;
	}
	return 0;
}

static inline void captouch_slider3_scroll(uint8_t value0, uint8_t value1, uint8_t value2, uint8_t* output, int16_t* memory, uint8_t threshold_change, uint8_t increment) {
	uint16_t value_sum = value0 + value1 + value2;
	uint16_t ratio;
	uint16_t sum0;
	uint16_t sum1;
	// possible source of division instability when value_sum is small (= doesn't have much contact to finger)
	if (value_sum > CAPTOUCH_SLIDER_CONTACT_THRESHOLD) {
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


/*
// these filters don't work so good
static inline void captouch_filter(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + input;
}
static inline void captouch_filter_2ndorder(uint8_t* output, uint8_t input) {
	*output = (*output - (*output >> 6)) + (*output - (*output >> 6)) + input;
}
*/

//######## small internal function definitions, static inline

/*
The following 3 functions serve to suspend all interrupts, except for the one you momentarily need.
The purpose of this is to not disturb the one interrupt of interest and let it run unimpeded.
procedure:
1. save the enabled IRQs: uint32_t IRQ_backup = NVIC_get_enabled_IRQs();
2. disable all IRQs: NVIC_clear_all_IRQs_except(IRQ_of_interest);
4. restore the previously enabled IRQs: NVIC_restore_IRQs(IRQ_backup);
bit layout of the IRQ backup
bit		0 | 1 | 2  |  3  | 4  |  5  | 6  .. 22 | 23 .. 28
IRQn		2 | 3 | 12 | res | 14 | res | 16 .. 31 | 32 .. 38
IRQn 2 and 3 aren't actually user-settable (see RM).
Specifying an invalid IRQn_to_keep like 0 will disable all interrupts.
*/

RV_STATIC_INLINE uint32_t NVIC_get_enabled_IRQs()
{
	return ( ((NVIC->ISR[0] >> 2) & 0b11) | ((NVIC->ISR[0] >> 12) << 2) | ((NVIC->ISR[1] & 0b1111111) << 23) );
}

RV_STATIC_INLINE void NVIC_clear_all_IRQs_except(uint8_t IRQn_to_keep)
{
	if (!(IRQn_to_keep >> 5)) {		// IRQn_to_keep < 32
		NVIC->IRER[0] = (~0) & (~(1 << IRQn_to_keep));
		NVIC->IRER[1] = (~0);
	}
	else {
		IRQn_to_keep = IRQn_to_keep >> 5;
		NVIC->IRER[0] = (~0);
		NVIC->IRER[1] = (~0) & (~(1 << IRQn_to_keep));
	}
}

RV_STATIC_INLINE void NVIC_restore_IRQs(uint32_t old_state)
{
	NVIC->IENR[0] = (old_state >> 2) << 12;
	NVIC->IENR[1] = old_state >> 23;
}

//######## implementation block
//#define CH32V003_LIBRARYNAME_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_LIBRARYNAME_IMPLEMENTATION)



#endif // CH32V003_LIBRARYNAME_IMPLEMENTATION
#endif // CH32V003_LIBRARYNAME_H
