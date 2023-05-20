all : flash

TARGET:=i2c_oled_G4
#ADDITIONAL_C_FILES=TIFF_G4/src/TIFF_G4.h TIFF_G4/src/tiffg4.c 

CFLAGS+=-DSTDOUT_UART

include ../../ch32v003fun/ch32v003fun.mk

flash : cv_flash
clean : cv_clean

