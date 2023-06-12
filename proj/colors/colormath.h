//######## necessities

// include guards
#ifndef COLORMATH_H
#define COLORMATH_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description

This library provides the ability to derive RGB (PWM) values for controlling RGB LEDs from values in the HSV color space.
This enables smooth fades between desired colors.

"colorwheel" is a simple maximum-brightness implementation of the hue space, it is the smallest for when you only need colors.
As such, brightness will vary if you fade.

"spectrum" and "rainbow" both originate from the FastLED library.
They have been stripped down to make them easier to understand.

"spectrum" is a mathematically correct implementation of the HSV color space.
It has been modified to accept hue values 0..255 instead of 0..191, a 10 bit version has been added (0..1023 in, 0..1023 out).
Both use 16bit calculations internally to provide for more accuracy and avoid the issue that 256 is not divisible by 3.

"rainbow" is FastLEDs improved HSV function with nonlinear brightness curves.
The difference is explained here:
https://github.com/FastLED/FastLED/wiki/FastLED-HSV-Colors
The spectrum of rainbow visually looks the most even.

Our eyes do not perceive brightness linearly but rather exponentially (gamma is usually 2.2), which is why apparrent LED brightness does not correlate linearly to PWM control.
Since we generate color additively using red, green and blue LEDs, the values must be scaled accordingly.
This is where the increased 10 bit resolution is of great use as the lower values need to be tightly distributed, compared to the high values (y=x^2).
*/


/*######## library usage and configuration

Before including this library, unlock the implementation block as it holds all the H(SV) to RGB functions:
#define COLORMATH_IMPLEMENTATION

First, you generate RGB values from H(SV) values.
Basic predefined color target hues for each function can be found in "colortargets.h".

Then, you scale the RGB values to PWM values.

Now you may display the RGB values on the LEDs.
*/


/*######## color adjustments

It is recommended that before expecting any serious color from your LED, you calibrate their whitepoint.
The goal is 5600k day-white.

To do so, set all three pins of your RGB led to high or tie them to VCC.
You need to find the manufacturers specifications on Vf of the three diodes at maximum recommended current (usually 20mA).
A good starting point is to give the red led full current, the green led 1/4 of full current and the blue led 1/2 of full current.
From there on, you need to experiment with the resistors of the R, G and B channels, paying attention to not exceed the rated current of any of the diodes.

To not go blind while working on the LEDs and achieve better color mixing, you may diffuse their light.
A thin white fabric or piece of tissue paper may be used externally.
Clear 5mm RGB LEDs may be painted white, when the paint is dry you may file off the dome, being careful not to damage the leads.
The resulting coarse surface makes for an excellent diffusor, the white surrounding paint reflects the produced light to it.

The rainbow functions can additionally be configured to suit your particular LEDs, though this should not be necessary if whitepoint is set correctly and will reduce total color resolution.
Yellow has a higher inherent brightness than any other color; 'pure' yellow is perceived to be 93% as bright as white.
In order to make yellow appear the correct relative brightness, it has to be rendered brighter than all other colors.
Level Y1 is a moderate boost, the default.
#define colorm_rainbow_Y1			1
Level Y2 is a strong boost.
#define colorm_rainbow_Y2			1
If you need to tame a very bright green LED of an RGB led and adjusting brightness with resistors was not an option, you can adjust it in software.
G2 controlls whether to divide all greens by two.
#define colorm_rainbow_G2			0
Gscale is what to scale green down by.
#define colorm_rainbow_Gscale			0

Any of the defines only take effect if they are defined before including this library.
*/


//######## function overview (declarations): use these!

void colorm_colorwheel_8bit(const uint16_t hue, uint8_t* RGB);
void colorm_colorwheel_10bit(const uint16_t hue, uint16_t* RGB);

void colorm_spectrum_8bit ( const uint8_t hue, const uint8_t sat, const uint8_t val, uint8_t* RGB);
void colorm_spectrum_10bit(const uint16_t hue, const uint16_t sat, const uint16_t val, uint16_t* RGB);

void colorm_rainbow_8bit( const uint8_t hue, const uint8_t sat, uint8_t val, uint8_t* RGB);
void colorm_rainbow_10bit( const uint16_t hue, const uint16_t sat, uint16_t val, uint16_t* RGB);

