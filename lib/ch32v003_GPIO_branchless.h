//######## necessities

// include guards
#ifndef CH32V003_GPIO_BR_H
#define CH32V003_GPIO_BR_H

// includes
#include <cstddef>
#include <stdint.h>								//uintN_t support
#include "../ch32v003fun/ch32v003fun.h"



/*######## library description
This is a speedy and light GPIO library due to
	static inlining of most functions
	compile-time abstraction
	branchless where it counts
*/



/*######## library usage and configuration
 * in your .c file, unlock this libraries implementation block:
then
#include "../../extralibs/ch32v003_GPIO_branching.h"

first, enable the desired port.

digital usage is quite Arduino-like:
pinMode
digitalWrite
digitalWrite_lo
digitalWrite_hi
digitalRead

analog usage is almost Arduino-like:
ADCinit
pinMode
analogRead

you can also disable the ADC to save power

there is no analogWrite (yet)


to unlock the ability to easily iterate over the pins
#define CH32V003_GPIO_ITER_IMPLEMENTATION
before the #include
this unlocks the GPIO_from_pinNumber function you can use to derive the desired pins from a simple pinNumber
0 .. 1		A1 .. A2
2 .. 9		C0 .. C7
10..17		D0 .. D7


By default, this library inserts a delay of 300 µs between configureation of the ADC input mux and the time the conversion starts.
This serves to counteract the high input impedance of the ADC, especially if it is increased by external resistors.
The input impedance of port A appears to be especially large.
You can modify it to your liking using the following define before including this library.
#define GPIO_ADC_MUX_DELAY 1200

The ADC on CH32V003 is a capacitive sampler.
GPIO_ADC_sampletime controls the time each conversion is granted, by default it is GPIO_ADC_sampletime_241cy_default, all options come from the GPIO_ADC_sampletimes enum.
To alter it, you have 3 options:
 * `#define GPIO_ADC_sampletime GPIO_ADC_sampletime_43cy` before including this library
 * call the GPIO_ADC_set_sampletime function-like macro to momentarrily set it for one channel
 * call the GPIO_ADC_set_sampletimes_all function-like macro to to momentarrily set it for all channels

*/



//######## pins and states: use these for the functions below!

enum GPIO_port_n {
	GPIO_port_A = 0b00,
	GPIO_port_C = 0b10,
	GPIO_port_D = 0b11,
};

#define GPIOx_to_port_n(GPIOx)				\
	((GPIOx == GPIOA)	?	0b00	:	\
	((GPIOx == GPIOC)	?	0b10	:	\
	((GPIOx == GPIOD)	?	0b11	:	\
					NULL	)))

#define GPIO_port_n_to_GPIOx(GPIO_port_n)		\
	((GPIO_port_n == 0b00)	?	GPIOA	:	\
	((GPIO_port_n == 0b10)	?	GPIOC	:	\
	((GPIO_port_n == 0b11)	?	GPIOD	:	\
					NULL	)))

#define GPIO_port_n_to_RCC_APB2Periph(GPIO_port_n)			\
	((GPIO_port_n == 0b00)	?	RCC_APB2Periph_GPIOA	:	\
	((GPIO_port_n == 0b10)	?	RCC_APB2Periph_GPIOC	:	\
	((GPIO_port_n == 0b11)	?	RCC_APB2Periph_GPIOD	:	\
					NULL			)))



enum GPIO_pinModes {
	GPIO_pinMode_I_floating,
	GPIO_pinMode_I_pullUp,
	GPIO_pinMode_I_pullDown,
	GPIO_pinMode_I_analog,
	GPIO_pinMode_O_pushPull,
	GPIO_pinMode_O_openDrain,
	GPIO_pinMode_O_pushPullMux,
	GPIO_pinMode_O_openDrainMux,
};

