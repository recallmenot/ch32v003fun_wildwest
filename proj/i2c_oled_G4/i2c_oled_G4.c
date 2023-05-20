/*
 * Example for using I2C with 128x32 graphic OLED
 * 03-29-2023 E. Brombaugh
 */

// Could be defined here, or in the processor defines.
#include <stdint.h>
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

// what type of OLED - uncomment just one
//#define SSD1306_64X32
#define SSD1306_128X32
//#define SSD1306_128X64

#include "../../ch32v003fun/ch32v003fun.h"

// toggle for printf
#define DEBUG_LVL 0

#if DEBUG_LVL == 1
#include <stdio.h>
#define LOG(x) printf(x)
#define LOGu(x, y) printf(x, y)
#define LOGu2(x, y, z) printf(x, y, z)
#else
#define LOG(x)
#define LOGu(x, y)
#define LOGu2(x, y, z)
#endif


#include "ssd1306_i2c.h"
#include "ssd1306.h"


#undef __cplusplus			// prevent gcc from seing the C++ wrapper in .h
#define __LINUX__			// circumvent #include of arduino.h
#define __AVR__				// reduce decode buffer size for the tiny 2k of ram we have
#define REENTRANT			// define as empty! used only for 8051 targets
#include "TIFF_G4/src/TIFF_G4.h"
#include "TIFF_G4/src/tiffg4.c"
#undef REENTRANT
#undef __AVR__
#undef __LINUX__
TIFFIMAGE tiff;				// instance of TIFFIMAGE struct
#include "bomb-om.h"



// provide draw callback for TIFF_G4
void TIFFDraw(TIFFDRAW *pDraw) {
	uint8_t x_absolute;
	uint8_t y_absolute = pDraw->iDestY + pDraw->y;
	// abort if lines > display height
	if (y_absolute >= SSD1306_H) {
		return;
	}
	// create typed pointer to decoded line data
	uint8_t* src = pDraw->pPixels;
	
	uint8_t pixel;
	// calculate number of bytes to draw for line (rounding up)
	uint8_t bytes_to_draw = pDraw->iWidth / 8;
#if DEBUG_LVL == 1
	for (uint8_t byte = 0; byte < bytes_to_draw; byte++) {
		LOGu2("b%u %02x ", byte, src[byte]);
	}
	LOG("\n\r");
#endif
	for (uint8_t byte = 0; byte < bytes_to_draw; byte++) {
		for (pixel = 0; pixel < 8; pixel++) {
			x_absolute = pDraw->iDestX + 8 * (bytes_to_draw - byte) + pixel;
			if (x_absolute >= SSD1306_W) {
				break;
			}
			if(src[byte] & (1 << pixel)) {
				ssd1306_drawPixel(x_absolute, y_absolute, 1);
			}
			#if defined (TIFFG4_opaque_black)
			else {
				ssd1306_drawPixel(x_absolute, y_absolute, 0);
			}
			#endif
		}
		//pDraw->iDestX += 8;
	}
}

int main()
{
	// 48MHz internal clock
	SystemInit48HSI();

	// start serial @ default 115200bps

#if DEBUG_LVL == 1
#ifdef STDOUT_UART
	SetupUART( UART_BRR );
	Delay_Ms( 100 );
#else
	SetupDebugPrintf();
#endif
#endif
	Delay_Ms( 1000 );
	LOG("\r\r\n\ni2c_oled example\n\r");

	// init i2c and oled
	Delay_Ms( 100 );	// give OLED some more time
	LOG("initializing i2c oled...");
	if(!ssd1306_i2c_init())
	{
		ssd1306_init();
		LOG("done.\n\r");
		
		// clear buffer for next mode
		ssd1306_setbuf(0);
		TIFF_openRAW(&tiff, 32, 32, BITDIR_MSB_FIRST, (uint8_t*)bomb_om, sizeof(bomb_om), TIFFDraw);
		//TIFF_openTIFFRAM(&tiff, (uint8_t*)bomb_om, sizeof(bomb_om), TIFFDraw);
		LOGu("Error after openRAW %u\n\r", tiff.iError);
		TIFF_setDrawParameters(&tiff, 1.0f, TIFF_PIXEL_1BPP, 0, 0, 32, 32, NULL);
		uint8_t tiff_x = TIFF_getWidth(&tiff);
		uint8_t tiff_y = TIFF_getHeight(&tiff);
		LOGu2("Image opened, size = %d x %d\n\r", tiff_x, tiff_y);
		/*
		TIFF_decodeIncBegin(&tiff, tiff_x, tiff_y, BITDIR_MSB_FIRST, TIFFDraw);
		printf("Error after decodeIncBegin %u\n\r", tiff.iError);
		TIFF_decodeInc(&tiff, 1);
		printf("Error after decodeInc %u\n\r", tiff.iError);
		*/
		TIFF_decode(&tiff);
		LOGu("Error after decode %u\n\r", tiff.iError);
		TIFF_close(&tiff);
		LOGu("Error after close %u\n\r", tiff.iError);
		ssd1306_refresh();
		while(1) {
			Delay_Ms(2000);
		}
	}
	else
		printf("failed.\n\r");
	
	printf("Stuck here forever...\n\r");
	while(1);
}
