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
#define COMP_HEATSHRINK 0
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
	uint16_t output_max_size = (width / 8) * height;
	#if MALLOC_OVERRIDE > 0
	uint8_t* output = malloc(output_max_size);
	#else
	uint8_t output[output_max_size];
	#endif

	#if COMP_PACKBITS == 1
	uint16_t unpack_error = decompress_packbits(input, size_of_input, output, &output_max_size);
	#elif COMP_HEATSHRINK == 1
	uint16_t unpack_error = decompress_heatshrink(input, size_of_input, output, &output_max_size);
	#endif
	#ifdef LOGimage
	printf("unpackbits return %u\n\r", unpack_error);
	printf("unpackbits sizes IN %u OUT %u\n\r", size_of_input, output_max_size);
	#endif
	draw_image(output, width, height, x, y);
	#if MALLOC_OVERRIDE > 0
	free(output);
	#endif
}



void unpack_image_number(uint8_t* input, uint16_t size_of_input, uint8_t width, uint8_t height, uint8_t image_number, uint8_t x, uint8_t y) {
	uint16_t output_max_size = (width / 8) * height;
	#if MALLOC_OVERRIDE > 0
	uint8_t* output = malloc(output_max_size);
	#else
	uint8_t output[output_max_size];
	uint16_t start_byte_offset = output_max_size * image_number;
	#endif
	#if COMP_PACKBITS == 1
	uint16_t unpack_error = decompress_packbits_window(input, size_of_input, output, start_byte_offset, &output_max_size);
	#elif COMP_HEATSHRINK == 1
	uint16_t unpack_error = decompress_heatshrink_window(input, size_of_input, output, start_byte_offset, &output_max_size);
	#endif
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
						unpack_image(bomb_i_packed, bomb_i_packed_len, 32, 32, 16, 0);
						#elif COMP_HEATSHRINK == 1
						printf("heatshrink\n\r");
						unpack_image(bomb_i_heatshrunk, bomb_i_heatshrunk_len, 32, 32, 16, 0);
						#endif
						break;
					case 2:
						printf("launch a compressed rocket\n\r");
						for (uint8_t loop = 0; loop < 5; loop++) {
							for (uint8_t image = 0;image < 26; image++) {
								ssd1306_setbuf(0);
								#if COMP_PACKBITS == 1
								unpack_image_number(rocket_i_packed, rocket_i_packed_len, 32, 32, image, 0, 0);
								#elif COMP_HEATSHRINK == 1
								unpack_image_number(img_rocket_i_heatshrunk, img_rocket_i_heatshrunk_len, 32, 32, image, 0, 0);
								#endif
								ssd1306_refresh();
								Delay_Ms( 30 );
							}
						}
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
