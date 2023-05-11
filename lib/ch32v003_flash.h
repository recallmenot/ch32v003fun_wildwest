//######## necessities

// include guards
#ifndef CH32V003_FLASH_H
#define CH32V003_FLASH_H

// includes
#include<stdint.h>								//uintN_t support
#include"../ch32v003fun/ch32v003fun.h"



/*######## library description
MAIN GOAL
ability to
	store settings in flash (non-volatile)
	retrieve settings from flash after power cycle

IMPLEMENTATION:
write to and read from
	main flash
		reserved page(s) at the end (64 bytes)
	option bytes data1 and data0
		for some applications 16 bits are enough
*/



/*######## library usage and configuration

SELECTION:

depending on your storage requirements, you have 3 options:
	- option bytes: 16 bits / 2 bytes 
	- main flash: n 64 byte pages (for most uses)
	- main flash: n 1024 byte large pages
The last two options are dictated by the size of the erase commands, which sets all bits to 1, wiping either 64 bytes or 1024 bytes at once.



USAGE:

in the .c file:
before #including this library, you will need to do a define:
#define SYSTEM_CORE_CLOCK = ......
Now you can #include this library.

During your boot phase, call flash_set_latency() once.

Read-operations (the getter functions) can be performed at any time and do not require unlocking.

To "alter" means to erase / program / write.



OPTION BYTES

The functions with "OB" in the name concern the option bytes.

To alter option bytes data1 and data0:
1. unlock flash
2. unlock OB
3. write. this will internally erase them and restore all other values of the option bytes as they were before.
4. lock



MAIN FLASH: both

in your Makefile, after "include ../../ch32v003fun/ch32v003fun.mk", inject your linker file:
LDFLAGS+=-T nonvolatile.ld 
The reason for injecting it afterwards: FLASH needs to be defined beforehand by ch32v003fun.ld

For any changes to the Makefile or nonvolatile.ld to apply, running "make clean" before "make" may be necessary.



MAIN FLASH: 64 byte pages

The fast erase command wipes one page (64 bytes) at once.

To store variables in main flash (nonvolatile) you need to reserve space first.
nonvolatile storage needs to be
	aligned to 64 bytes
	a whole multiple of 64 bytes

in nonvolatile.ld, we define the .nonvolatile section:
SECTIONS
{
	.nonvolatile : {
		. = ALIGN(64);
		_reserved_nv_start = .;
		KEEP(*(.nonvolatile));
		. += (64 * n);
		_reserved_nv_end = .;
	} >FLASH AT>FLASH
}
"KEEP" tells gcc to really allocate the space, despite calling gcc with -Wl,--gc-sections, which will remove any flash allocations it thinks are "unused"

then, still in the .ld we pass the start and end addresses to the .c file
PROVIDE(_reserved_nv_start = _reserved_nv_start);
PROVIDE(_reserved_nv_end = _reserved_nv_end);

to alter main flash:
1. unlock flash
2. unlock fast programming mode for 64 page erase
3. erase 64 byte page(s) to be written to (sadly we can't erase any smaller)
4. lock fast programming mode
5. program all desired values to the page(s)
6. lock



MAIN FLASH: 1024 byte large pages

The standard erase command wipes 16 pages at once (1K = 2^10 bytes = 1024 bytes).

To store variables in main flash (nonvolatile) you need to reserve space first.
nonvolatile storage needs to be
	aligned to 1K
	a whole multiple of 1K

in nonvolatile.ld, we define the .nonvolatile section:
SECTIONS
{
	.nonvolatile : {
		. = ALIGN(1K);
		_reserved_nv_start = .;
		KEEP(*(.nonvolatile));
		. += (1K * n);
		_reserved_nv_end = .;
	} >FLASH AT>FLASH
}
"KEEP" tells gcc to really allocate the space, despite calling gcc with -Wl,--gc-sections, which will remove any flash allocations it thinks are "unused"

then, still in the .ld we pass the start and end addresses to the .c file
PROVIDE(_reserved_nv_start = _reserved_nv_start);
PROVIDE(_reserved_nv_end = _reserved_nv_end);

to alter main flash:
1. unlock flash
3. erase 1024 byte large page(s) to be written to
4. lock fast programming mode
5. program all desired values to the page(s)
6. lock



TECHNICAL NOTES:
HARDWARE:
CH32V003 has 16K =
	16 * 2^10 bytes
	16384 bytes
	0x08003FFF-0x08000000+1
	256 pages * 64 bytes
erase	= turn all bits into 1
program = turn the desired bits into 0, always 2 bytes at a time
write	= erase + program

Integrated flash has limited write cycles (10K as per datasheet) and will thus wear out quickly if you're not very careful!!
For frequent writes use external flash chips, they are far more robust!

main flash:
trying to write to addresses outside the main flash (offsets larger than what you reserved) will likely make your micro controller freeze or lead to garbled serial output, so be sure that your code doesn't attempt.

option bytes (inside user-selected words):
The data is stored as IIIIIIII DDDDDDDD, where D is data (byte0) and I is the inverse of data (byte 1).
The user is supposed to be able to compare and discover errors, but it doesn't allow for error recovery.
To write a byte (8 bits), the write needs to be 16 bits (uint16_t), the upper 16 bits (I) will automagically be replaced with the inverted bit pattern of D.
To read them (and write the data1 and data0 bytes), feel free to use the provided functions, they abstract this away.
You may also OB->Data1 and discard the upper 8 (inverted) bits by >>8 yourself if you require raw speed.



ADDRESS CALCULATIONS:

While the flash region may be aliased to 0x00, we want to use its real address, starting at 0x08000000.
For storing variables in main flash the address can be calculated like this:
address of byte nonvolatile[n] = FLASH_BASE + N_BYTES + [n];
where n is the offset from the start of your reserved nonvolatile storage
example:
nonvolatile[15] = 0x08000000 + _reserved_nv_start + 15

The flash_calcualte_nonvolatile_addr(n) function and FLASH_PRECALCULATE_NONVOLATILE_ADDR(n) preprocessor macro do exactly that.
We recommend using the macro to #define the addresses of the variables you want to store at compile time.
This way, that math stays off the MCU.
*/


