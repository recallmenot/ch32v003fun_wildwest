CC := riscv64-unknown-elf-gcc
CFLAGS += -march=rv32ec -mabi=ilp32e -Os -flto -ffunction-sections

PROJECT = libheatshrink_static
OPTIMIZE = -O3
WARN = -Wall -Wextra -pedantic #-Werror
WARN += -Wmissing-prototypes
WARN += -Wstrict-prototypes
WARN += -Wmissing-declarations

CFLAGS += -std=c99 -g ${WARN} ${OPTIMIZE}


OBJS = heatshrink/heatshrink_encoder.o heatshrink/heatshrink_decoder.o
STATIC_OBJS = $(OBJS:.o=.os)
CFLAGS_STATIC = ${CFLAGS} -DHEATSHRINK_DYNAMIC_ALLOC=0

heatshrink/%.os: heatshrink/%.c Makefile heatshrink/*.h
	${CC} -c -o $@ $< ${CFLAGS_STATIC}

libheatshrink_static.a: ${STATIC_OBJS}
	ar -rcs $@ $^

all: libheatshrink_static.a

heatshrink/heatshrink_encoder.os heatshrink/heatshrink_decoder.os: Makefile heatshrink/*.h

clean:
	$(RM) heatshrink/*.os heatshrink/libheatshrink_static.a
