# nRF24L001+ demo
this is the Arduino-based "friend" for [spi_24L01_tx](https://github.com/cnlohr/ch32v003fun/tree/master/examples/spi_24L01_tx)
The other half for Arduino is [here](https://github.com/recallmenot/ch32v003fun_friends/tree/main/Arduino/NRF24L01_TX)

## nRF24L01(+) library
This is the doing of [Reza Ebrahimi](https://github.com/ebrezadev), I've just included a copy of [his library](https://github.com/ebrezadev/nRF24L01-C-Driver) here and made some modifications.

## pinout for Arduino
same perspective
nRF     | UNO     | nRF   | UNO
--------|---------|-------|-------
VCC		  | 3V3			|	GND		| GND
CSN/SS	| 9				| CE		| 8
MOSI	  | 11			| SCK		| 13
IRQ		  | NC			| MISO	| 12

And an LED on pin 4, for current-limiting resistor 1k is usually safe with 5V.
