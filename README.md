# FreeTX
Open source radio control system for almost anything.  
Remotely operate model airplanes, multicopters, helicopters, boats, land vehicles, robots, animatronics, etc.
<p align="left">
<img src="doc/tx_views.jpg" width = "480"/>
</p>

## Main features
- 20 fully proportional RC channels, each encoded with 10 bits.
- Configurable RC channel output signal format.
- Update rate of 50x per second for all RC channels.
- Receiver binding, adjustable RF power, frequency hopping.
- Reverse, subtrim, endpoints, failsafe.
- Dual rates and expo, custom curves, flight modes.
- Powerful mixer system, logical switches, function generators, counters, timers.
- Telemetry, alarms, warnings, notifications, home screen widgets.
- Custom splash screen, screenshot capture, etc.
- Model memory. I2C eeprom and SD card supported. 
- Model types, context-aware nomenclature.
- Intuitive user interface.

## Hardware and Schematics
The transmiter and receiver schematics can be found in the 'schematics' folder. 
The major components are
- 1x Atmega2560 microcontroller, 2x Atmega328p microcontrollers.
- 2x SX1276/77/78/79 or RFM95 based Lora modules. Other RF modules can possibly be used with some modifications to the source code.
- 128x64 KS0108 based LCD. Any 128x64 LCD can however be used (provide own driver code).

## User Interface
Three buttons are used for navigation; Up, Select, Down. Long press Select to go Back.
<p align="left">
<img src="doc/img1.svg"/>
</p>

## Quick guide
[Mixer](doc/mixer.md)
<br>[Custom curves](doc/custom_curves.md)
<br>[Logical switches](doc/logical_switches.md)
<br>[Function generators](doc/function_generators.md)
<br>[Timers](doc/timers.md)
<br>[Counters](doc/counters.md)
<br>[Notifications](doc/notifications.md)
<br>[Telemetry](doc/telemetry.md)
<br>[Receiver binding and configuration](doc/receiver_config.md)

## Gallery
<p align="left">
<img src="doc/img6.jpg" width = "240"/>
<img src="doc/img2.jpg" width="240"/>
<img src="doc/img4.jpg" width="240"/>
</p>

[<img src="doc/yt.jpg" width = "240"/>](https://www.youtube.com/watch?v=rDpRMRE8av0)
[<img src="doc/yt2.jpg" width = "240"/>](https://www.youtube.com/watch?v=9EZkyRHkMpg)

## Frequently Asked Questions
Got a question? [See the FAQ.](doc/faq.md)  
<br> This project is based on an older project found [here.](https://github.com/buk7456/Arduino-LoRa-RC-transmitter-and-reciever)
