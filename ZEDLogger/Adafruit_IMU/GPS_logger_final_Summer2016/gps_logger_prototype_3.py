#!/usr/bin/env python2.7
#version 2 incorporates goprohero library for wireless control of gopro
import RPi.GPIO as GPIO
from time import sleep
from goprohero import GoProHero
from wireless import Wireless
import time
import os
from gps import *
from time import *
import threading
import subprocess
 
GPIO.setmode(GPIO.BCM)  #set pin numbering mode to Broadcom values not physical numbers
GPIO.setup(6, GPIO.IN, pull_up_down=GPIO.PUD_UP) # set pin 6 as input, pulled up
GPIO.setup(17, GPIO.OUT,initial = 0) # RECORD indicator: set pin 17 as output, intially OFF
GPIO.setup(19, GPIO.OUT,initial = 1) # PROGRAM RUNNING indicator: set pin 19 as output, intially ON

class GpsPoller(threading.Thread):
  def __init__(self):
    threading.Thread.__init__(self)
    global gpsd #bring it in scope
    self.session = gps(mode=WATCH_ENABLE) #starting the stream of info
    self.current_value = None
    self.running = True #setting the thread running to true

  def get_current_value(self):
      return self.current_value
 
  def run(self):
    try:
        while self.running:
            self.current_value = self.session.next() #this will continue to loop and grab EACH set of gpsd info to clear the buffer
    except StopIteration:
        pass

def gps_logging_thread(gpsp, gps_log_stop, DATA_SET_NUMBER):
    fileName = '/home/pi/Documents/GPS_data/gps_data_log_%i.txt' %(DATA_SET_NUMBER)
    outfile = open(fileName,'w')
    i = 1
    while not gps_log_stop.isSet():
    	#print i
    	i = i+1
    	report = gpsp.get_current_value()
    	if report['class'] == 'TPV':
    	   lat = report['lat']
           lon = report['lon']
           tutc = report['time']
           fixMode = report['mode']
           outfile.write('%s\t%.8f\t%.8f\t%i dog\n' %(tutc,lat,lon,fixMode))
        #   print '%s\t%.8f\t%.8f\t%i' %(tutc,lat,lon, fixMode)
    	sleep(1)
    outfile.close()
    return

#def connect_to_gopro():
  
def start_recording(gpsp,camera, DATA_SET_NUMBER):
    print "start recording\n"
    GPIO.output(17,1)  #turn on indicator LED light
    camera.command('record','on')  #issue command to go pro to start recording
    gps_log_stop = threading.Event()   #create event to eventually terminate  gps logging
    t = threading.Thread(target=gps_logging_thread, args=(gpsp,gps_log_stop, DATA_SET_NUMBER)) #gps logging thread
    t.start() #start logging gps data to file
    return gps_log_stop     #return this event so it can be set by stop_recording function


def stop_recording(gps_log_stop, camera): 
    print "stop recording\n"
    camera.command('record','off')  #issue command to go pro to stop recording
    gps_log_stop.set()   #set event which tells gpslogger to stop logging
    GPIO.output(17,0)   #turn off indicator LED

def set_system_time(dummary_arg, gpsp): #thread function to set system time using gps data
    SET_TIME = 0
    print "in set_system_time thread"
    while not SET_TIME:
      report = gpsp.get_current_value()
      if report['class'] == 'TPV' and (report['mode'] == 2 or report['mode'] ==3):
          tutc = report['time']
          cmdString = 'sudo date -s %s --utc' %(tutc)
          subprocess.call(cmdString, shell=True)
          print cmdString
          SET_TIME = 1
      sleep(1)
    return



#### MAIN PROGRAM ########### 

try:
    PROG_STATUS = 1
    gpsp = GpsPoller() # create the thread
    gpsp.start() # start it up
    sleep(1) #allow gpsd thread to start up, otherwise first report is "None" 
    time_set_thread = threading.Thread(target=set_system_time,args=(1, gpsp)) #set system time based on gps data
    time_set_thread.start()
    camera = GoProHero(password = 'goprohero') #create gopro instance - gopro does not need to be connected yet, just needs to be connected when commands are issued
    RECORDING = 0  # status variable indictaing if data is being recorded or not
    DATA_SET_NUMBER = 1   #incrementing variable indicating number of data set acquired
    while PROG_STATUS:
        GPIO.wait_for_edge(6, GPIO.FALLING)
        #poll GPIO 5 to distingish short press (record start/stop) from long press (stop program)
        # ~ 3 second cuttoff to distinguish short from long
        for i in range(60):
           if GPIO.input(6):
              break
           sleep(0.05)

        if 5< i <58:
           print "start/stop recording\n"
           if RECORDING == 0:
               gps_log_stop = start_recording(gpsp, camera, DATA_SET_NUMBER)
               DATA_SET_NUMBER +=1
               RECORDING = 1
           elif RECORDING == 1:
               stop_recording(gps_log_stop, camera)
               RECORDING = 0
           
        elif i >= 58:
           PROG_STATUS = 0; #stop program
           print "Long button press detected, shutdown program\n"


except KeyboardInterrupt:
    print "terminated by user\n"
finally:
    print "cleaning up GPIO ports\n"
    GPIO.output(19,0)
    GPIO.cleanup()
    gpsp.running = False
    gpsp.join() #wait for thread to finish what it's doing
    