// convert a single brightness value to a PWM value
#define colorm_brightness_to_pwm(x, bits)

// convert an RGB array (of brightness values) to PWM values 
#define colorm_RGB_to_PWM(RGB, bits)



//######## internal function declarations



//######## preprocessor macros

#define bit_NUMn(bit, multiplier)		(((uint16_t)1 << bit) * multiplier)
#define bit_top(bit, multiplier)		(bit_NUMn(bit, multiplier) - 1)

#undef colorm_brightness_to_pwm
// convert a single brightness value to a PWM value
#define colorm_brightness_to_pwm2(x, bits)	colorm_brightness_to_pwm_##bits(x)
#define colorm_brightness_to_pwm(x, bits)	colorm_brightness_to_pwm2(x, bits)
#define colorm_brightness_to_pwm_8(x)		((((uint16_t)x) * (1 + ((uint16_t)x))) >> 8)
#define colorm_brightness_to_pwm_10(x)		((((uint32_t)x) * (1 + ((uint32_t)x))) >> 10)

#undef colorm_RGB_to_PWM
// convert an RGB array to PWM values 
#define colorm_RGB_to_PWM(RGB, bits) ({				\
	RGB[0] = colorm_brightness_to_pwm(RGB[0], bits);	\
	RGB[1] = colorm_brightness_to_pwm(RGB[1], bits);	\
	RGB[2] = colorm_brightness_to_pwm(RGB[2], bits);	\
})



// hue section quadrants for colorm_spectrum_
// scaled to full 16 bit to avoid issues at 255 (256th number, not divisible by 85)
#define HSV_SECTION_3_8bit (0x5555)
// scaled to full 16 bit for increased resolution (it's free real estate)
#define HSV_SECTION_3_10bit (0x5555)

/// Apply dimming compensation to values
#define APPLY_DIMMING(X) (X)



/// Scale one byte by a second one, which is treated as
/// the numerator of a fraction whose denominator is 256. 
/// In other words, it computes i * (scale / 256)
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
#define scale8( i, scale)			((((uint16_t)i) * (1 + (uint16_t)(scale))) >> 8)
#define scale10( i, scale)			((((uint32_t)i) * (1 + (uint32_t)(scale))) >> 10)

/// The "video" version of scale8() guarantees that the output will
/// be only be zero if one or both of the inputs are zero. 
/// If both inputs are non-zero, the output is guaranteed to be non-zero.  
/// This makes for better "video"/LED dimming, at the cost of
/// several additional cycles.
/// @param i input value to scale
/// @param scale scale factor, in n/256 units
/// @returns scaled value
/// @see scale8()
#define scale8_video( i, scale)			((((uint16_t)i * (uint16_t)scale) >> 8) + ((i&&scale)?1:0))
#define scale10_video( i, scale)		((((uint32_t)i * (uint32_t)scale) >> 10) + ((i&&scale)?1:0))

/// Force a variable reference to avoid compiler over-optimization. 
/// Sometimes the compiler will do clever things to reduce
/// code size that result in a net slowdown, if it thinks that
/// a variable is not used in a certain location.
/// This macro does its best to convince the compiler that
/// the variable is used in this location, to help control
/// code motion and de-duplication that would result in a slowdown.
#define FORCE_REFERENCE(var)			asm volatile( "" : : "r" (var) )

// hue section quadrants for colorm_rainbow_
#define K255 255
#define K171 171
#define K170 170
#define K85  85

#define K1023 1023
#define K683 683
#define K682 682
#define K341 341

// default rainbow configuration
#ifndef colorm_rainbow_Y1
#define colorm_rainbow_Y1			1
#endif

#ifndef colorm_rainbow_Y2
#define colorm_rainbow_Y2			0
#endif

#ifndef colorm_rainbow_G2
#define colorm_rainbow_G2			0
#endif

#ifndef colorm_rainbow_Gscale
#define colorm_rainbow_Gscale			0
#endif

// maintenance defines
#if !defined(SYSTEM_CORE_CLOCK)
#define COLORMATH_IMPLEMENTATION
#endif



