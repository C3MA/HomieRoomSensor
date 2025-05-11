#!/usr/bin/python3
import serial
import time
import argparse
import re
import random
from termcolor import colored
from time import localtime, strftime, gmtime
from datetime import datetime, timezone

parser = argparse.ArgumentParser()
parser.add_argument('-d', '--device', help='ttyUSB device. By default, /dev/ttyUSB0 is used.')
args = parser.parse_args()

if (args.device):
    serialDevice=args.device
else:
    serialDevice="/dev/ttyUSB0"

with serial.Serial(serialDevice, 19200, timeout=1)  as ser:
    print(ser.name)         # check which port was really used

    lastUpdate=0

    # some dummy values
    battery_volt=12000

    # Main Loop
    while (True):
        s=ser.readline()
        now=int(time.time())
        updateSerial=((now - lastUpdate) > 2)

        if ((s is not None) and (len(s) > 0)):
            print(colored(strftime("%Y-%m-%d %H:%M:%S", gmtime()) + " " + str(s.decode('utf-8', errors='ignore')).rstrip(), "green"))
        
        if (updateSerial):
            lastUpdate=int(time.time())
            # Send the status string
            outStr="SER#\tHQ2202K3VD9\nV\t{0:d}\nChecksum	f\r\n".format(battery_volt)
            # write a string
            ser.write(outStr.encode(encoding='UTF-8'))
            print(colored(outStr.rstrip(), "white"))
            time.sleep(0.2)
    
    ser.close()             # close port

# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4