#define GPIO_pinMode_to_CFG(GPIO_pinMode, GPIOSpeed)									\
	((GPIO_pinMode == GPIO_pinMode_I_floating)	?	(GPIO_SPEED_IN	| GPIO_CNF_IN_FLOATING)		:	\
	((GPIO_pinMode == GPIO_pinMode_I_pullUp)	?	(GPIO_SPEED_IN	| GPIO_CNF_IN_PUPD)		:	\
	((GPIO_pinMode == GPIO_pinMode_I_pullDown)	?	(GPIO_SPEED_IN	| GPIO_CNF_IN_PUPD)		:	\
	((GPIO_pinMode == GPIO_pinMode_I_analog)	?	(GPIO_SPEED_IN	| GPIO_CNF_IN_ANALOG)		:	\
	((GPIO_pinMode == GPIO_pinMode_O_pushPull)	?	(GPIOSpeed	| GPIO_CNF_OUT_PP)		:	\
	((GPIO_pinMode == GPIO_pinMode_O_openDrain)	?	(GPIOSpeed	| GPIO_CNF_OUT_OD)		:	\
	((GPIO_pinMode == GPIO_pinMode_O_pushPullMux)	?	(GPIOSpeed	| GPIO_CNF_OUT_PP_AF)		:	\
	((GPIO_pinMode == GPIO_pinMode_O_openDrainMux)	?	(GPIOSpeed	| GPIO_CNF_IN_ANALOG)		:	\
  								NULL						))))))))

#define GPIO_pinMode_set_PUPD(GPIO_pinMode, GPIO_port_n, pin) 										\
	((GPIO_pinMode == GPIO_pinMode_I_pullUp)	? 	(GPIO_port_n_to_GPIOx(port)->BSHR = (1 << pin))			:	\
	((GPIO_pinMode == GPIO_pinMode_I_pullDown)	?	(GPIO_port_n_to_GPIOx(port)->BSHR = (1 << (pin + 16)))		:	\
								NULL								))


enum lowhigh {
	low,
	high,
};

// analog inputs
enum GPIO_analog_inputs {
	GPIO_Ain0_A2,
	GPIO_Ain1_A1,
	GPIO_Ain2_C4,
	GPIO_Ain3_D2,
	GPIO_Ain4_D3,
	GPIO_Ain5_D5,
	GPIO_Ain6_D6,
	GPIO_Ain7_D4,
	GPIO_AinVref,
	GPIO_AinVcal,
};

// how many cycles the ADC shall sample the input for (speed vs precision)
enum GPIO_ADC_sampletimes {
	GPIO_ADC_sampletime_3cy,
	GPIO_ADC_sampletime_9cy,
	GPIO_ADC_sampletime_15cy,
	GPIO_ADC_sampletime_30cy,
	GPIO_ADC_sampletime_43cy,
	GPIO_ADC_sampletime_57cy,
	GPIO_ADC_sampletime_73cy,
	GPIO_ADC_sampletime_241cy_default,
};



//######## function overview (declarations): use these!




//######## internal function declarations



//######## internal variables



//######## preprocessor macros

#if !defined(GPIO_ADC_MUX_DELAY)
#define GPIO_ADC_MUX_DELAY 200
#endif

#if !defined(GPIO_ADC_sampletime)
#define GPIO_ADC_sampletime GPIO_ADC_sampletime_241cy_default
#endif




//######## preprocessor #define requirements



//######## small function definitions, static inline

#define GPIO_port_enable(GPIO_port_n) RCC->APB2PCENR |= GPIO_port_n_to_RCC_APB2Periph(GPIO_port_n);

#define GPIO_pin_mode(GPIO_port_n, pin, pinMode, output_speed) ({					\
	GPIO_port_n_to_GPIOx(GPIO_port_n)->CFGLR &= ~(0b1111 << (4 * pin));				\
	GPIO_port_n_to_GPIOx(GPIO_port_n)->CFGLR |= GPIO_pinMode_to_CFG(pinMode, ouput_speed);		\
	GPIO_pinMode_set_PUPD(GPIO_port_n, pin);							\
})

#define GPIO_digitalWrite_hi(GPIO_port_n, pin)		GPIO_port_n_to_GPIOx(GPIO_port_n)->BSHR = (1 << pin)
#define GPIO_digitalWrite_lo(GPIO_port_n, pin)		GPIO_port_n_to_GPIOx(GPIO_port_n)->BSHR = (1 << (pin + 16))

#define GPIO_digitalWrite(GPIO_port_n, pin, lowhigh) ( lowhigh				?	\
						GPIO_digitalWrite_hi(GPIO_port_n, pin)	:	\
						GPIO_digitalWrite_lo(GPIO_port_n, pin)	)	\

