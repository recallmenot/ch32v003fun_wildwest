# I2C OLED demonstration
This example code demonstrates the possibility and impracticality of using CCITT G4 image compression on the ch32v003.  
The decompression algorithm uses around 10k of flash storage, hence impractical.  

But it's cool that it works at all.  

Maybe someone using a bigger microcontroller will find this useful, compression ratios of up to 1:20 are possible!

## building
```
git clone https://github.com/bitbank2/TIFF_G4.git
```

then build and upload with `make` as usual

## generating compressed images

You'll have to use imagemagick to generate the G4 compressed image: `convert input.pbm -monochrome -compress Group4 output.tiff`.
The [image_to_c](https://github.com/bitbank2/image_to_c) tool then generates the right c files with `./image_to_c --strip input.tiff > output.h`.
