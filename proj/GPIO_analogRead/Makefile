all : flash

TARGET:=GPIO_analogRead

CFLAGS+=-DTINYVECTOR
CFLAGS+=-DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

