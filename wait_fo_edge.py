#!/usr/bin/env python2.7


import RPi.GPIO as GPIO
GPIO.setmode(GPIO.BCM)
import time
import subprocess
from subprocess import Popen
import os
import signal
flag = 0


GPIO.setup(8, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
def getUptime():
	return time.time() - startTime




try:


	while True:
		GPIO.wait_for_edge(8,GPIO.BOTH)
		flag = flag +1
		
		#if flag == 1:
			#startTime = time.time()
			
		if flag%2 == 1:
			print flag
			s = 'recaved interrupt nr:'
			data ='flag\n' 
			startTime = time.time() #tim = getUptime()
			f = open('output.txt','a',0)
			f.write(s + repr(flag) + ' ' + ' test start \n')# + repr(tim) + ' test start \n')
			f.close
			
			
			
			Popen(['/home/pi/Datalogger/Datalogger oDroid.config'],shell=True)
			
		else:
			print flag
			p2= Popen(['killall Datalogger'],shell=True)
			p2.wait()
			s = 'recaved interrupt nr:'
			data ='flag\n' 
			tim = getUptime()
			f = open('power.log','a',0)
			f.write(s + repr(flag) + ' ' + repr(tim) + ' test stop \n')
			f.close
		
		
		


except KeyboardInterrupt:
    GPIO.cleanup()       # clean up GPIO on CTRL+C exit
GPIO.cleanup()           # clean up GPIO on normal exitpi
