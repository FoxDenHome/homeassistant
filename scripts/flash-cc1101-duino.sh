#!/bin/sh

avrdude -v -P /dev/serial/by-id/usb-SHK_NANO_CUL_433-if00-port0 -patmega328p -carduino -D "-Uflash:w:$1:i"
