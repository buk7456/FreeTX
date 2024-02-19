# FreeTX
Remotely operate model airplanes, multicopters, boats, land vehicles, robots, etc.
<p align="left">
<img src="doc/tx_views.jpg"/>
</p>

## Main features
- 20 fully proportional RC channels, each encoded with 10 bits.
- Configurable RC channel output signal format.
- Update rate of 50x per second for all RC channels.
- Receiver binding, Adjustable RF power, Frequency hopping.
- Reverse, subtrim, endpoints, failsafe.
- Dual rates and expo, Custom curves, Flight modes.
- Powerful mixer system, Logical switches, Function generators, Counters, Timers.
- Custom telemetry, Alarms, Warnings, Notifications, etc.
- Intuitive user interface.
- Model memory. I2C eeprom and SD card supported.
- Model types, context-aware nomenclature.

## User Interface
Three buttons are used for navigation; Up, Select, Down. Long press Select to go Back.
<p align="left">
<img src="doc/img1.svg"/>
</p>

## Hardware and Schematics
The transmiter and receiver schematics can be found in the 'schematics' folder. 
The major components are listed below.
- 1x Atmega2560 microcontroller, 2x Atmega328p microcontrollers.
- 2x SX1276/77/78/79 or RFM95 based Lora modules. However any module can possibly be used.
- 128x64 KS0108 based LCD. Any 128x64 LCD can however be used (provide own driver code).

<p align="left">
<img src="doc/img2.jpg" width="400"/>
<img src="doc/img4.jpg" width="400"/>
</p>

## User guide
[Mixer](doc/mixer.md)
<br>[Custom curves](doc/custom_curves.md)
<br>[Logical switches](doc/logical_switches.md)
<br>[Telemetry](doc/telemetry.md)
<br>[Receiver binding and configuration](doc/receiver_config.md)

## Demonstration videos
[![Features overview - FreeTX](doc/yt.jpg)](https://www.youtube.com/watch?v=rDpRMRE8av0) 
[![RF link test - FreeTX](doc/yt2.jpg)](https://www.youtube.com/watch?v=9EZkyRHkMpg)

## Frequently Asked Questions
[Got a question? See the FAQ first.](doc/faq.md)
