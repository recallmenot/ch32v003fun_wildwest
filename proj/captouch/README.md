# capTouch TM (C) patent pending rights reserved
Our favorite victim of software abuse now gained the ability to sense capacitive touch!  
The library provides for reading from single pins, which you can use for buttons, as a slider or even as a scrolling slider!  

## working principle

A floating input pin has a high impedance, which means it can hold a tiny ammount of charge as it forms a capacitor against the environment.  

A sense pin can pull itself up (regular pushpull output mode) to VDD and then go into input mode (high impedance).  
Now, a large resistor (>=1 megaohm) connecting it to ground will pull it down at a controlled rate.  

Bringing a finger close to the sense pad increases its capacitance (ability to hold more charge), thus it takes longer to charge, thus we see a change in time, which we can use as we see fit.  
Since we're trying to measure a change in capacitance, any extra cable length between the sense pins and the capacitive surfaces adds parasitic capacitance.  

It's a bit like a motor glider pulling himself up into the sky using the engine, then turning it off, starting the stopwatch and stopping it on touchdown.  
In that analogy, the extra capacitance from the finger is like steadily rising air slowing the planes descent.  

## building it

### sniff the glue

to make both capsense pads:
1. print `pad_slider3.svg`, it was created using inkcape and prints fine there, measurements 50mm x 8mm and 30mm x 8mm
2. cut out both slider pads from the paper, leave 10 mm on all sides, don't separate the two yet
3. cut aluminium foil to around the same size
4. retrieve the _tactical_ glue stick from the weapons cabinet
5. coat the bottom half of the undersides of the sliders with glue stick, we want to only glue the bottom half so behind the top half we can insert wires later
6. slap on the aluminium foil, let the glue cure
7. cut the sliders out, then separate their pieces
8. on your sensor platform, place some double-sided adhesive tape
9. place the paper-aluminium plates ontop of the double-sided adhesive tape, leaving an airgap of 1-2mm between them. fine tweezers are a great help here. the glued half needs to face downwards.
10. from the top side of your platform, insert the conductors between paper and aluminium, one by one, each time snugly fixing them to the platform with a small piece of clear adhesive tape. Ensure good electrical contact to the aluminium foil from below.
11. glue a large piece of clear adhesive tape over the paper-aluminium plates and the double-sided adhesive to insulate the aluminium plates from the user. This ensures only capacitive-coupling takes place.
12. glue another large piece clear adhesive tape over the row of connectors to give them more rigidity.
13. bend the wires to plug into a breadboard, first in air to get the rough shape, then plugged in to get them precise and relaxed. take care not to pull them out from under the aluminium foil.

#### conductor choice

The conductor wires should be thin enough to not impede the users finger sliding too much but also make good contact with the breadboard terminals.
I used silver coated 0.6mm copper wire, an unbent paper-clip would do fine aswell, even if thicker.
You can tell you have good contact if all 3 pads register a small change (~5 ticks) just by proximity of your finger, without exerting any pressure on it.
Symptoms of bad contact:
 * no readings with <1 mm proximity but not touching
 * jumpy output while gently pressing the finger on it in one spot
 * high sensitivity while moving the finger slowly but low sensitivity while moving the finger quickly (though this is also affected by a slow poll rate)

### connections

Connect the capacitive sensing plates to each sense pin (C6, C5, D2).
Connect each sense pin to GND through its own large resistor (>=1 megaohm).
LEDs (with current limiting resistors) can be connected to C4 (status) and D4 (PWM).
