import socket
import time
import subprocess
from subprocess import Popen
import os
import signal
flag = 0
 
UDP_IP = "192.168.11.62"#"127.0.0.1"
UDP_PORT = 5005
 
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))

#while True:
#	data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
#	print "received message:", data
	




def getUptime():
	return time.time() - startTime




try:

	while True:
		data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
		if data == 'INTERRUPT':
			print data
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
	print "keybord interrupt"
