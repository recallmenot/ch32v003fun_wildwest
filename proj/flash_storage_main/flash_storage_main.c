#define CT_ADDR					// comment / uncomment to toggle compile-time address calculations



// Could be defined here, or in the processor defines.
#include <stdint.h>
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
#define NONVOLATILE_END_ADDR FLASH_PRECALCULATE_NONVOLATILE_ADDR(1024)
#else
uint32_t NONVOLATILE_START_ADDR;
uint32_t NONVOLATILE_VAR_ADDR;
uint32_t NONVOLATILE_END_ADDR;
#endif



int main()
{
	SystemInit48HSI();
	SetupUART( UART_BRR );

	Delay_Ms(5000);

	printf("\r\n\r\n\r\nnonvolatile storage testing\r\n");
	// Enable GPIOs
	RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

	// GPIO D4 Push-Pull
	GPIOD->CFGLR &= ~(0xf<<(4*4));
	GPIOD->CFGLR |= (GPIO_Speed_10MHz | GPIO_CNF_OUT_PP)<<(4*4);

	flash_set_latency();
	#if !defined (CT_ADDR)
	NONVOLATILE_START_ADDR = flash_calcualte_nonvolatile_addr(0);
	NONVOLATILE_VAR_ADDR = flash_calcualte_nonvolatile_addr(10);
	NONVOLATILE_END_ADDR = flash_calcualte_nonvolatile_addr(1024);
	#endif
	
	#if defined (CT_ADDR)
	printf("_reserved_nv_start is         %lu\r\n", (uint32_t)(uintptr_t)_reserved_nv_start);
	printf("_reserved_nv_end is           %lu\r\n", (uint32_t)(uintptr_t)_reserved_nv_end);
	#endif
	printf("non-volatile start address is %lu\r\n", NONVOLATILE_START_ADDR);
	printf("non-volatile var address is   %lu\r\n", NONVOLATILE_VAR_ADDR);
	printf("non-volatile end address is   %lu\r\n", NONVOLATILE_END_ADDR);

	uint8_t write_counter = 0;

	while(write_counter < 4)
	{
		GPIOD->BSHR = (1<<(16+4)); // LED on
		count = flash_get_16(NONVOLATILE_VAR_ADDR);
		printf("   memory contained value %u\r\n", count);
		Delay_Ms(250);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(9750);

		count--;
		flash_unlock();
		printf("memory unlocked\r\n");
		printf("erasing 1K page\r\n");
		flash_erase_1K(NONVOLATILE_START_ADDR);
		printf("memory erased\r\n");
		printf("programming 2 bytes\r\n");
		flash_program_16(NONVOLATILE_VAR_ADDR, count);
		printf("memory written\r\n");
		flash_lock();
		printf("memory locked\r\n");
		write_counter++;
	}
	printf("\r\nENOUGH WRITES FOR TODAY\r\n\r\n");
	while(1) {
		GPIOD->BSHR = (1<<(16+4)); // LED on
		Delay_Ms(1000);
		GPIOD->BSHR = (1<<4); // LED off
		Delay_Ms(1000);
	}
}
