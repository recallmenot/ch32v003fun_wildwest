//######## necessities

// include guards
#ifndef CH32V003_BTNi_H
#define CH32V003_BTNi_H

// includes
#include<stdint.h>								//uintN_t support
#include"../../ch32v003fun/ch32v003fun.h"



/*######## library description


*/


/*######## library usage and configuration


*/


/*######## requirements: configure before #including this library!
Before #including this library, you will need to configure your desired timings in Ticks.
Ticks_from_Ms(n) can be used for the conversion.

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


enum BTNi_states BTNi_interpret_state(uint8_t button_pressed, uint32_t* time_memory, uint8_t* state_memory );

#define BTNi_analog_threshold(input, threshold) (input > analog_threshold ? 1 : 0)

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


enum BTNi_states BTNi_interpret_state(uint8_t button_pressed, uint32_t* time_memory, uint8_t* state_memory ) {
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
				return BTNi_state_pushed;
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
				return BTNi_state_held;
				break;
		}
	}
	else {
		*state_memory = BTNi_state_open;
		return BTNi_state_open;
	}
	return BTNi_state_open;
}
// hide functionalities behind the interval defines?
// double-press needs upstroke detection?



//######## small internal function definitions, static inline



//######## implementation block
//#define CH32V003_BTNi_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_BTNi_IMPLEMENTATION)



#endif // CH32V003_BTNi_IMPLEMENTATION
#endif // CH32V003_BTNi_H


