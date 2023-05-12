all : flash

TARGET:=flash_storage_main
CFLAGS+=-DTINYVECTOR -DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

LDFLAGS+=-T nonvolatile.ld 

flash : cv_flash
clean : cv_clean


