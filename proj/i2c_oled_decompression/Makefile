all : flash

TARGET:=i2c_oled_decompression

CFLAGS+=-DSTDOUT_UART -I heatshrink

libheatshrink_static.a:
	$(MAKE) -f heatshrink_static.mk

ADDITIONAL_C_FILES+=packbits/packbits.c libheatshrink_static.a

include ../../ch32v003fun/ch32v003fun.mk

LDFLAGS += -Wl,--start-group -lc -Wl,--end-group libheatshrink_static.a

heatshrink_clean:
	$(MAKE) -f heatshrink_static.mk clean

flash : cv_flash
clean : cv_clean heatshrink_clean

$(TARGET).elf : $(SYSTEM_C) $(TARGET).c $(ADDITIONAL_C_FILES) libheatshrink_static.a
	$(PREFIX)-gcc -o $@ $^ $(CFLAGS) $(LDFLAGS)
