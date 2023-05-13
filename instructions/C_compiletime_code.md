The pre-processor is a powerful text processing tool and runs before the actual compilation.

# compile-time math

Math using the #define directive is executed by the compiler
The major benefit is that this math won't end up on the MCU.
You might need to adapt the calculations for the compiler to calculate them at compile-time though.

# optional code

The preprocessor can help make code optional and easily turned on / off.  
This can be useful for "heavy" functions like printing to serial.  

first #define a control value:
`#define SERIAL_LEVEL = 1`

## wrap in #if
This approach is well-suited for wrapping lines that are not used often, like an `#include` or a `library_initialize()`.  

### example
```
#if SERIAL_LEVEL == 1
#include "library.h"
#endif
```

## proxy-macro
This approach is well-suited to lines that will be repeated often in your code (e.g. function calls).
The principle is to call the function if you set the correct controll value and otherwise let the proprocessor insert an emptyline in its place.

### example
```
#if SERIAL_LEVEL == 1
#define LOG(x) printf(x)
#else
#define LOG(x)
#endif
```

## enable / disable
You can now easily enable / disable the code by altering the control value.  

Since the `#if` directive supports the usual C comparison operators and you're not lmited to using 1 or 0, you can controll your code in complex ways.
