# the deepest slumber

The standby library enables easy access to CH32V003s lowest power mode, standby, and wake by auto-wakeup timer, button(s) or both.  

Power consumption should be around 10uA.  

The MCU only toggles the LED and prints a message, then it goes back to sleep.  
The LED staying on demonstrates that GPIO keeps its state even when the rest of the mcu is in a coma.  

Based on the groundwork of Marek M.  
