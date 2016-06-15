import psutil
import time
import RPi.GPIO as GPIO
from datetime import timedelta

dataPin=3
dataClockPin=5

epulse = 0.000050
edelay = 0.00005
# sync word is 127 / 01111111 / 7F
charMap = [
' ', '!', '"', '#', '$', '%', '&', '\'', '(', ')',
'*', '+', ',', '-', '.', '/', '0', '1', '2', '3',
'4', '5', '6', '7', '8', '9', ':', ';', '<', '=',
'>', '?', '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G',
'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q',
'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[',
'\\', ']', '^', '_', '`', 'a', 'b', 'c', 'd', 'e',
'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y',
'z', '{', '|', '}', '~' ]
#charMap = { '0':0, '1':1, '2':2, '3':3, '4':4, '5':5, '6':6, '7':7, '8':8, '9':9, '.':10, ':':11, 'K':12, 'M':13, ' ':14, 'Z':15}
charMasks = (64,32,16,8,4,2,1) # have to be in desc order so that we send highest bit first
toSendMasks = (4,2,1)
toSend = {
	0:0,
	1:0,
	2:0,
	3:0,
	4:0
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
#	print(toSend);
	for i in range(0,len(toSend)):
#		print(toSend[i]);
		for j in range(0,len(toSend[i])):
			for mask in toSendMasks:
				if (i & mask):
#					print('-1');
					GPIO.output(dataPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);
				else:
#					print('-0');
					GPIO.output(dataPin, 0);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);
			if j == 0:
				idx = 127
			else:
				idx = charMap.index(toSend[i][j])
#				print(toSend[i][j])
#				print(idx)
			for mask in charMasks:
				if (idx & mask):
#					print('-1');
					GPIO.output(dataPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);
				else:
#					print('-0');
					GPIO.output(dataPin, 0);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 1);
					time.sleep(epulse);
					GPIO.output(dataClockPin, 0);
					time.sleep(edelay);

GPIO.setmode(GPIO.BOARD);
GPIO.setup(dataPin, GPIO.OUT);
GPIO.setup(dataClockPin, GPIO.OUT);
GPIO.output(dataClockPin, 0);
GPIO.output(dataPin, 0);

while(1):
	toSend[0]=str('Z'+str(psutil.cpu_percent()));
	toSend[1]=str('Z'+readableSize(psutil.avail_phymem()));
	tp = psutil.network_io_counters();
	toSend[3]=str('Z'+readableSize(tp[0]));
	toSend[4]=str('Z'+readableSize(tp[1]));
	toSend[2]=str('Z'+str(getUptime()).replace(" days","d"));
	sendData();
	time.sleep(1);
