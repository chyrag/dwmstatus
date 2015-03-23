#!/usr/bin/env python

# dwm status

# network (lan/wlan essid)
# biff (gmail)
# temperature
# battery info
# load
# date/time (in city0)
# date/time (in city1)

import os
import array
import fcntl
import socket
import struct
import datetime
import pytz
import time
import sched

def getESSID(interface):
    """Return the ESSID for an interface, or None if we aren't connected."""
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    maxLength = { "interface": 16, "essid": 32 }
    calls = { "SIOCGIWESSID": 0x8B1B }
    essid = array.array("c", "\0" * maxLength["essid"])
    essidPointer, essidLength = essid.buffer_info()
    request = array.array("c",
                          interface.ljust(maxLength["interface"], "\0") +
                          struct.pack("PHH", essidPointer, essidLength, 0)
    )
    fcntl.ioctl(sock.fileno(), calls["SIOCGIWESSID"], request)
    name = essid.tostring().rstrip("\0")
    if name:
        return name
    return None

def getMailStatus():
    # TBD
    # user, password = getAuthInfo()
    pass

def getTemperature():
    # for Dell Latitude E5440, the following path has the max
    # temperature; we'll use that.
    temp_input = '/sys/class/hwmon/hwmon1/device/temp1_input'
    f = open(temp_input)
    temp = int(f.read().rstrip())/1000
    f.close()
    # the OS reports temperature in milli units; divide by 1000 to get
    # the units (Centigrade)
    #return temp.__str__() + chr(176) + "C"
    return temp.__str__() + "C"


def getBatteryInfo():
    # for Dell Latitude E5440, the following path has battery info.
    bat0 = '/sys/class/power_supply/BAT0'
    capacity = bat0 + '/capacity'
    status = bat0 + '/status'
    f = open(capacity)
    cap = f.read().rstrip()
    f.close()
    f = open(status)
    st = f.read()[:2]
    f.close()
    return cap + "% " + st
    

def getLoad():
    return os.getloadavg()[0]


def getDate(tz=None, fmt=None):
    if not fmt:
        fmt = "%a %d %b %Y %R %Z"
    if not tz:
        return time.strftime(fmt, time.localtime())
    else:
        return datetime.datetime.now(tz).strftime(fmt)

def getStatus():
    print "[%s] %s (%s) %s %s" % (getESSID('wlan0'),
                                 getDate(pytz.timezone('Asia/Kolkata')),
                                 getDate(pytz.timezone('America/Los_Angeles'), "%H:%M %Z"),
                                 getTemperature(),
                                 getLoad())


if __name__ == "__main__":
    getStatus()
