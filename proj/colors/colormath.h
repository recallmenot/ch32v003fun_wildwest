//######## necessities

// include guards
#ifndef COLORMATH_H
#define COLORMATH_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description


*/


/*######## library usage and configuration


*/


/*######## features: use your desired before #including this library!

#define LIBRARY_ENABLE_FEATURE
*/


//######## function overview (declarations): use these!

void colorwheel_8(const uint16_t hue, uint8_t* out_R, uint8_t* out_G, uint8_t* out_B);



//######## internal function declarations



//######## internal variables



//######## preprocessor macros


#define bit_top(bit, multiplier)	(((uint16_t)1 << bit) * multiplier)

// maintenance defines
#if !defined(SYSTEM_CORE_CLOCK)
#define COLORMATH_IMPLEMENTATION
#endif


//######## preprocessor #define requirements



//######## small function definitions, static inline



//######## small internal function definitions, static inline



//######## implementation block
//#define COLORMATH_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(COLORMATH_IMPLEMENTATION)

void colorwheel_8(const uint16_t hue, uint8_t* out_R, uint8_t* out_G, uint8_t* out_B) {
	if (hue < bit_top(8, 1)) {
		*out_R = bit_top(8, 1) - 1;
		*out_G = hue - 0;			// ++G
		*out_B = 0;
	}
	else if (hue < bit_top(8, 2)) {
		*out_R = bit_top(8, 2) - hue;		// --R
		*out_G = bit_top(8, 1) - 1;
		*out_B = 0;
	}
	else if (hue < bit_top(8, 3)) {
		*out_R = 0;
		*out_G = bit_top(8, 1) - 1;
		*out_B = hue - bit_top(8, 2);		// ++B
	}
	else if (hue < bit_top(8, 4)) {
		*out_R = 0;
		*out_G = bit_top(8, 4) - hue;		// --G
		*out_B = bit_top(8, 1) - 1;
	}
	else if (hue < bit_top(8, 5)) {
		*out_R = (hue - bit_top(8, 4));		// ++R
		*out_G = 0;
		*out_B = bit_top(8, 1) - 1;
	}
	else if (hue < bit_top(8, 6)) {
		*out_R = bit_top(8, 1) - 1;
		*out_G = 0;
		*out_B = bit_top(8, 4) - hue;		// --B
	}
}



#endif // COLORMATH_IMPLEMENTATION
#endif // COLORMATH_H
