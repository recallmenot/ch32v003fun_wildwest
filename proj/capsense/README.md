# capsense!!!

Our favorite victim of software abuse now gained the ability of capsense!
The library provides for reading from single pins, which you can use for buttons or even with the slider functions!
It won't be very accurate since we're relying on 


This capaciti
ve library 
The standby library enables easy access to CH32V003s lowest power mode, standby, and wake by auto-wakeup timer, button(s) or both.  

Power consumption should be around 10uA.  

The MCU only toggles the LED and prints a message, then it goes back to sleep.  
The LED staying on demonstrates that GPIO keeps its state even when the rest of the mcu is in a coma.  

### connections

One pin is the push-pull pin, it pushes and pulls the sense pins (digital inputs, floating) up and down.
Connect the push-pull pin to each sense pin through >=1 megaohm resistor.
Connect the sense pins to your capacitive sensing surfaces.
We're trying to measure a change in capacitance, any extra cable length between the sense pins and the capacitive surfaces adds parasitic capacitance.

### sniff the glue

make the capsense pad:

1. print `pad_slider3.svg`, it was created using inkcape, measurements 50mm x 8mm and 30mm x 8mm
2. glue the aluminum foil on the back; glue-stick doesn't make the paper curl too much
3. cut
4. on your sensor platform, place some double-sided adhesive tape
5. using tweezers, place the paper-aluminum plates ontop of the double-sided adhesive tape, leaving an airgap of 1mm between them
6. above the paper-aluminum plates, glue the 3 conductors in place using clear adhesive tape; I used silver coated 0.6mm copper wire, an unbent paper-clip would do fine aswell. you'll want to hold them firmly in place, make good contact to the paper-aluminum plates yet not impede the users finger too much.
7. glue some clear adhesive tape over the paper-aluminum plates to insulate the plates from the user and hide the ridges.
8. bend the wires to plug into a breadboard
