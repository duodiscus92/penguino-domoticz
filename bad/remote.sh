#!/bin/bash
/usr/bin/mosquitto_sub -h 192.168.1.39 -v -t "domoticz/out" | /home/pi/penguino-domoticz/jsonextractor -i 1029| /home/pi/penguino-domoticz/irsend -d -l -r 5 2>>/home/pi/penguino-domoticz/lgtv.log &
/usr/bin/mosquitto_sub -h 192.168.1.39 -v -t "domoticz/out" | /home/pi/penguino-domoticz/jsonextractor | /home/pi/penguino-domoticz/irsend -d  -r 5 2>>/home/pi/penguino-domoticz/penguino.log  &
