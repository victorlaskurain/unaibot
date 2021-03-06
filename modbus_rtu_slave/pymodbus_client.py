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

def get_and_print_coils(client):
    print('')
    addr = 0x0000
    bits = 0xd8
    print('client.read_coils(0x%04x, 0x%02x, unit=0x76)'%(addr, bits))
    coil = client.read_coils(addr, bits, unit=0x76)
    print(coil)
    bits = [int(b) for b in coil.bits]
    print ('        0, 1, 2, 3, 4, 5, 6, 7')
    for i in range(int(len(bits) / 8)):
        print('0x%04x %s'%(addr + i * 8, bits[i * 8:i * 8 + 8]))

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
    print('client.write_register(0x0000, 0x10, unit=0x76)')
    register = client.write_register(0x0000, 0x10, unit=0x76)
    print(register)

    print('')
    print('client.write_registers(0x0000, [0x10, 0x20, 0x30], unit=0x76)')
    registers = client.write_registers(0x0000, [0x10, 0x20, 0x30], unit=0x76)
    print(registers)

    print('')
    print('client.read_holding_registers(0x0000, 1, unit=0x76)')
    registers = client.read_holding_registers(0x0300, 1, unit=0x76)
    print(registers)

    get_and_print_coils(client)

    print('SET PINS PD2 TO PD7 DIRECTION AS OUTPUT')
    print('client.write_coils(0x004a, [1, 1, 1, 1, 1, 1], unit=0x76)')
    coil = client.write_coils(0x004a, [1, 1, 1, 1, 1, 1], unit=0x76)
    print(coil)
    get_and_print_coils(client)

    print('SET PINS PD2 TO PD7 TO ONE')
    print('client.write_coils(0x00c2, [1, 1, 1, 1, 1, 1], unit=0x76)')
    coil = client.write_coils(0x00c2, [1, 1, 1, 1, 1, 1], unit=0x76)
    print(coil)
    get_and_print_coils(client)

    print('\nSET PIN PB5 DIRECTION AS OUTPUT')
    print('client.write_coil(0x0055, 1, unit=0x76)')
    coil = client.write_coil(0x0055, 1, unit=0x76)
    print(coil)
    get_and_print_coils(client)

    print('\nSET PIN PB5 TO TO ONE')
    print('client.write_coil(0x00cd, 1, unit=0x76)')
    coil = client.write_coil(0x00cd, 1, unit=0x76)
    print(coil)
    get_and_print_coils(client)

    return

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
