#!/bin/bash
cd 
. ~/esp/esp-idf/export.sh
cd ~/CLionProjects/sensor-dev

idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
#use ctrl+] to exit monitoring