//######## function overview (declarations): use these!

// calculate address at runtime, you can use compile-time addresses instead!
uint32_t flash_calcualte_nonvolatile_addr(uint16_t byte_number);

// set the flash controller latency in accordance with the SYSTEM_CORE_CLOCK speed
static inline void flash_set_latency();

// unlock flash altering
static inline void flash_unlock();
// unlock fast programming mode for 64byte erase
static inline void flash_fastp_unlock();
// unlock option bytes altering, additionally
static inline void flash_OB_unlock();
// lock flash when you're done
static inline void flash_lock();
// lock fast programming mode
static inline void flash_fastp_lock();

// erase a page (sorry, smaller erases impossible on CH32V003!)
// x -> 1
static inline void flash_erase_1K(uint32_t start_addr);
static inline void flash_erase_64b(uint32_t start_addr);

// program 2 bytes at once
// 1 -> 0
static inline void flash_program_16(uint32_t addr, uint16_t data);
static inline uint16_t flash_get_16(uint32_t addr);
static inline void flash_program_2x8(uint32_t addr, uint8_t byte1, uint8_t byte0);
static inline uint8_t flash_get_8(uint32_t addr);
static inline void flash_program_float(uint32_t addr, float value);
static inline float flash_get_float(uint32_t addr);

// write data to option bytes, keeping other contents intact
// 		= backup + erase + write (restoring backup)
static inline void flash_OB_write_data_16(uint16_t data);
// read data of option bytes
static inline uint8_t flash_OB_get_DATA1();
static inline uint8_t flash_OB_get_DATA0();
static inline uint16_t flash_OB_get_DATA_16();

static inline void flash_OB_write_data_2x8(uint8_t data1, uint8_t data0);

// read option bytes
static inline uint8_t flash_OB_get_USER();
static inline uint8_t flash_OB_get_RDPR();
static inline uint8_t flash_OB_get_WRPR1();
static inline uint8_t flash_OB_get_WRPR0();



//######## internal function declarations

// get opterational states of flash
static inline uint8_t flash_is_busy();
static inline uint8_t flash_is_done();
static inline uint8_t flash_is_ERR_WRPRT();

// clear flash operation done (EOP) bit
static inline void flash_is_done_clear();

// wait for flash
static inline void flash_wait_until_not_busy();
//static inline void flash_wait_until_done();

// test that first 8 bits are the inverse of the last 8 bits
static inline uint8_t flash_dechecksum(uint16_t input);

// erase option byte
static inline void flash_OB_erase();



