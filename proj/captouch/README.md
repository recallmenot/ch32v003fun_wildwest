# CapTouch!!!
Our favorite victim of software abuse now gained the ability to sense capacitive touch!  

The library provides for reading from single pins, which you can use for buttons, as a slider or even as a scrolling slider!  

## working principle

A floating input pin has a high impedance, which means it can hold a tiny ammount of charge as it forms a capacitor against the environment.  
We use a pushpull pin to drive the sensing input at a rate controlled by a large resistor between them.  
1. drive the pushpull pin low
2. wait until the sense pin registers as discharged
3. note the time
4. drive the pushpull pin high
5. wait until the sense pin registers as charged
6. save the time it took to charge
7. drive the pushpull pin low to have it truly discharged the next time

Bringing a finger close to the sense pad increases its capacitance (ability to hold more charge), thus it takes longer to charge, thus we see a change in time, which we can use as we see fit.  
Since we're trying to measure a change in capacitance, any extra cable length between the sense pins and the capacitive surfaces adds parasitic capacitance.  

With only one pushpull pin, we can determine the charge time of any number of pins, BUT we have to wait until all sense pins and their sense surfaces are fully discharged between measurments (1ms is enough).  

## building it

### sniff the glue

to make the capsense pad:
1. print `pad_slider3.svg`, it was created using inkcape, measurements 50mm x 8mm and 30mm x 8mm
2. glue the aluminum foil on the back; glue-stick doesn't make the paper curl too much
3. cut
4. on your sensor platform, place some double-sided adhesive tape
5. place the paper-aluminum plates ontop of the double-sided adhesive tape, leaving an airgap of 1mm between them. fine tweezers are a great help here.
6. above the paper-aluminum plates, glue the 3 conductors in place using clear adhesive tape; I used silver coated 0.6mm copper wire, an unbent paper-clip would do fine aswell. you'll want to hold them firmly in place, make good contact to the aluminum yet not impede the users finger sliding too much.
7. glue some clear adhesive tape over the paper-aluminum plates to insulate the plates from the user and hide the ridges.
8. bend the wires to plug into a breadboard

### connections

One pin (C7 here) is the push-pull pin, it pushes and pulls the sense pins (digital inputs, floating) up and down.  
Connect the push-pull pin to each sense pin (C6, C5, D2) through >=1 megaohm resistor.  
Connect the sense pins to your capacitive sensing surfaces.  
LEDs (with current limiting resistors) are connected to C3 and C4.
