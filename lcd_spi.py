import os
import time
import RPi.GPIO as GPIO
from datetime import datetime

dataPin=3
dataClockPin=5

Epulse = 0.100
Edelay = 0.100

debug = 0

class lcd:
	def __init__(self):
		GPIO.setup(dataPin, GPIO.OUT);
		GPIO.setup(dataClockPin, GPIO.OUT);
		GPIO.output(dataClockPin, 0);
		GPIO.output(dataPin, 0);

	def writeLcdDemo2(self):
		for i in range(0,8):
			time.sleep(Epulse);
			GPIO.output(dataClockPin, 1);
			time.sleep(Edelay);
			GPIO.output(dataClockPin, 0);

	def writeLcdDemo1(self):
		GPIO.output(dataPin, 1); # start with 1
		for i in range(0,8):
			time.sleep(Epulse);
			GPIO.output(dataClockPin, 1);
			time.sleep(Edelay);
			GPIO.output(dataClockPin, 0);
			GPIO.output(dataPin, 0);


if __name__ == '__main__':
	print('RPI REVISION: ' + str(GPIO.RPI_REVISION));
	print('RPI SW VERSION: ' + str(GPIO.VERSION));
	print('setting board mode');
	GPIO.setmode(GPIO.BOARD);

	l = lcd();
	l.writeLcdDemo2();

	exit(0);
