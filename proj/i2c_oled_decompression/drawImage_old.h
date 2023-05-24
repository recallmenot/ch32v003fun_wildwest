// below are the functions previously used for drawing the image
// this function has been superceeded by the integrated ssd1306_drawImage function eventually pulled into emebs ssd1306 library

// v1, relying on drawPixel for horizontal->vertical conversion
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

// v2, direct buffer manipulation, predecessor of the function eventually pulled into emebs ssd1306 library
void draw_image(uint8_t* input, uint8_t width, uint8_t height, uint8_t x, uint8_t y, uint8_t color_mode) {
	uint8_t x_absolute;
	uint8_t y_absolute;
	uint8_t pixel;
	uint8_t bytes_to_draw = width / 8;
	uint16_t buffer_addr;

	for (uint8_t line = 0; line < height; line++) {
		y_absolute = y + line;
		if (y_absolute >= SSD1306_H) {
			break;
		}

		// SSD1306 is in vertical mode, yet we want to draw horizontally, which necessitates assembling the output bytes from the input data
		// bitmask for current pixel in vertical (output) byte
		uint8_t v_mask = 1 << (y_absolute & 7);

		for (uint8_t byte = 0; byte < bytes_to_draw; byte++) {
			uint8_t input_byte = input[byte + line * bytes_to_draw];

			for (pixel = 0; pixel < 8; pixel++) {
				x_absolute = x + 8 * (bytes_to_draw - byte) + pixel;
				if (x_absolute >= SSD1306_W) {
					break;
				}
				// looking at the horizontal display, we're drawing bytes bottom to top, not left to right, hence y / 8
				buffer_addr = x_absolute + SSD1306_W * (y_absolute / 8);
				// state of current pixel
				uint8_t input_pixel = input_byte & (1 << pixel);

				switch (color_mode) {
					case 0:
						// write pixels as they are
						ssd1306_buffer[buffer_addr] = (ssd1306_buffer[buffer_addr] & ~v_mask) | (input_pixel ? v_mask : 0);
						break;
					case 1:
						// write pixels after inversion
						ssd1306_buffer[buffer_addr] = (ssd1306_buffer[buffer_addr] & ~v_mask) | (!input_pixel ? v_mask : 0);
						break;
					case 2:
						// 0 clears pixel
						ssd1306_buffer[buffer_addr] &= input_pixel ? 0xFF : ~v_mask;
						break;
					case 3:
						// 1 sets pixel
						ssd1306_buffer[buffer_addr] |= input_pixel ? v_mask : 0;
						break;
					case 4:
						// 0 sets pixel
						ssd1306_buffer[buffer_addr] |= !input_pixel ? v_mask : 0;
						break;
					case 5:
						// 1 clears pixel
						ssd1306_buffer[buffer_addr] &= input_pixel ? ~v_mask : 0xFF;
						break;
				}
			}
			#if LOGimage == 1
			printf("%02x ", input_byte);
			#endif
		}
		#if LOGimage == 1
		printf("\n\r");
		#endif
	}
}
