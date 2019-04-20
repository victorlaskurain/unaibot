#!/usr/bin/python3

import serial
import sys
import time
import os

msg = b'\x0a\x01\x27\x11\x00\x08\x66\x06'

def format(binary):
    return ' '.join(['0x%02x'%b for b in binary])

def main():
    os.system('stty -hup  -F /dev/ttyACM0');
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=5, dsrdtr=False)
    # time.sleep(2)
    while True:
        print(format(msg))
        ser.write(msg)
        line = ser.read(4)
        print(format(line))
        print
        time.sleep(1);

if __name__ == '__main__':
    main()
