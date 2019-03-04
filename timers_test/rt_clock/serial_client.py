#!/usr/bin/python

import serial
import sys
import time

def main():
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=5000)
    while True:
        line = ser.readline().strip()
        print(line)
        if line == 'BEGIN':
            tbegin = time.time()
        if line == 'ALARM 8"':
            t8s = time.time()
        if line == 'ALARM 10"':
            t10s = time.time()
        if line == 'DONE':
            break
    print('Time  8": %s'%(t8s  - tbegin))
    print('Time 10": %s'%(t10s - tbegin))

if __name__ == '__main__':
    main()
