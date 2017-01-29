#!/usr/bin/env python
import minimalmodbus

instrument = minimalmodbus.Instrument('/dev/ttyUSB0', 2) # port name, slave address (in decimal)
instrument.serial.baudrate = 115200
instrument.serial.stopbits = 2
instrument.serial.timeout  = 5

## Read temperature (PV = ProcessValue) ##
temperature = instrument.read_register(0, 2) # Registernumber, number of decimals
print temperature
if temperature >= 30:
  instrument.write_register(3,1,1)
else:
  instrument.write_register(3,0,1)

temperature = instrument.read_register(1, 2) # Registernumber, number of decimals
print temperature

if temperature >= 30:
  instrument.write_register(4,1,1)
else:
  instrument.write_register(4,0,1)

## Change temperature setpoint (SP) ##
#NEW_TEMPERATURE = 95
#instrument.write_register(7, NEW_TEMPERATURE, 1) # Registernumber, value, number of decimals for storage
