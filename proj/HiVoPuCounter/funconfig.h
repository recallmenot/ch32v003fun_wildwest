#ifndef _FUNCONFIG_H
#define _FUNCONFIG_H

#define CH32V003           1
#define FUNCONF_USE_DEBUGPRINTF 0
#define FUNCONF_USE_UARTPRINTF  1
#define FUNCONF_UART_PRINTF_BAUD 115200
#define FUNCONF_USE_PLL 1               // Use built-in 2x PLL 
#define FUNCONF_USE_HSI 1               // Use HSI Internal Oscillator
#define FUNCONF_SYSTEM_CORE_CLOCK  48000000  // Computed Clock in Hz.

#endif

/*

#define FUNCONF_USE_PLL 1               // Use built-in 2x PLL 
#define FUNCONF_USE_HSI 1               // Use HSI Internal Oscillator
#define FUNCONF_USE_HSE 0               // Use External Oscillator
#define FUNCONF_HSITRIM 0x10            // Use factory calibration on HSI Trim.
#define FUNCONF_SYSTEM_CORE_CLOCK  48000000  // Computed Clock in Hz.
#define FUNCONF_HSE_BYPASS 0            // Use HSE Bypass feature (for oscillator input)
#define FUNCONF_USE_CLK_SEC	1			// Use clock security system, enabled by default
#define FUNCONF_USE_DEBUGPRINTF 1
#define FUNCONF_USE_UARTPRINTF  0
#define FUNCONF_NULL_PRINTF 0           // Have printf but direct it "nowhere"
#define FUNCONF_SYSTICK_USE_HCLK 0      // Should systick be at 48 MHz or 6MHz?
#define FUNCONF_TINYVECTOR 0            // If enabled, Does not allow normal interrupts.
#define FUNCONF_UART_PRINTF_BAUD 115200 // Only used if FUNCONF_USE_UARTPRINTF is set.
#define FUNCONF_DEBUGPRINTF_TIMEOUT 160000 // Arbitrary time units

*/
