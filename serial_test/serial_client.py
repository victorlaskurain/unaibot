#!/usr/bin/python

import serial
import sys

def echo():
    s = "agur\n\0";
    while True:
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
        sys.stdout.write(s)
        ser.write(s)
        sys.stdout.write(ser.readline())
def only_read():
    while True:
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
        sys.stdout.write(ser.readline())
def main():
    echo()

if __name__ == '__main__':
    main()
