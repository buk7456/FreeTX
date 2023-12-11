# Receiver 
Each receiver supports up to 10 channels. To use all the 20 channels, two receivers are required.

## Binding to a receiver
A receiver can be bound as either a main receiver or a secondary receiver. 
<br>By default, RC channels 1 to 10 are handled by the main receiver, while RC channels 11 to 20 are 
handled by the secondary receiver.
<br>To bind the receiver, on the transmitter side access the receiver menu and select the bind option. 
Then cycle the power to the receiver. If the binding is successful, a "Success" toast message will be shown and a tune played.

## Configuring receiver outputs
The receiver outputs can be configured from the transmitter to any of the three signal types; 
servo PWM, digital on-off, or 'normal' PWM. 
For example 'normal' PWM output can be used to control brushed DC motors without the need for 
complex electronics. The digital on-off output makes it easy to directly control components such as 
electromechanical relays and lights.
<br>Note: 
1. These configuration settings are stored in the receiver, not the transmitter.
2. Normal PWM is only available on certain pins, depending on the pin mapping.
3. Any changes made take effect immediately upon pressing the 'Write' button.
4. In digital on-off mode, the output value range of -100 to -50 corresponds to LOW, -49 to 49 is 
ignored, and 50 to 100 corresponds to HIGH.