#define GPIO_digitalRead(GPIO_port_n, pin) 	((GPIO_port_n_to_GPIOx(GPIO_port_n)->INDR >> pin) & 0b1)


// 0:7 => 3/9/15/30/43/57/73/241 cycles
#define GPIO_ADC_set_sampletime(GPIO_analog_input, GPIO_ADC_sampletime) ({	\
	ADC1->SAMPTR2 &= ~(0b111) << (3 * GPIO_analog_input);			\
	ADC1->SAMPTR2 |= GPIO_ADC_sampletime << (3 * GPIO_analog_input);	\
})

#define GPIO_ADC_set_sampletimes_all(GPIO_ADC_sampletime) ({	\
	ADC1->SAMPTR2 &= 0;					\
	ADC1->SAMPTR2 |=					\
			GPIO_ADC_sampletime << (0 * 3)		\
		|	GPIO_ADC_sampletime << (1 * 3)		\
		|	GPIO_ADC_sampletime << (2 * 3)		\
		|	GPIO_ADC_sampletime << (3 * 3)		\
		|	GPIO_ADC_sampletime << (4 * 3)		\
		|	GPIO_ADC_sampletime << (5 * 3)		\
		|	GPIO_ADC_sampletime << (6 * 3)		\
		|	GPIO_ADC_sampletime << (7 * 3)		\
		|	GPIO_ADC_sampletime << (8 * 3)		\
		|	GPIO_ADC_sampletime << (9 * 3);		\
	ADC1->SAMPTR1 &= 0;					\
	ADC1->SAMPTR1 |=					\
			GPIO_ADC_sampletime << (0 * 3)		\
		|	GPIO_ADC_sampletime << (1 * 3)		\
		|	GPIO_ADC_sampletime << (2 * 3)		\
		|	GPIO_ADC_sampletime << (3 * 3)		\
		|	GPIO_ADC_sampletime << (4 * 3)		\
		|	GPIO_ADC_sampletime << (5 * 3);		\
})


#define GPIO_ADC_set_power(enable)					\
	((enable)	?	ADC1->CTLR2 |= ADC_ADON		:	\
				ADC1->CTLR2 &= ~(ADC_ADON)	)	\

#define GPIO_ADC_calibrate() ({			\
	ADC1->CTLR2 |= ADC_RSTCAL;		\
	while(ADC1->CTLR2 & ADC_RSTCAL);	\
	ADC1->CTLR2 |= ADC_CAL;			\
	while(ADC1->CTLR2 & ADC_CAL);		\
})

// large but will likely only ever be called once
static inline void GPIO_ADCinit() {
	// select ADC clock source
	// ADCCLK = 24 MHz => RCC_ADCPRE = 0: divide by 2
	RCC->CFGR0 &= ~(0x1F<<11);

	// enable clock to the ADC
	RCC->APB2PCENR |= RCC_APB2Periph_ADC1;

	// Reset the ADC to init all regs
	RCC->APB2PRSTR |= RCC_APB2Periph_ADC1;
	RCC->APB2PRSTR &= ~RCC_APB2Periph_ADC1;

	// set sampling time for all inputs to 241 cycles
	GPIO_ADC_set_sampletimes_all(GPIO_ADC_sampletime);

	// set trigger to software
	ADC1->CTLR2 |= ADC_EXTSEL;

	// pre-clear conversion queue
	ADC1->RSQR1 = 0;
	ADC1->RSQR2 = 0;
	ADC1->RSQR3 = 0;

	// power the ADC
	GPIO_ADC_set_power(1);
	GPIO_ADC_calibrate();
}


static inline uint16_t GPIO_analogRead(enum GPIO_analog_inputs input) {
	// set mux to selected input
	ADC1->RSQR3 = input;

	Delay_Us(GPIO_ADC_MUX_DELAY);

	// start sw conversion (auto clears)
	ADC1->CTLR2 |= ADC_SWSTART;
	
	// wait for conversion complete
	while(!(ADC1->STATR & ADC_EOC)) {}
	
	// get result
	return ADC1->RDATAR;
}

//######## small internal function definitions, static inline



//######## implementation block
#define CH32V003_GPIO_ITER_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_GPIO_ITER_IMPLEMENTATION)



#endif // CH32V003_GPIO_ITER_IMPLEMENTATION
#endif // CH32V003_GPIO_BR_H
