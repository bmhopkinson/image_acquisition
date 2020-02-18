#!/bin/sh
echo "ubuntu" | sudo -S systemctl stop gpsd.socket
echo "ubuntu" | sudo -S systemctl disable gpsd.socket
echo "ubuntu" | sudo -S gpsd /dev/ttyTHS2 -F /var/run/gpsd.sock
echo "ubuntu" | sudo -S /home/ubuntu/Documents/ZEDLogger/ZEDLogger


