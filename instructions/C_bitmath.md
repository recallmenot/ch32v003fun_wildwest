# quick reference
### set bit  
`|= 1 << bit_number`
### clear bit  
`&= ~(1<<bit_number)`

# operations

## AND
`A&B` returns the bits that are the same between A and B
### uses
testing bits
clearing bits through a bitmask
### example
```
	11001100
&	00001001
=	00001000
```
```
	11001100
&	00111111
=	00001100
```

## OR
`A|B` returns the bits present in A, B or both
### uses
setting bits
### example
```
	11001100
|	00010000
=	11011100
```

## NOT
`~A` inverts the bits
### example
```
~	11001100
=	00110011
```

## left / right shift
`A<<n` shifts the bits of A by n places.  
The bits "pushed on" from the right are zeroes.
The bits that "drop off" on the left side just cease to exist.
The same is true in the other direction `>>`
### example
```
<<2	11001100
=	00110000
```

## XOR
= exclusive or  
`A^B` retuns the bits that are opposite (not same)
### uses
toggling bits
### example
```
	11001100
^	00010000
=	11011100
```

# bit operations
## set bit
`|=` retrieves the value, then does an OR-operation with what you provide and stores the result
### example
```
	11001100    is loaded
|	00100000    
=	11101100    is stored back
```
## set bit, specifying bit number
`|= (1<<bit_number)` retrieves the value, then left-shifts the 1 to the bit number you specified, does an OR-operation and stores the result
### example
```
	11001100    is loaded
|	00100000    = (1<<5)
=	11101100    is stored back
```

## clear bit
`&= ~` inverts what you provide, retrieves the value, then does an AND-operation and stores the result
### example
```
~	01000000
=   10111111

	11001100    is loaded
&	10111111
=	10001100    is stored back
```
## clear bit, specifying bit number
`&= ~(1<<bit_number)` left-shifts the `1` by `bit_number` places, inverts it, retrieves the value, then does an AND-operation and stores the result
### example
```
~	01000000    = (1<<6)
=   10111111

	11001100    is loaded
&	10111111
=	10001100    is stored back
```

## toggle bit, specifying bit number
`^= (1<<bit_number)` left-shifts the `1` by `bit_number` places, retrieves the value, then does an XOR-operation and stores the result
### example
first time
```
	11001100    is loaded
^	01000000    = (1<<6)
=	10001100    is stored back
```
second time
```
	10001100    is loaded
^	01000000    = (1<<6)
=	11001100    is stored back
```
