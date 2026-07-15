# Frequently asked questions

### Where are the mechanical CAD files?
As this is a DIY project, I did not make any detailed mechanical CAD; only a plan
layout for the front plate of the transmitter. This is mainly because dimensions
of parts differ widely across manufacturers.

### Does it support protocol x?
Out of the box, no; there are just too many different and incompatible protocols.
You can however modify the firmware and hardware to support any protocol or
module you want.
There are also several DIY projects that address this problem
specifically, and interfacing to them would not be difficult.

### Is the RF link stable and what is the range?
The RF link is stable enough. Keep in mind that this implementation is not
designed for security, and is neither interference nor jamming free.  
The range largely depends on the specific RF module, power amplification
setting, quality of your antennas, your environment, etc. From my tests, I easily
got more than 2 km line of sight with monopole antennas and default LoRa
settings, with packet loss of less than 3%. This range is more than necessary to
control an RC model (of course you won't be able to see your model at such
distance anyway).

### What is the latency of this system? Servo jitter, lag?
Latency varies between 20 to 40 ms maximum between flicking a switch and
getting the output on the receiver’s pin. As for servo jitter, none; it is a matter
of quality of your servos; I have not experienced any issues with cheap ones like
the popular SG90 servos.

### Why still use 8-bit AVR and not something more powerful like a 32-bit ARM microcontroller?
It is what I had lying around at the time. I initially planned to use an STM32 but
the chip shortage for these forced me to rethink and revise the design.  
I found the Atmega2560 to be performant enough for this application after some
clever code optimizations and performance tuning. In fact, it takes roughly 15
ms on average to compute everything including redrawing the screen.

### Can it do voice alerts?
Voice alerts are not currently supported. For now, an alternative is to use
custom notifications with tunes to report events.  
I plan on working on this in the near future, but I haven't yet found a compelling
reason to add them.

### SD card is not working or can't be detected.
The card should be formatted with the FAT (FAT16 or FAT32) file system. Other
file systems are not supported.  
Try to keep the number of files on the card minimal.  
Always power the system off before inserting or removing the SD card.  
Also try using a class 4 or lower, 4 GB or smaller SD card.

### I am having trouble binding.
Double check your connections. Make sure the receiver has a stable supply
voltage. Also leave enough space between the transmitter and the receiver.  
During binding, ensure that the power to the receiver is cycled within the binding
window (typically 5 seconds), otherwise a timeout occurs.

### The analog controls do not work. I am getting wrong readings when I move them.
You probably skipped the calibration procedure, or you are doing it wrong.
See the section about stick and knob calibration for the proper procedure.  
The quality of your hardware also matters, even though the system will try its
best to cope with low quality gimbals.  
If your readings appear reversed, check your connections. You may have to
swap the VCC and GND wires on the problematic potentiometer.

### Models and system settings are corrupted after a firmware update.
Ensure you back up your models to the SD card before performing any firmware
updates. You can then restore them from the SD card afterwards.  
To back up the system settings, use the corresponding option in the Debug menu.