//######## implementation block
//#define COLORMATH_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(COLORMATH_IMPLEMENTATION)

void colorm_colorwheel_8bit(const uint16_t hue, uint8_t* RGB) {
	if (hue < bit_NUMn(8, 1)) {
		RGB[0] = bit_top(8, 1);
		RGB[1] = hue - 0;			// ++G
		RGB[2] = 0;
	}
	else if (hue < bit_NUMn(8, 2)) {
		RGB[0] = bit_top(8, 2) - hue;		// --R
		RGB[1] = bit_top(8, 1);
		RGB[2] = 0;
	}
	else if (hue < bit_NUMn(8, 3)) {
		RGB[0] = 0;
		RGB[1] = bit_top(8, 1);
		RGB[2] = hue - bit_NUMn(8, 2);		// ++B
	}
	else if (hue < bit_NUMn(8, 4)) {
		RGB[0] = 0;
		RGB[1] = bit_top(8, 4) - hue;		// --G
		RGB[2] = bit_top(8, 1);
	}
	else if (hue < bit_NUMn(8, 5)) {
		RGB[0] = hue - bit_NUMn(8, 4);		// ++R
		RGB[1] = 0;
		RGB[2] = bit_top(8, 1);
	}
	else if (hue < bit_NUMn(8, 6)) {
		RGB[0] = bit_top(8, 1);
		RGB[1] = 0;
		RGB[2] = bit_top(8, 6) - hue;		// --B
	}
}

void colorm_colorwheel_10bit(const uint16_t hue, uint16_t* RGB) {
	if (hue < bit_NUMn(10, 1)) {
		RGB[0] = bit_top(10, 1);
		RGB[1] = hue - 0;			// ++G
		RGB[2] = 0;
	}
	else if (hue < bit_NUMn(10, 2)) {
		RGB[0] = bit_top(10, 2) - hue;		// --R
		RGB[1] = bit_top(10, 1);
		RGB[2] = 0;
	}
	else if (hue < bit_NUMn(10, 3)) {
		RGB[0] = 0;
		RGB[1] = bit_top(10, 1);
		RGB[2] = hue - bit_NUMn(10, 2);		// ++B
	}
	else if (hue < bit_NUMn(10, 4)) {
		RGB[0] = 0;
		RGB[1] = bit_top(10, 4) - hue;		// --G
		RGB[2] = bit_top(10, 1);
	}
	else if (hue < bit_NUMn(10, 5)) {
		RGB[0] = hue - bit_NUMn(10, 4);		// ++R
		RGB[1] = 0;
		RGB[2] = bit_top(10, 1);
	}
	else if (hue < bit_NUMn(10, 6)) {
		RGB[0] = bit_top(10, 1);
		RGB[1] = 0;
		RGB[2] = bit_top(10, 6) - hue;		// --B
	}
}



void colorm_spectrum_8bit ( const uint8_t hue, const uint8_t sat, const uint8_t val, uint8_t* RGB)
{
	// Convert hue, saturation and brightness ( HSV/HSB ) to RGB
	// "Dimming" is used on saturation and brightness to make
	// the output more visually linear.

	// Apply dimming curves
	uint8_t value = APPLY_DIMMING( val );
	uint8_t saturation = sat;

	// The brightness floor is minimum number that all of
	// R, G, and B will be set to.
	uint8_t invsat = APPLY_DIMMING( 255 - saturation);
	uint8_t brightness_floor = (value * invsat) / 256;

	// The color amplitude is the maximum amount of R, G, and B
	// that will be added on top of the brightness_floor to
	// create the specific hue desired.
	uint8_t color_amplitude = value - brightness_floor;

	// Figure out which section of the hue wheel we're in,
	// and how far offset we are withing that section
	uint8_t section = ((((uint16_t)hue) << 8) / HSV_SECTION_3_8bit); // 0..2
	uint16_t offset = (((uint16_t)hue) << 8) % HSV_SECTION_3_8bit;  // 0..63

	uint16_t rampup = offset; // 0..63
	uint16_t rampdown = (HSV_SECTION_3_8bit - 1) - offset; // 63..0

   // compute color-amplitude-scaled-down versions of rampup and rampdown
	// dividing by a third of the range = the same as muliplying by 3 and then dividing by the range
	uint8_t rampup_amp_adj	 = (rampup * color_amplitude) / HSV_SECTION_3_8bit;
	uint8_t rampdown_amp_adj   = (rampdown * color_amplitude) / HSV_SECTION_3_8bit;

	// add brightness_floor offset to everything
	uint8_t rampup_adj_with_floor	= rampup_amp_adj   + brightness_floor;
	uint8_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

	if( section ) {
		if( section == 1) {
			// section 1: 0x40..0x7F
			RGB[0] = brightness_floor;
			RGB[1] = rampdown_adj_with_floor;
			RGB[2] = rampup_adj_with_floor;
		} else {
			// section 2; 0x80..0xBF
			RGB[0] = rampup_adj_with_floor;
			RGB[1] = brightness_floor;
			RGB[2] = rampdown_adj_with_floor;
		}
	} else {
		// section 0: 0x00..0x3F
		RGB[0] = rampdown_adj_with_floor;
		RGB[1] = rampup_adj_with_floor;
		RGB[2] = brightness_floor;
	}
}

