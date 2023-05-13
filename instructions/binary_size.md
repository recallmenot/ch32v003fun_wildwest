dec = what gets written to flash

components
*   text = code
*   data = values (constant, initialization)
*   bss (block started by symbol) = uninitialized variables

To retrieve the size from an .elf after compilation, run:  
`riscv64-unknown-elf-size *.elf`
