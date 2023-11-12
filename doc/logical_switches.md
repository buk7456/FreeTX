# Logical switches
These can be used to combine various conditions, compare values, etc. They are in essence user
programmed switches. They produce an on/off output.
<br>The available functions can be categorized into the following types.
- Comparators (Greater than, Less than, Equal, Greater than or equal, Less than or equal)
- Combinational (AND, OR, XOR)
- Flip flop (Latch, Toggle)
- Generators (Pulse)

### Example 1: A mix activated by combination of two switches
Suppose we want a mix to be turned on only when two switches (e.g SwA, SwB) are both on. 
We need to set up a logical switch say L1 as follows.

```txt
L1
---
Functn: AND
Value1: SwA_down
Value2: SwB_down
```
Then in the mixer, we simply have to set the control switch to L1.

### Example 2: LED lights that flash when flight battery is low
Suppose we have a model that is equipped with some LED lights that we want to switch on when the flight battery
voltage (telemetry) falls below a certain threshold for more than say five seconds.
<br>We can easily achieve this like so.

```txt
L1
---
Functn: a<x
Value1: FlghtBatt
Value2: 10.80V
Delay:  5.0s

L2
---
Functn: Pulse
Width:  1.0s
Period: 2.0s
Delay:  --

``` 
Then in the mixer, we simply use L2 as the input source and L1 as the control switch for the mix, to
operate the channel assigned to the LED lights.

### Example 3: Stopwatch operated by a single momentary switch for start/stop and reset
When the switch is clicked, the timer runs/stops. When the switch is held long enough, the timer is
reset. 
<br>For this we set up two logical switches to operate the timer as below, assuming SwH is our momentary switch.

```txt
L1
---
Functn: Toggle
Clock:  SwH_down
Edge:   Falling
Clear:  L2

L2
---
Functn:   a>x
Value1:   SwH
Value2:   0
Delay:    0.5s
Duration: --

Timer1
------
Switch:  L1
Reset:   L2
```