void colorm_spectrum_10bit(const uint16_t hue, const uint16_t sat, const uint16_t val, uint16_t* RGB)
{
	// Convert hue, saturation, and brightness (HSV/HSB) to RGB
	// "Dimming" is used on saturation and brightness to make
	// the output more visually linear.

	// Apply dimming curves
	uint16_t value = APPLY_DIMMING(val);
	uint16_t saturation = sat;

	// The brightness floor is minimum number that all of
	// R, G, and B will be set to.
	uint16_t invsat = APPLY_DIMMING(1023 - saturation);
	uint16_t brightness_floor = (value * invsat) / 1024;

	// The color amplitude is the maximum amount of R, G, and B
	// that will be added on top of the brightness_floor to
	// create the specific hue desired.
	uint16_t color_amplitude = value - brightness_floor;

	// Figure out which section of the hue wheel we're in,
	// and how far offset we are within that section
	uint8_t section = (hue << 6) / HSV_SECTION_3_10bit; // 0..2
	uint16_t offset = (hue << 6) % HSV_SECTION_3_10bit; // 0..255

	uint16_t rampup = offset; // 0..255
	uint16_t rampdown = (HSV_SECTION_3_10bit - 1) - offset; // 255..0

	// compute color-amplitude-scaled-down versions of rampup and rampdown
	// dividing by a third of the range = the same as muliplying by 3 and then dividing by the range
	uint16_t rampup_amp_adj = (rampup * color_amplitude) / HSV_SECTION_3_10bit;
	uint16_t rampdown_amp_adj = (rampdown * color_amplitude) / HSV_SECTION_3_10bit;

	// add brightness_floor offset to everything
	uint16_t rampup_adj_with_floor = rampup_amp_adj + brightness_floor;
	uint16_t rampdown_adj_with_floor = rampdown_amp_adj + brightness_floor;

	if (section) {
		if (section == 1) {
			// section 1: 0x100..0x1FF
			RGB[0] = brightness_floor;
			RGB[1] = rampdown_adj_with_floor;
			RGB[2] = rampup_adj_with_floor;
		} else {
			// section 2; 0x200..0x2FF
			RGB[0] = rampup_adj_with_floor;
			RGB[1] = brightness_floor;
			RGB[2] = rampdown_adj_with_floor;
		}
	} else {
		// section 0: 0x00..0xFF
		RGB[0] = rampdown_adj_with_floor;
		RGB[1] = rampup_adj_with_floor;
		RGB[2] = brightness_floor;
	}
}






