# Telemetry
The receiver has built-in basic telemetry i.e external voltage, rssi, and packet rate. 
<br>To measure other paramaters, you will need to build custom sensors and extend the receiver firmware to support these.
<br>All telemetry parameters are transmitted as 16-bit signed values, which unfortunately means that 
certain data, such as strings (GPS data, etc), cannot be sent to the transmitter.
<br>To quickly mute/unmute telemetry alarms, long press the Down key on the home screen.

## Fields
- Name: The sensor name.
- Units: The units of measurement.
- ID: This is the sensor ID. It needs to be unique, but we can use the same ID if for example we want
to display a value in two ways for example altitude as Relative altitude and Above sea level altitude.
- Factor10: The raw value is multiplied by 10 to power this factor.
- Multiplier: The raw value is multiplied by this number. Useful when we want to scale the value.
- Threshold: This is the value below or above which the telemetry alarm will be sounded.
- Alerts: This parameter specifies how the alarm behaves.
- On home: Whether to show the telemetry sensor on the home screen.
- Record max, and record min: Whether to track the max/min values received from the sensor. Useful 
for simple stats. Note that max and min are sensitive to outliers hence the filtering and smoothing
should be done before sending the telemetry to the transmitter.

