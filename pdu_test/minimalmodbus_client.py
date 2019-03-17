#!/usr/bin/python3

import minimalmodbus
import time

instrument = minimalmodbus.Instrument('/dev/ttyACM0', 0x0a) # port name, slave address (in decimal)
instrument.serial.baudrate = 9600
time.sleep(5)
## Read temperature (PV = ProcessValue) ##
coil = instrument.read_bit(10001, 1) # Registernumber, number of decimals
print(coil)
