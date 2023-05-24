#ifndef DECOMPRESS_IMAGE_H
#define DECOMPRESS_IMAGE_H

#include <string.h>
#include <stdint.h>

#include "compression.h"

#include "../../examples/i2c_oled/ssd1306.h"



void unpack_image(uint8_t x, uint8_t y, const unsigned char* input, uint16_t size_of_input, uint8_t width, uint8_t height, uint8_t color_mode) {
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
	#if LOGimage == 1
	printf("unpack return %u\n\r", unpack_error);
	printf("unpack size IN %u OUT %u\n\r", size_of_input, output_max_size);
	#endif
	ssd1306_drawImage(x, y, output, width, height, color_mode);
	#if MALLOC_OVERRIDE > 0
	free(output);
	#endif
}



void unpack_image_number(uint8_t x, uint8_t y, const unsigned char* input, uint16_t size_of_input, uint8_t width, uint8_t height, uint8_t image_number, uint8_t color_mode) {
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
	#if LOGimage == 1
	printf("unpack return %u\n\r", unpack_error);
	printf("unpack size IN %u OUT %u\n\r", size_of_input, output_max_size);
	#endif
	ssd1306_drawImage(x, y, output, width, height, color_mode);
	#if MALLOC_OVERRIDE > 0
	free(output);
	#endif
}

#endif // DECOMPRESS_IMAGE_H
