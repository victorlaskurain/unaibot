#!/usr/bin/python3

# import logging
# logging.basicConfig()
# log = logging.getLogger()
# log.setLevel(logging.DEBUG)

from pymodbus.client.sync import ModbusSerialClient
from pymodbus import utilities

client = ModbusSerialClient(
    method = "rtu",
    port="/dev/ttyACM0",
    stopbits = 1,
    bytesize = 8,
    parity = 'E',
    baudrate= 9600,
    timeout=0.1)
conn = client.connect()
print(conn)
coil = client.read_coils(10001, 1, unit=0x0a)
print(coil)