void colorm_rainbow_8bit( const uint8_t hue, const uint8_t sat, uint8_t val, uint8_t* RGB)
{
	uint8_t offset = hue & 0x1F; // 0..31
	
	uint8_t offset8 = offset << 3;			// *= 8
	
	uint8_t third = scale8( offset8, (256 / 3)); // max = 85
	
	uint8_t r, g, b;
	
	if( ! (hue & 0x80) ) {
		// 0XX
		if( ! (hue & 0x40) ) {
			// 00X
			//section 0-1
			if( ! (hue & 0x20) ) {
				// 000
				//case 0: // R -> O
				r = K255 - third;
				g = third;
				b = 0;
				FORCE_REFERENCE(b);
			} else {
				// 001
				//case 1: // O -> Y
				if( colorm_rainbow_Y1 ) {
					r = K171;
					g = K85 + third ;
					b = 0;
					FORCE_REFERENCE(b);
				}
				if( colorm_rainbow_Y2 ) {
					r = K170 + third;
					//uint8_t twothirds = (third << 1);
					uint8_t twothirds = scale8( offset8, ((256 * 2) / 3)); // max=170
					g = K85 + twothirds;
					b = 0;
					FORCE_REFERENCE(b);
				}
			}
		} else {
			//01X
			// section 2-3
			if( !  (hue & 0x20) ) {
				// 010
				//case 2: // Y -> G
				if( colorm_rainbow_Y1 ) {
					//uint8_t twothirds = (third << 1);
					uint8_t twothirds = scale8( offset8, ((256 * 2) / 3)); // max=170
					r = K171 - twothirds;
					g = K170 + third;
					b = 0;
					FORCE_REFERENCE(b);
				}
				if( colorm_rainbow_Y2 ) {
					r = K255 - offset8;
					g = K255;
					b = 0;
					FORCE_REFERENCE(b);
				}
			} else {
				// 011
				// case 3: // G -> A
				r = 0;
				FORCE_REFERENCE(r);
				g = K255 - third;
				b = third;
			}
		}
	} else {
		// section 4-7
		// 1XX
		if( ! (hue & 0x40) ) {
			// 10X
			if( ! ( hue & 0x20) ) {
				// 100
				//case 4: // A -> B
				r = 0;
				FORCE_REFERENCE(r);
				//uint8_t twothirds = (third << 1);
				uint8_t twothirds = scale8( offset8, ((256 * 2) / 3)); // max=170
				g = K171 - twothirds; //K170?
				b = K85  + twothirds;
				
			} else {
				// 101
				//case 5: // B -> P
				r = third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K255 - third;
				
			}
		} else {
			if( !  (hue & 0x20)  ) {
				// 110
				//case 6: // P -- K
				r = K85 + third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K171 - third;
				
			} else {
				// 111
				//case 7: // K -> R
				r = K170 + third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K85 - third;
			}
		}
	}
	
	// This is one of the good places to scale the green down,
	// although the client can scale green down as well.
	if( colorm_rainbow_G2 ) g = g >> 1;
	if( colorm_rainbow_Gscale ) g = scale8_video( g, colorm_rainbow_Gscale);
	
	// Scale down colors if we're desaturated at all
	// and add the brightness_floor to r, g, and b.
	if( sat != 255 ) {
		if( sat == 0) {
			r = 255; b = 255; g = 255;
		} else {
			uint8_t desat = 255 - sat;
			desat = scale8_video( desat, desat);

			uint8_t satscale = 255 - desat;
			//satscale = sat; // uncomment to revert to pre-2021 saturation behavior

			//nscale8x3_video( r, g, b, sat);
			r = scale8( r, satscale);
			g = scale8( g, satscale);
			b = scale8( b, satscale);
			uint8_t brightness_floor = desat;
			r += brightness_floor;
			g += brightness_floor;
			b += brightness_floor;
		}
	}
	
	// Now scale everything down if we're at value < 255.
	if( val != 255 ) {
		val = scale8_video( val, val);
		if( val == 0 ) {
			r=0; g=0; b=0;
		} else {
			// nscale8x3_video( r, g, b, val);
			r = scale8( r, val);
			g = scale8( g, val);
			b = scale8( b, val);
		}
	}
	
	RGB[0] = r;
	RGB[1] = g;
	RGB[2] = b;
}

