#!/bin/sh
sudo killall gpsd
sudo gpsd /dev/ttyUSB0 -F /var/run/gpsd.sock
sudo python /home/pi/Desktop/GPS_IMU_logger/gps_logger_prototype_3.py