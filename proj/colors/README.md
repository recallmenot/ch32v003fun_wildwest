# HSV control for RGB LEDs

This library + example provide the ability to generate 8 and 10 bit RGB data from HSV and display it on an RGB led, using PWM.

`colorm_spectrum` and `colorm_rainbow` originate from the [FastLED library](https://github.com/FastLED/FastLED/tree/master).

# RGB example

Connect red to D4, green to D3 and blue to C0.
Calibrate the the whitepoint by selecting the right resisors as described in colormath.h color adjustments section.
For reference, with my specific RGB led, at 3.3V, I chose 56, 330 and 470 Ohm.
You will likely need different values.

The desired demo may be selected in colors.c by setting it to 1 and the others to 0.  
