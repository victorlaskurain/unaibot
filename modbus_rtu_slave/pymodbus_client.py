#!/usr/bin/python3

import logging
logging.basicConfig()
log = logging.getLogger()
log.setLevel(logging.DEBUG)

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

def main():
    client = ModbusSerialClient(
        method = "rtu",
        port="/dev/ttyACM0",
        stopbits = 1,
        bytesize = 8,
        parity = 'E',
        baudrate= 19200,
        dsrdtr=False,
        timeout=0.01)
    conn = client.connect()

    print('')
    print('client.read_holding_registers(0x0000, 4, unit=0x76)')
    registers = client.read_holding_registers(0x0000, 4, unit=0x76)
    print(registers)
    print(registers.registers)

    print('')
    print('client.read_holding_registers(0x0000, 1, unit=0x76)')
    registers = client.read_holding_registers(0x0300, 1, unit=0x76)
    print(registers)

    print('')
    print('client.read_coils(0x0000, 0x13, unit=0x76)')
    coil = client.read_coils(0x0000, 0x13, unit=0x76)
    print(coil)
    print([int(b) for b in coil.bits])

    print('')
    print('client.write_coil(0x0100, 1, unit=0x76)')
    coil = client.write_coil(0x0100, 1, unit=0x76)
    print(coil)

    print('')
    print('client.write_coils(0x0100, [0, 0], unit=0x76)')
    coil = client.write_coils(0x0100, [0, 0], unit=0x76)
    print(coil)

    print('')
    print('client.write_coils(0x0100, [0, 1], unit=0x76)')
    coil = client.write_coils(0x0100, [0, 1], unit=0x76)
    print(coil)

    print('')
    print('client.write_coils(0x0100, [1, 0], unit=0x76)')
    coil = client.write_coils(0x0100, [1, 0], unit=0x76)
    print(coil)

    print('')
    print('client.write_coils(0x0100, [1, 1], unit=0x76)')
    coil = client.write_coils(0x0100, [1, 1], unit=0x76)
    print(coil)

if __name__ == '__main__':
    main()