//######## internal variables
// import from .ld, halal by https://sourceware.org/binutils/docs/ld/Source-Code-Reference.html
extern char _reserved_nv_start[]; 
extern char _reserved_nv_end[]; 

union float_2xuint16t {
	float f;
	uint16_t u16[2];
};

//######## preprocessor macros

#define FLASH_VOLATILE_CAPACITY (_reserved_end - _reserved_nv_start)

// use this to define main flash nonvolatile addresses at compile time!
#define FLASH_PRECALCULATE_NONVOLATILE_ADDR(n) (FLASH_BASE + (uint16_t)(uintptr_t)(_reserved_nv_start) + n)



//######## preprocessor #define requirements


#ifndef SYSTEM_CORE_CLOCK
#error "SYSTEM_CORE_CLOCK is not defined. Please define it in your .c before you #include ch32v003_flash.h."
#endif



//######## small function definitions, static inline

uint32_t flash_calcualte_nonvolatile_addr(uint16_t byte_number) {
	//if (byte_number < FLASH_VOLATILE_CAPACITY) {}
	return (FLASH_BASE + ((uint16_t)(uintptr_t)_reserved_nv_start + byte_number));
}

static inline void flash_set_latency() {
	#if SYSTEM_CORE_CLOCK <= 24000000
		FLASH->ACTLR = FLASH_Latency_0;
	#else
		FLASH->ACTLR = FLASH_Latency_1;
	#endif
}

static inline void flash_unlock() {
	FLASH->KEYR = FLASH_KEY1;
	FLASH->KEYR = FLASH_KEY2;
}

static inline void flash_fastp_unlock() {
	FLASH->MODEKEYR = FLASH_KEY1;
	FLASH->MODEKEYR = FLASH_KEY2;
}
static inline void flash_OB_unlock() {
	FLASH->OBKEYR = FLASH_KEY1;
	FLASH->OBKEYR = FLASH_KEY2;
}
static inline void flash_lock() {
	FLASH->CTLR |= FLASH_CTLR_LOCK;
}
static inline void flash_fastp_lock() {
	FLASH->CTLR |= CR_FLOCK_Set;
}

// x -> 1
static inline void flash_erase_1K(uint32_t start_addr) {
	if(FLASH->CTLR & FLASH_CTLR_LOCK) {
		return;
	}
	flash_wait_until_not_busy();
	FLASH->CTLR |= CR_PER_Set;
	FLASH->ADDR = start_addr; 
	FLASH->CTLR |= CR_STRT_Set;
	flash_wait_until_not_busy();
	FLASH->CTLR &= CR_PER_Reset;
}

static inline void flash_erase_64b(uint32_t start_addr) {
	if(FLASH->CTLR & (FLASH_CTLR_LOCK | FLASH_CTLR_FLOCK)) {
		return;
	}
	flash_wait_until_not_busy();
	FLASH->CTLR |= CR_PAGE_ER;
	FLASH->ADDR = start_addr; 
	FLASH->CTLR |= CR_STRT_Set;
	flash_wait_until_not_busy();
	FLASH->CTLR &= ~CR_PAGE_ER;
}

// 1 -> 0
static inline void flash_program_16(uint32_t addr, uint16_t data) {
	if(FLASH->CTLR & FLASH_CTLR_LOCK) {
		return;
	}
	flash_wait_until_not_busy();
	//FLASH->ADDR = addr;
	//FLASH->OBR = 0;
	//FLASH->OBR |= data << 10;
	FLASH->CTLR |= CR_PG_Set;
	*(uint16_t*)(uintptr_t)addr = data;
	flash_wait_until_not_busy();
	FLASH->CTLR &= CR_PG_Reset;
}
static inline uint16_t flash_get_16(uint32_t addr) {
	return *(uint16_t*)(uintptr_t)addr;
}

static inline void flash_program_2x8(uint32_t addr, uint8_t byte1, uint8_t byte0) {
	flash_program_16(addr, (byte1<<8)+byte0);
}
static inline uint8_t flash_get_8(uint32_t addr) {
	return *(uint8_t*)(uintptr_t)addr;
}

static inline void flash_program_float(uint32_t addr, float value) {
	union float_2xuint16t conv;
	conv.f = value;
	flash_program_16(addr, conv.u16[0]);
	flash_program_16(addr, conv.u16[1]);
}
static inline float flash_get_float(uint32_t addr) {
	union float_2xuint16t conv;
	conv.u16[0] = ((uint16_t*)(uintptr_t)addr)[0];
	conv.u16[1] = ((uint16_t*)(uintptr_t)addr)[1];
	return conv.f;
}

