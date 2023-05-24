# I2C OLED image decompression demo

Flash of ch32v003 is limited to 16k, images get large really quick, especially if you want to store lots of them.  

This code demonstrates live decompression of image data and displaying it on the SSD1306, library by emeb.  

## architecture

### compression.h

compression.h implements compression- and decompression using the following methods:
 * packbits (run length encoding, smaller code footprint, smaller compression ratio)
 * heatshrink (dictionary encoding for systems with very limited RAM)

You can even decompress only a part of the compressed data using the `_window` functions.  
This is useful when you store lots of compressed data like animation frames or sprites and don't want to decompress the entire thing into RAM but are only interested in the one frame you want to display.  

Nothing in here is specific to working with images.  

### decompress_image.h

These functions decompresses the data and write it to the display buffer.  
The drawImage function has already been integrated into emebs ssd1306 library.  



## building

First, install packbits and heatshrink in this dir
```
git clone https://github.com/skirridsystems/packbits.git
git clone https://github.com/atomicobject/heatshrink.git
```

The #defines of `COMP_PACKBITS` and `COMP_HEATSHRINK` allow to select the compression method to use.  
Build and upload with `make` as usual.  

### resources

To generate the files, take a look at [6zip micro compressor](https://github.com/recallmenot/6zip_micro_compressor), it's quite simple.

## a balance of trade-offs

Which algorithm is better depends on the task at hand.  
Compression algorithms take up code space but allow for smaller stored size.  
RAM must be sufficiently available.  
The compression ratio will depend on the data being compressed.  

In this case:  
`rocket` compressed well with heatshrink but actually increased in size with packbits.  
Using packbits, each frame took 7ms to decompress and draw on average, with heatshrink it was 15ms.  
`bomb` compressed well well with both algorithms.  

You'll have to test and decide for your specific usecase.
