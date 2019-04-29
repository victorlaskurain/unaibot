#!/usr/bin/python3

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.WARNING)

from pymodbus.client.sync import ModbusSerialClient
from pymodbus import utilities
import os
import time

# This prevents arduino autoreset. It does not work on the first call
# because it doesn't give time to the Arduino program to
# initialize. After that it is not necessary until the configuration
# of the port changes again but keeping it here seems like the
# cleanest and simplest solution.
os.system('stty -hup -F /dev/ttyACM0')

client = ModbusSerialClient(
    method = "rtu",
    port="/dev/ttyACM0",
    stopbits = 1,
    bytesize = 8,
    parity = 'E',
    baudrate= 9600,
    dsrdtr=False,
    timeout=0.01)
conn = client.connect()

print('')
print('client.read_coils(10001, 8, unit=0x76)')
coil = client.read_coils(10001, 8, unit=0x76)
print(coil)

print('')
print('client.write_coil(0x0000, 1, unit=0x76)')
coil = client.write_coil(0x0000, 1, unit=0x76)
print(coil)

print('')
print('client.write_coils(0x0000, [0, 0], unit=0x76)')
coil = client.write_coils(0x0000, [0, 0], unit=0x76)
print(coil)

print('')
print('client.write_coils(0x0000, [0, 1], unit=0x76)')
coil = client.write_coils(0x0000, [0, 1], unit=0x76)
print(coil)

print('')
print('client.write_coils(0x0000, [1, 0], unit=0x76)')
coil = client.write_coils(0x0000, [1, 0], unit=0x76)
print(coil)

print('')
print('client.write_coils(0x0000, [1, 1], unit=0x76)')
coil = client.write_coils(0x0000, [1, 1], unit=0x76)
print(coil)