void colorm_rainbow_10bit( const uint16_t hue, const uint16_t sat, uint16_t val, uint16_t* RGB)
{
	uint8_t offset = hue & 0x7F; // 0..127
	
	uint16_t offset8 = offset << 3;						// *= 8

	uint32_t third = scale10( offset8, (1024 / 3)); // max = 341
	
	uint32_t r, g, b;
	
	if( ! (hue & 0x200) ) {
		// 0XX
		if( ! (hue & 0x100) ) {
			// 00X
			//section 0-1
			if( ! (hue & 0x80) ) {
				// 000
				//case 0: // R -> O
				r = K1023 - third;
				g = third;
				b = 0;
				FORCE_REFERENCE(b);
			} else {
				// 001
				//case 1: // O -> Y
				if( colorm_rainbow_Y1 ) {
					r = K683;
					g = K341 + third ;
					b = 0;
					FORCE_REFERENCE(b);
				}
				if( colorm_rainbow_Y2 ) {
					r = K682 + third;
					//uint8_t twothirds = (third << 1);
					uint32_t twothirds = scale10( offset8, ((1023 * 2) / 3)); // max=682
					g = K341 + twothirds;
					b = 0;
					FORCE_REFERENCE(b);
				}
			}
		} else {
			//01X
			// section 2-3
			if( !  (hue & 0x80) ) {
				// 010
				//case 2: // Y -> G
				if( colorm_rainbow_Y1 ) {
					//uint8_t twothirds = (third << 1);
					uint32_t twothirds = scale10( offset8, ((1023 * 2) / 3)); // max=682
					r = K683 - twothirds;
					g = K682 + third;
					b = 0;
					FORCE_REFERENCE(b);
				}
				if( colorm_rainbow_Y2 ) {
					r = K1023 - offset8;
					g = K1023;
					b = 0;
					FORCE_REFERENCE(b);
				}
			} else {
				// 011
				// case 3: // G -> A
				r = 0;
				FORCE_REFERENCE(r);
				g = K1023 - third;
				b = third;
			}
		}
	} else {
		// section 4-7
		// 1XX
		if( ! (hue & 0x100) ) {
			// 10X
			if( ! ( hue & 0x80) ) {
				// 100
				//case 4: // A -> B
				r = 0;
				FORCE_REFERENCE(r);
				//uint8_t twothirds = (third << 1);
				uint32_t twothirds = scale10( offset8, ((1023 * 2) / 3)); // max=682
				g = K683 - twothirds; //K682?
				b = K341  + twothirds;
				
			} else {
				// 101
				//case 5: // B -> P
				r = third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K1023 - third;
				
			}
		} else {
			if( !  (hue & 0x80)  ) {
				// 110
				//case 6: // P -- K
				r = K341 + third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K683 - third;
				
			} else {
				// 111
				//case 7: // K -> R
				r = K682 + third;
				g = 0;
				FORCE_REFERENCE(g);
				b = K341 - third;
			}
		}
	}
	
	// This is one of the good places to scale the green down,
	// although the client can scale green down as well.
	if( colorm_rainbow_G2 ) g = g >> 1;
	if( colorm_rainbow_Gscale ) g = scale10_video( g, colorm_rainbow_Gscale);
	
	// Scale down colors if we're desaturated at all
	// and add the brightness_floor to r, g, and b.
	if( sat != 1023 ) {
		if( sat == 0) {
			r = 1023; b = 1023; g = 1023;
		} else {
			uint16_t desat = 1023 - sat;
			desat = scale10_video( desat, desat);

			uint16_t satscale = 1023 - desat;
			//satscale = sat; // uncomment to revert to pre-2021 saturation behavior

			//nscale8x3_video( r, g, b, sat);
			r = scale10( r, satscale);
			g = scale10( g, satscale);
			b = scale10( b, satscale);
			uint16_t brightness_floor = desat;
			r += brightness_floor;
			g += brightness_floor;
			b += brightness_floor;
		}
	}
	
	// Now scale everything down if we're at value < 1023.
	if( val != 1023 ) {
		val = scale10_video( val, val);
		if( val == 0 ) {
			r=0; g=0; b=0;
		} else {
			// nscale8x3_video( r, g, b, val);
			r = scale10( r, val);
			g = scale10( g, val);
			b = scale10( b, val);
		}
	}
	
	RGB[0] = r;
	RGB[1] = g;
	RGB[2] = b;
}



#endif // COLORMATH_IMPLEMENTATION
#endif // COLORMATH_H
