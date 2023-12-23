## Frequently asked questions
### Where do i start?
Probably with the simulation to test out the features before committing your time and effort building the actual hardware. Also please note that
this project is not suitable for beginners. 

### Where are the mechanical CAD files?
As this is a DIY project, I did not make any detailed mechanical CAD, only a plan layout for the front 
plate of the transmitter. This is because part dimensions for example of the LCD screen differ
widely between manufacturers and it is not a "one size fits all" thing. I also do not have a 3D printer.

### Does it support protocol x?
Out of the box, no; there are just too many different and incompatible protocols. This compelled me to make a custom protocol.
<br>However, with dedication and a bit of technical expertise, you can extend the firmware 
and hardware to support any protocol or module you want. There are a number of DIY projects that address this problem specifically, and interfacing to them wouldn't be hard. 

### Is the RF link stable and what is the range?
The RF link is stable enough. Keep in mind that this implementation is not designed for security, and is neither interference nor jamming free.
<br>The range largely depends on the module, power amplification setting, quality of your antennas, your environment, etc. From my tests, I easily got more than 2km line of sight with monopole antennas and default LoRa settings, with packet loss of less than 3%. This range is more than necessary to control an RC model (of course you won't be able to see your model at such distance anyway).

### What is the latency of this system? Servo jitter, lag? 
Latency varies between 20 to 40ms maximum between flicking a switch and getting the output on the receivers pin.
As for servo jitter, none. It is a matter of quality of your servos; I have not experienced any issues
with cheap ones like the popular SG90 servos.

### Why still use 8-bit AVR and not something more powerful like a 32-bit ARM microcontroller? 
It is what I had lying around at the time. I initially planned to use an STM32 but the chip shortage for these 
forced me to rethink and revise the design. 
<br>I found the Atmega2560 to be performant enough for this application after some clever code optimizations and performance tuning.
In fact it takes less than 15ms on average to compute everything including redrawing the screen.

### Can it do voice alerts?
Voice alerts are not currently supported, as I did not see much need for this feature, in fact I find them quite annoying. You can
however modify the firmware to support voice via an external serial mp3 module.

### SD card is not working or can't be detected.
Make sure the card is formatted with the FAT (FAT16 or FAT32) file system. Other file systems are not supported. Also try to keep
the number of files on the card minimal.

### I have discovered a bug or I have an issue. 
Whereas I have done extensive testing for all features and fixed all known bugs and issues, I cannot guarantee total bug freeness. 
In case you come across a bug or have an issue, you can file it here on GitHub and I will be happy to look into it and support.