// 1 -> 0
// is inlining still ok?
//		big
//		but won't get called often / from many places
//	else move to implementation block and remove static inline
static inline void flash_OB_write_data_16(uint16_t data) {
	flash_wait_until_not_busy();
	//backup
	uint16_t tmp_user = OB->USER; 
	uint16_t tmp_rdpr = OB->RDPR; 
	uint16_t tmp_wrpr1 = OB->WRPR1; 
	uint16_t tmp_wrpr0 = OB->WRPR0; 
	uint16_t tmp_data1 = (data>>8) & 0b11111111;
	uint16_t tmp_data0 = data & 0b11111111;

	//erase
	flash_OB_erase();

	//enable programming
	FLASH->CTLR |= CR_OPTPG_Set;

	//restore backup
	OB->USER = tmp_user; 
	flash_wait_until_not_busy();
	OB->RDPR = tmp_rdpr;
	flash_wait_until_not_busy();
	OB->WRPR0 = tmp_wrpr0;
	flash_wait_until_not_busy();
	OB->WRPR1 = tmp_wrpr1;
	flash_wait_until_not_busy();

	// write data
	OB->Data1 = tmp_data1;				// data high byte
	flash_wait_until_not_busy();
	OB->Data0 = tmp_data0;				// data low byte
	flash_wait_until_not_busy();
	
	//disable programming
	FLASH->CTLR &= CR_OPTPG_Reset;
}

static inline uint8_t flash_OB_get_DATA1() {
	return flash_dechecksum(OB->Data1);
}
static inline uint8_t flash_OB_get_DATA0() {
	return flash_dechecksum(OB->Data0);
}
static inline uint16_t flash_OB_get_DATA_16() {
	return (flash_OB_get_DATA1()<<8)+flash_OB_get_DATA0();
}

static inline void flash_OB_write_data_2x8(uint8_t data1, uint8_t data0) {
	flash_OB_write_data_16((data1<<8)+data0);
}

static inline uint8_t flash_OB_get_USER() {
	return flash_dechecksum(OB->USER);
}
static inline uint8_t flash_OB_get_RDPR() {
	return flash_dechecksum(OB->RDPR);
}
static inline uint8_t flash_OB_get_WRPR1() {
	return flash_dechecksum(OB->WRPR1);
}
static inline uint8_t flash_OB_get_WRPR0() {
	return flash_dechecksum(OB->WRPR0);
}



//######## small internal function definitions, static inline
static inline uint8_t flash_is_busy() {
	return ((FLASH->STATR & FLASH_STATR_BSY) == FLASH_STATR_BSY);
}
static inline uint8_t flash_is_done() {
	return ((FLASH->STATR & FLASH_STATR_EOP) == FLASH_STATR_EOP);
}
static inline uint8_t flash_is_ERR_WRPRT() {
	return ((FLASH->STATR & FLASH_STATR_WRPRTERR) == FLASH_STATR_WRPRTERR);
}
static inline void flash_is_done_clear() {
	FLASH->STATR |= FLASH_STATR_EOP;
}

static inline void flash_wait_until_not_busy() {
	while(flash_is_busy()) {}
}

/*
static inline void flash_wait_until_done() {
	while(flash_is_busy() || !flash_is_done()) {}
	flash_is_done_clear();
}
*/

static inline uint8_t flash_dechecksum(uint16_t input) {
	uint8_t noninverted = input & 0b11111111;
	uint8_t deinverted = ~(input>>8);
	if (deinverted == noninverted) {
		return noninverted;
	}
	else {
		return 0;
	}
}

// x -> 1
static inline void flash_OB_erase() {
	FLASH->CTLR |= CR_OPTER_Set;
	FLASH->CTLR |= CR_STRT_Set;
	flash_wait_until_not_busy();
	FLASH->CTLR &= CR_OPTER_Reset;
}


//######## implementation block
//#define CH32V003_FLASH_IMPLEMENTATION //enable so LSP can give you text colors while working on the implementation block, disable for normal use of the library
#if defined(CH32V003_FLASH_IMPLEMENTATION)



#endif // CH32V003_FLASH_IMPLEMENTATION
#endif // CH32V003_FLASH_H
