import psutil
import time
import RPi.GPIO as GPIO
from datetime import timedelta

dataPin=3
dataClockPin=5

epulse = 0.010
edelay = 0.010
# sync word is 15 / 1111
charMap = { '0':0, '1':1, '2':2, '3':3, '4':4, '5':5, '6':6, '7':7, '8':8, '9':9, '.':10, ':':11, 'K':12, 'M':13}
masks = (8,4,2,1) # have to be in desc order so that we send highest bit first
toSend = {
	"cpuLoad":0,
	"memFree":0,
	"netIn":0,
	"netOut":0,
	"uptime":0
}

def readableSize(size):
	if (size > 1024):#K
		if (size > 1048576):#M
			return(str(str(size / 1048576)) + "M")
		else:
			return(str(str((size/1024)) + "K"))
	else:
		return(str(size))

def getUptime():
	with open('/proc/uptime', 'r') as f:
		uptime_seconds = float(f.readline().split()[0])
		uptime_string = str(timedelta(seconds = uptime_seconds))
	return(uptime_string[:-7])

def sendData():
	print(toSend);
	return;
	for i in toSend:
		for j in range(0,len(toSend[i])):
			for mask in masks:
				if (charMap[toSend[i][j]] & mask):
					GPIO.output(dataPin, 1);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);
				else:
					GPIO.output(dataPin, 0);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);

GPIO.setmode(GPIO.BOARD);
while(1):
	toSend["cpuLoad"]=str(psutil.cpu_percent());
	toSend["memFree"]=readableSize(psutil.avail_phymem());
	tp = psutil.network_io_counters();
	toSend["netIn"]=readableSize(tp[0]);
	toSend["netOut"]=readableSize(tp[1]);
	toSend["uptime"]=str(getUptime());
	sendData();
	time.sleep(1);
