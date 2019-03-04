#!/usr/bin/python

import serial
import sys
import time

def main():
    while True:
        ser = serial.Serial('/dev/ttyACM0', 9600, timeout=5000)
        last_time = time.time()
        while True:
            tag = ser.readline().strip()
            new_time = time.time()
            print('%s %s'%(tag, new_time - last_time))
            last_time = new_time

if __name__ == '__main__':
    main()
