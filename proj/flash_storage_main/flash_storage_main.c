// comment / uncomment to toggle compile-time address calculations
#define CT_ADDR
// enable (1) / disable (0) everything serial to observe actual flash usage of the flash library
#define DBG_LVL 0



#if DBG_LVL == 1
#define LOG(X) printf(X)
#define LOG32(X) printf("%s: %lu\r\n", #X, X)
#define LOG32char(X) printf("%s: %lu\r\n", #X, (uint32_t)(uintptr_t)X)
#define LOG16(X) printf("%s: %u\r\n", #X, X)
#else
#define LOG(X)			// will get replaced to empty line
#define LOG32(X)
#define LOG32char(X)
#define LOG16(X)
#endif



// Could be defined here, or in the processor defines.
#define SYSTEM_CORE_CLOCK 48000000
#define APB_CLOCK SYSTEM_CORE_CLOCK

#include "../../ch32v003fun/ch32v003fun.h"
#include <stdio.h>

#define FLASH_ENABLE_MAIN
#if !defined(CT_ADDR)
#define FLASH_ENABLE_HELPER_ADDR
#endif
#include "../../lib/ch32v003_flash.h"



uint16_t count;


#if defined (CT_ADDR)
#define NONVOLATILE_START_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(0)
#define NONVOLATILE_VAR_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(10)
#define NONVOLATILE_END_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(64)
#else
uint32_t NONVOLATILE_START_ADDR;
uint32_t NONVOLATILE_VAR_ADDR;
uint32_t NONVOLATILE_END_ADDR;
#endif



int main()
{
	SystemInit48HSI();
	#if DBG_LVL == 1
		SetupUART( UART_BRR );
	#endif

	Delay_Ms(5000);

	LOG("\r\n\r\n\r\nnonvolatile storage testing\r\n");
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	flash_set_latency();
	#if !defined (CT_ADDR)
	NONVOLATILE_START_ADDR = flash_calcualte_nonvolatile_addr(0);
	NONVOLATILE_VAR_ADDR = flash_calcualte_nonvolatile_addr(10);
	NONVOLATILE_END_ADDR = flash_calcualte_nonvolatile_addr(64);
	#endif
	
	#if defined (CT_ADDR)
	LOG32char(_reserved_nv_start);
	LOG32char(_reserved_nv_end);
	#endif
	LOG32(NONVOLATILE_START_ADDR);
	LOG32(NONVOLATILE_VAR_ADDR);
	LOG32(NONVOLATILE_END_ADDR);

	uint8_t write_counter = 0;
	LOG("\r\n");
	while(write_counter < 4)
	{
		GPIOD->BSHR = (1<<(16+4)); // LED on
		count = flash_get_16(NONVOLATILE_VAR_ADDR);
		LOG16(count);
		Delay_Ms(100);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(2400);

		count--;
		flash_unlock();
		LOG("memory unlocked\r\n");
		flash_fastp_unlock();
		LOG("fast programming unlocked\r\n");
		LOG("erasing 64b page\r\n");
		flash_erase_64b(NONVOLATILE_START_ADDR);
		LOG("memory erased\r\n");
		flash_fastp_lock();
		LOG("fast programming locked\r\n");
		LOG("programming 2 bytes\r\n");
		flash_program_16(NONVOLATILE_VAR_ADDR, count);
		LOG("memory written\r\n");
		flash_lock();
		LOG("memory locked\r\n");
		write_counter++;
		LOG("\r\n");
	}
	LOG("\r\nENOUGH WRITES FOR TODAY\r\n\r\n");
	while(1) {
		GPIOD->BSHR = (1<<(16+4)); // LED on
		Delay_Ms(1000);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(1000);
	}
}
