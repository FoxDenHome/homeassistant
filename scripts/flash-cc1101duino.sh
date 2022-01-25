#!/bin/sh
apk add avrdude
avrdude -v -P /dev/serial/by-id/usb-FOXDEN_CC1101Duino_1101-if00-port0 -patmega328p -carduino -D "-Uflash:w:$1:i"
