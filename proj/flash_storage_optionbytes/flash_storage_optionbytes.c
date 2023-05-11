// disable (0) / enable (8) / enable (16) everything serial to observe actual flash usage of the flash library
#define DBG_BITS 8



#if DBG_BITS > 0
	#define LOG(X) printf(X)
	#define LOG32(X) printf("%s: %lu\r\n", #X, X)
	#define LOG32char(X) printf("%s: %lu\r\n", #X, (uint32_t)(uintptr_t)X)
	#define LOG16(X) printf("%s: %u\r\n", #X, X)
	#define LOGregister(X) printf("register: %s contains %s\n\r", #X, bits)
	#if DBG_BITS == 16
		#define INIT_register_str char bits[17]
		#define CNVregister(X) uint16_to_binary_string(X, bits, 16)
	#elif DBG_BITS == 8
		#define INIT_register_str char bits[9]
		#define CNVregister(X) uint8_to_binary_string(X, bits, 8)
	#endif
#else
	#define LOG(X)			// will get replaced to empty line
	#define LOG32(X)
	#define LOG32char(X)
	#define LOG16(X)
	#define LOGregister(X)
	#define CNVregister(X)
	#define INIT_register_str 
	#define CNVregister(X)
#endif



// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"

#include <stdio.h>

#define FLASH_ENABLE_OB
#define FLASH_ENABLE_HELPER_OB_GETTERS
#include "../../lib/ch32v003_flash.h"



uint16_t count;



//######### debug fn

void uint16_to_binary_string(uint16_t value, char* output, int len) {
	for (int i = 0; i < len; i++) {
			output[len - i - 1] = (value & 1) ? '1' : '0';
			value >>= 1;
	}
	output[len] = '\0';
}
void uint8_to_binary_string(uint8_t value, char* output, int len) {
	for (int i = 0; i < len; i++) {
			output[len - i - 1] = (value & 1) ? '1' : '0';
			value >>= 1;
	}
	output[len] = '\0';
}

#if DBG_BITS == 8
void print_debug() {
	INIT_register_str;
	CNVregister(flash_OB_get_USER());
	LOGregister(OB->USER);
	CNVregister(flash_OB_get_RDPR());
	LOGregister(OB->RDPR);
	CNVregister(flash_OB_get_WRPR1());
	LOGregister(OB->WRPR1);
	CNVregister(flash_OB_get_WRPR0());
	LOGregister(OB->WRPR0);
	CNVregister(flash_OB_get_DATA1());
	LOGregister(OB->Data1);
	CNVregister(flash_OB_get_DATA0());
	LOGregister(OB->Data0);
}
#elif DBG_BITS == 16
void print_debug() {
	INIT_register_str;
	CNVregister(OB->USER);
	LOGregister(OB->USER);
	CNVregister(OB->RDPR);
	LOGregister(OB->RDPR);
	CNVregister(OB->WRPR1);
	LOGregister(OB->WRPR1);
	CNVregister(OB->WRPR0);
	LOGregister(OB->WRPR0);
	CNVregister(OB->Data1);
	LOGregister(OB->Data1);
	CNVregister(OB->Data0);
	LOGregister(OB->Data0);
}
#endif



//######### main

int main()
{
	SystemInit48HSI();
	#if DBG_BITS > 0
	SetupUART( UART_BRR );
	#endif

	Delay_Ms(5000);

	LOG("\r\nnonvolatile storage testing: option bytes\r\n");
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	flash_set_latency();

	uint8_t write_counter = 0;

	while(write_counter < 4)
	{
		GPIOD->BSHR = (1<<(16+4)); // LED on
		#if DBG_BITS > 0
		print_debug();
		#endif
		count = flash_OB_get_DATA_16();
		LOG16(count);
		Delay_Ms(100);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(2400);

		count--;
		flash_unlock();
		flash_OB_unlock();
		LOG("memory unlocked\r\n");
		flash_OB_write_data_16(count);
		LOG("memory written\r\n");
		flash_lock();
		LOG("memory locked\r\n");
		write_counter++;
	}
	LOG("\r\nENOUGH WRITES FOR TODAY\r\n\r\n");
	while(1) {
		GPIOD->BSHR = (1<<(16+4)); // LED on
		Delay_Ms(1000);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(1000);
	}
}
