//######## necessities

// include guards
#ifndef CH32V003_BTNi_H
#define CH32V003_BTNi_H

// includes
#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"



/*######## library description
This library derives well-behaved button states from digital and analog sources.

The only thing that makes it specific to CH32V003 is the use of SysTick for retrieving the current time.

*/


/*######## library usage and configuration
BTNi_interpret_state takes in a digital button signal (1 for pressed, 0 for open) and requires memory for time and state.

The output is the state memory, "encoded" as BTNi_states.

Your program only needs to call BTNi_interpret_state (~ 1 ms recommended) and then perform the desired action when the state memory holds your desired state.

BTNi_analog_threshold may be used to apply a threshold to an analog value, the result returned is digital.
*/



/*######## requirements: configure before #including this library!
Before #including this library, you will need to configure your desired timings in Ticks.
Ticks_from_Ms(n) may be used for the conversion.

#define BTNi_i_DEBOUNCE		Ticks_from_Ms(10)
#define BTNi_i_HOLD		Ticks_from_Ms(1000)
#define BTNi_i_HOLD_REPEAT	Ticks_from_Ms(250)
*/



//######## button states: use the ones with comments in your program!

enum BTNi_states {
	BTNi_state_open,		// not pushed
	BTNi_state_released,
	BTNi_state_wait_for_push,
	BTNi_state_pushed,		// push registered
	BTNi_state_wait_for_hold,
	BTNi_state_wait_for_hold_repeat,
	BTNi_state_held,		// hold registered
};



//######## function overview (declarations): use these!

#define BTNi_analog_threshold(input, threshold)

void BTNi_interpret_state(const uint8_t button_pressed, uint32_t* time_memory, uint8_t* state_memory );



//######## internal function declarations



//######## internal variables



//######## preprocessor macros



//######## preprocessor #define requirements

#if !defined(BTNi_i_DEBOUNCE) || !defined(BTNi_i_HOLD) || !defined(BTNi_i_HOLD_REPEAT)
#error "Please define BTNi_i_DEBOUNCE, BTNi_i_HOLD and BTNi_i_HOLD_REPEAT with your preferred values in SysTicks before including this library."
// maintainance defines
#define SYSTEM_CORE_CLOCK	48000000
#define BTNi_i_DEBOUNCE		Ticks_from_Ms(10)
#define BTNi_i_HOLD		Ticks_from_Ms(1000)
#define BTNi_i_HOLD_REPEAT	Ticks_from_Ms(250)
#endif

//######## small function definitions, static inline


#undef BTNi_analog_threshold
#define BTNi_analog_threshold(input, threshold) (input > threshold ? 1 : 0)

void BTNi_interpret_state(const uint8_t button_pressed, uint32_t* time_memory, uint8_t* state_memory ) {
	if (button_pressed) {
		switch (*state_memory) {
			case BTNi_state_open:
				*time_memory = SysTick->CNT;
				*state_memory = BTNi_state_wait_for_push;
				break;
			case BTNi_state_wait_for_push:
				if (SysTick->CNT - *time_memory > BTNi_i_DEBOUNCE) {
					*state_memory = BTNi_state_pushed;
				}
				break;
			case BTNi_state_pushed:
				*time_memory = SysTick->CNT;
				*state_memory = BTNi_state_wait_for_hold;
				//return BTNi_state_pushed;
				break;
			case BTNi_state_wait_for_hold:
				if (SysTick->CNT - *time_memory > BTNi_i_HOLD) {
					*state_memory = BTNi_state_held;
				}
				break;
			case BTNi_state_wait_for_hold_repeat:
				if (SysTick->CNT - *time_memory > BTNi_i_HOLD_REPEAT) {
					*state_memory = BTNi_state_held;
				}	
				break;
			case BTNi_state_held:
				*time_memory = SysTick->CNT;
				*state_memory = BTNi_state_wait_for_hold_repeat;
				//return BTNi_state_held;
				break;
		}
	}
	else {
		*state_memory = BTNi_state_open;
		//return BTNi_state_open;
	}
	//return BTNi_state_open;
}
// double-press requires upstroke detection?


//######## small internal function definitions, static inline



//######## implementation block
//#define CH32V003_BTNi_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_BTNi_IMPLEMENTATION)



#endif // CH32V003_BTNi_IMPLEMENTATION
#endif // CH32V003_BTNi_H


