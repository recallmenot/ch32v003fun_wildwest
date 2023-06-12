//######## necessities

// include guards
#ifndef COLORTARGETS_H
#define COLORTARGETS_H

// includes
#include<stdint.h>						//uintN_t support



/*######## library description

This is a colection of basic pre-defined color target hues for the H(SV) to RGB functions of colormath.h
*/


/*######## library usage and configuration

Each function has an 8bit and a 10bit set of hues, use the appropriate ones for your application.
*/


//######## color hues: use these!

// hues for colorwheel_ functions
#define colort_c8_red			0
#define colort_c8_red_orange		68
#define colort_c8_orange		140
#define colort_c8_yellow		255
#define colort_c8_green_lawn		383
#define colort_c8_green_super		512
#define colort_c8_green_spring		640
#define colort_c8_cyan			767
#define colort_c8_blue_deepsky		832
#define colort_c8_blue			1023
#define colort_c8_violet		1156
#define colort_c8_pink			1279
#define colort_c8_magneta		1339
#define colort_c8_pink_deep2		1399

#define colort_c10_red			0
#define colort_c10_red_orange		273
#define colort_c10_orange		562
#define colort_c10_yellow		1023
#define colort_c10_green_lawn		1533
#define colort_c10_green_super		2047
#define colort_c10_green_spring		2563	
#define colort_c10_cyan			3071	
#define colort_c10_blue_deepsky		3329	
#define colort_c10_blue			4095	
#define colort_c10_violet		4626
#define colort_c10_pink			5119	
#define colort_c10_magneta		5356
#define colort_c10_pink_deep2		5597



// hues for spectrum_ functions
#define colort_s8_red			0
#define colort_s8_red_orange		18
#define colort_s8_orange		30
#define colort_s8_yellow		42
#define colort_s8_green_lawn		56
#define colort_s8_green_super		84
#define colort_s8_green_spring		113
#define colort_s8_cyan			127
#define colort_s8_blue_deepsky		133
#define colort_s8_blue			169
#define colort_s8_violet		199
#define colort_s8_pink			212
#define colort_s8_magneta		217
#define colort_s8_pink_deep2		225

#define colort_s10_red			0
#define colort_s10_red_orange		72
#define colort_s10_orange		121
#define colort_s10_yellow		170
#define colort_s10_green_lawn		226
#define colort_s10_green_super		340
#define colort_s10_green_spring		455	
#define colort_s10_cyan			511	
#define colort_s10_blue_deepsky		535	
#define colort_s10_blue			681	
#define colort_s10_violet		798
#define colort_s10_pink			852	
#define colort_s10_magneta		874
#define colort_s10_pink_deep2		904



// hues for rainbow_ functions
#define colort_r8_red			0
#define colort_r8_red_orange		21
#define colort_r8_orange		35
#define colort_r8_yellow		64
#define colort_r8_green_lawn		77
#define colort_r8_green_super		96
#define colort_r8_green_spring		128
#define colort_r8_cyan			136
#define colort_r8_blue_deepsky		140
#define colort_r8_blue			160
#define colort_r8_violet		193
#define colort_r8_pink			208
#define colort_r8_magneta		214
#define colort_r8_pink_deep2		223

#define colort_r10_red			0
#define colort_r10_red_orange		81
#define colort_r10_orange		141
#define colort_r10_yellow		256
#define colort_r10_green_lawn		307
#define colort_r10_green_super		384
#define colort_r10_green_spring		512	
#define colort_r10_cyan			544	
#define colort_r10_blue_deepsky		558	
#define colort_r10_blue			640	
#define colort_r10_violet		771
#define colort_r10_pink			832	
#define colort_r10_magneta		857
#define colort_r10_pink_deep2		891





#endif // COLORTARGETS_H
