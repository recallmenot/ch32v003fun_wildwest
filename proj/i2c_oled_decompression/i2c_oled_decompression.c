/*
 * example for decompressing images to SSD1306
 * 05-22-2023 recallmenot
 * I2C SSD1306 library by E. Brombaugh
 */

#include <stdio.h>

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK
#include "../../ch32v003fun/ch32v003fun.h"

// what type of OLED - uncomment just one
//#define SSD1306_64X32
#define SSD1306_128X32
//#define SSD1306_128X64
#include "../../examples/i2c_oled/ssd1306_i2c.h"
#include "../../examples/i2c_oled/ssd1306.h"

#define MALLOC_OVERRIDE 0

#if MALLOC_OVERRIDE > 0
#include <stdlib.h>
#endif

#define COMP_PACKBITS		1
#define COMP_HEATSHRINK		0
#if (COMP_PACKBITS == 1) && (COMP_HEATSHRINK == 1)
#error "please only enable packbits OR heatshrink"
#endif
#if (COMP_PACKBITS == 0) && (COMP_HEATSHRINK == 0)
#error "please enable packbits or heatshrink"
#endif

#include "bomb_i_packed.h"
#include "bomb_i_heatshrunk.h"
#include "rocket_i_packed.h"
#include "rocket_i_heatshrunk.h"

#include "compression.h"
#include "decompress_image.h"

#define STDOUT_UART
#define LOGimage 0



int main()
{
	// 48MHz internal clock
	SystemInit48HSI();

	// start serial @ default 115200bps
#ifdef STDOUT_UART
	SetupUART( UART_BRR );
#else
	SetupDebugPrintf();
#endif
	printf("\r\r\n\ni2c_oled example\n\r");

	// init i2c and oled
	Delay_Ms( 100 );	// give OLED some more time
	printf("initializing i2c oled...");
	if(!ssd1306_i2c_init())
	{
		ssd1306_init();
		printf("done.\n\r");
		
		printf("Looping on test modes...");
		while(1)
		{
			for(uint8_t mode=0;mode<3;mode++)
			{
				// clear buffer for next mode
				ssd1306_setbuf(0);

				switch(mode)
				{
					case 0:
						printf("buffer fill with binary\n\r");
						for(int i=0;i<sizeof(ssd1306_buffer);i++)
							ssd1306_buffer[i] = i;
						break;
					case 1:
						printf("draw decompressed bomb\n\r");
						#if COMP_PACKBITS == 1
						printf("packbits\n\r");
						unpack_image(16, 0, bomb_i_packed, bomb_i_packed_len, 32, 32, 0);
						#elif COMP_HEATSHRINK == 1
						printf("heatshrink\n\r");
						unpack_image(16, 0, bomb_i_heatshrunk, bomb_i_heatshrunk_len, 32, 32, 0);
						#endif
						break;
					case 2:
						printf("launch a compressed rocket\n\r");
						const uint16_t frame_i = 33;
						uint32_t frame_t = 0;
						uint32_t decomp_t = 0;
						for (uint8_t loop = 0; loop < 6; loop++) {
							for (uint8_t image = 0;image < 26; image++) {
								while (Time_Ms - frame_t < frame_i) {};
								//ssd1306_setbuf(0);
								frame_t = Time_Ms;		// start timer for next frame, but also missappropriate as stopwatch
								#if COMP_PACKBITS == 1
								unpack_image_number(0, 0, rocket_i_packed, rocket_i_packed_len, 32, 32, image, loop % 6);
								#elif COMP_HEATSHRINK == 1
								unpack_image_number(0, 0, rocket_i_heatshrunk, rocket_i_heatshrunk_len, 32, 32, image, loop % 6);
								#endif
								ssd1306_refresh();
								decomp_t += Time_Ms - frame_t;	// time to decompress and display last frame
							}
						}
						decomp_t /= 26 * 6;
						printf("spent %lu ms decompressing and writing each frame\n\r", decomp_t);
						break;
					default:
						break;
				}
				ssd1306_refresh();
			
				Delay_Ms(2000);
			}
		}
	}
	else
		printf("failed.\n\r");
	
	printf("Stuck here forever...\n\r");
	while(1);
}
