/*
 * Example for using I2C with 128x32 graphic OLED
 * 03-29-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

// what type of OLED - uncomment just one
//#define SSD1306_64X32
#define SSD1306_128X32
//#define SSD1306_128X64

#include "../../ch32v003fun/ch32v003fun.h"
#include <stdio.h>
#include "ssd1306_i2c.h"
#include "ssd1306.h"

#define MALLOC_OVERRIDE 0

#if MALLOC_OVERRIDE > 0
#include <stdlib.h>
#endif

#define COMP_PACKBITS 1
#define COMP_HEATSHRINK 1

#include "bomb_i_packed.h"
#include "bomb_i_heatshrunk.h"
#include "compression.h"

#define STDOUT_UART
#define LOGimage

void draw_image(uint8_t* input, uint8_t width, uint8_t height, uint8_t x, uint8_t y) {
	uint8_t x_absolute;
	uint8_t y_absolute;
	// abort if lines > display height
	uint8_t pixel;
	uint8_t bytes_to_draw = width / 8;
	for (uint8_t line = 0; line < height; line++) {
		y_absolute = y + line;
		if (y_absolute >= SSD1306_H) {
			break;
		}
		for (uint8_t byte = 0; byte < bytes_to_draw; byte++) {
			#ifdef LOGimage
			printf("%02x ", input[byte + line * bytes_to_draw]);
			#endif
			for (pixel = 0; pixel < 8; pixel++) {
				x_absolute = x + 8 * (bytes_to_draw - byte) + pixel;
				if (x_absolute >= SSD1306_W) {
					break;
				}
				if(input[byte + line * bytes_to_draw] & (1 << pixel)) {
					ssd1306_drawPixel(x_absolute, y_absolute, 1);
				}
				#if defined (TIFFG4_opaque_black)
				else {
					ssd1306_drawPixel(x_absolute, y_absolute, 0);
				}
				#endif
			}
		}
		#ifdef LOGimage
		printf("\n\r");
		#endif
      	}
}



void unpack_image(uint8_t* input, uint16_t size_of_input, uint8_t width, uint8_t height, uint8_t x, uint8_t y) {
	uint32_t output_max_size = (width / 8) * height;
	#if MALLOC_OVERRIDE > 0
	uint8_t* output = malloc(output_max_size);
	#else
	uint8_t output[output_max_size];
	#endif
	data_object data = {input, size_of_input, output, output_max_size, 0, width, height, 0};
	uint16_t unpack_error = decompress_heatshrink(&data);
	#ifdef LOGimage
	printf("unpackbits return %u\n\r", unpack_error);
	printf("unpackbits sizes IN %u OUT %u\n\r", size_of_input, output_max_size);
	#endif
	draw_image(output, width, height, x, y);
	#if MALLOC_OVERRIDE > 0
	free(output);
	#endif
}


int main()
{
	// 48MHz internal clock
	SystemInit48HSI();

	// start serial @ default 115200bps
#ifdef STDOUT_UART
	SetupUART( UART_BRR );
	Delay_Ms( 100 );
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
			for(uint8_t mode=7;mode<(SSD1306_H>32?8:9);mode++)
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
						printf("pixel plots\n\r");
						for(int i=0;i<SSD1306_W;i++)
						{
							ssd1306_drawPixel(i, i/(SSD1306_W/SSD1306_H), 1);
							ssd1306_drawPixel(i, SSD1306_H-1-(i/(SSD1306_W/SSD1306_H)), 1);
						}
						break;
					
					case 2:
						{
							printf("Line plots\n\r");
							uint8_t y= 0;
							for(uint8_t x=0;x<SSD1306_W;x+=16)
							{
								ssd1306_drawLine(x, 0, SSD1306_W, y, 1);
								ssd1306_drawLine(SSD1306_W-x, SSD1306_H, 0, SSD1306_H-y, 1);
								y+= SSD1306_H/8;
							}
						}
						break;
						
					case 3:
						printf("Circles empty and filled\n\r");
						for(uint8_t x=0;x<SSD1306_W;x+=16)
							if(x<64)
								ssd1306_drawCircle(x, SSD1306_H/2, 15, 1);
							else
								ssd1306_fillCircle(x, SSD1306_H/2, 15, 1);
						break;
					
					case 4:
						printf("Unscaled Text\n\r");
						ssd1306_drawstr(0,0, "This is a test", 1);
						ssd1306_drawstr(0,8, "of the emergency", 1);
						ssd1306_drawstr(0,16,"broadcasting", 1);
						ssd1306_drawstr(0,24,"system.",1);
						if(SSD1306_H>32)
						{
							ssd1306_drawstr(0,32, "Lorem ipsum", 1);
							ssd1306_drawstr(0,40, "dolor sit amet,", 1);
							ssd1306_drawstr(0,48,"consectetur", 1);
							ssd1306_drawstr(0,56,"adipiscing",1);
						}
						ssd1306_xorrect(SSD1306_W/2, 0, SSD1306_W/2, SSD1306_W);
						break;
						
					case 5:
						printf("Scaled Text 1, 2\n\r");
						ssd1306_drawstr_sz(0,0, "sz 8x8", 1, fontsize_8x8);
						ssd1306_drawstr_sz(0,16, "16x16", 1, fontsize_16x16);
						break;
					
					case 6:
						printf("Scaled Text 4\n\r");
						ssd1306_drawstr_sz(0,0, "32x32", 1, fontsize_32x32);
						break;
					
					
					case 7:
						printf("Scaled Text 8\n\r");
						ssd1306_drawstr_sz(0,0, "64", 1, fontsize_64x64);
						break;
					case 8:
						printf("packbits-compressed image\n\r");
						unpack_image(bomb_i_heatshrunk, bomb_i_heatshrunk_len, 32, 32, 16, 0);
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
