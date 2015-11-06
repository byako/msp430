import os
import time
import RPi.GPIO as GPIO
from datetime import datetime

pinsMap={3:0,5:1,7:4,8:14,10:15,11:17,12:18,13:21,15:22,16:23,18:24,19:10,21:9,22:25,23:11,24:8,26:7}

Epulse = 0.000000140
Edelay = 0.000000200

dataPins=8,10,12,16,18,22,24,26
CS1=3
CS2=5
RST=7
RW=11
DI=13
E=15
commandPins=CS1,CS2,RST,RW,DI,E

debug = 0

class lcd:
	def __init__(self):
		self.dataValues = {0:0, 1:0, 2:0, 3:0, 4:0, 5:0, 6:0, 7:0};
		self.values = {CS1:0,CS2:0,RST:0,RW:0,DI:0,E:0}
		for i in dataPins:
			GPIO.setup(i, GPIO.OUT);

		for i in commandPins:
			GPIO.setup(i, GPIO.OUT);

		GPIO.output(E,0);
		GPIO.output(RW,0);
		GPIO.output(DI,0);
		GPIO.output(CS1,0);
		GPIO.output(CS2,0);
		time.sleep(0.03);

	def writeCommandPins(self):
		GPIO.output(CS1, self.values[CS1]);
		GPIO.output(CS2, self.values[CS2]);
		GPIO.output(RW, self.values[RW]);
		GPIO.output(DI, self.values[DI]);

	def writeDataPins(self):
		GPIO.output(dataPins[0], self.dataValues[0]);
		GPIO.output(dataPins[1], self.dataValues[1]);
		GPIO.output(dataPins[2], self.dataValues[2]);
		GPIO.output(dataPins[3], self.dataValues[3]);
		GPIO.output(dataPins[4], self.dataValues[4]);
		GPIO.output(dataPins[5], self.dataValues[5]);
		GPIO.output(dataPins[6], self.dataValues[6]);
		GPIO.output(dataPins[7], self.dataValues[7]);

	def clearScreen(self):
		self.disp12();
		self.setXAddress(0);
		self.setYAddress(0);
		self.setZAddress(0);
		for i in range(0,8):
			self.setXAddress(i);
			for j in range(0,64):
				self.setData(0);

	def dump(self):
		statusStr = ' Command: - [ CS1,CS2,RST,RW,DI,E ] | '
		for i in commandPins:
			statusStr += str(self.values[i])+' | ';
		statusStr += '- ; Data:  -| ';
		for i in range(0,8):
			statusStr += str(self.dataValues[i]) + ' | ';
		if debug > 0:
			print(str(datetime.now()) + statusStr);

	def setOnOff(self,onoff):# on: 1; off: 0
		self.lcdWaitReady();
		if debug >= 2:
			print(str(datetime.now()) +  (' Disabling ' if onoff == 0 else 'Enabling ') + ' display');
		self.values[DI] = 0; # instruction
		self.values[RW] = 0;
		if onoff == 1: # on
			self.dataValues = {0:1, 1:1, 2:1, 3:1, 4:1, 5:1, 6:0, 7:0};
		else:
			self.dataValues = {0:0, 1:1, 2:1, 3:1, 4:1, 5:1, 6:0, 7:0};
		self.writeLcd();

	def setYAddress(self, yadd):
		self.lcdWaitReady();
		if debug >=2:
			print(str(datetime.now()) + ' Setting Y address: ' + str(yadd));
		if (yadd < 0 or yadd > 63):
			print('ERROR: Y ADDRESS OUT OF RANGE [0-63] : ' + yadd);
			return;

		self.dataValues[0] = yadd & 0x01;
		self.dataValues[1] = yadd & 0x02;
		self.dataValues[2] = yadd & 0x04;
		self.dataValues[3] = yadd & 0x08;
		self.dataValues[4] = yadd & 0x10;
		self.dataValues[5] = yadd & 0x20;

		self.values[DI] = 0;
		self.values[RW] = 0;
		self.dataValues[6] = 1;
		self.dataValues[7] = 0;
		self.writeLcd();

	def setXAddress(self, xadd):
		self.lcdWaitReady();
		if debug >= 2:
			print(str(datetime.now()) + ' Setting X address: ' + str(xadd));
		if (xadd > 7 or xadd < 0):
			print('ERROR: X ADDRESS OUT OF RANGE [0-7] : ' + xadd);
			return;

		self.dataValues[0] = xadd & 0x01;
		self.dataValues[1] = xadd & 0x02;
		self.dataValues[2] = xadd & 0x04;

		self.values[DI] = 0;
		self.values[RW] = 0;

		self.dataValues[3] = 1;
		self.dataValues[4] = 1;
		self.dataValues[5] = 1;
		self.dataValues[6] = 0;
		self.dataValues[7] = 1;

		self.writeLcd();

	def setZAddress(self, zadd):
		self.lcdWaitReady();
		if debug >=2:
			print(str(datetime.now()) + ' Setting Z address: ' + str(zadd));
		if (zadd > 63 or zadd < 0):
			print('ERROR: Z ADDRESS OUT OF RANGE [0-63] : ' + zadd);
			return;

		self.dataValues[0] = zadd & 0x01;
		self.dataValues[1] = zadd & 0x02;
		self.dataValues[2] = zadd & 0x04;
		self.dataValues[3] = zadd & 0x08;
		self.dataValues[4] = zadd & 0x10;
		self.dataValues[5] = zadd & 0x20;

		self.values[DI] = 0;
		self.values[RW] = 0;
		self.dataValues[6] = 1;
		self.dataValues[7] = 1;

		self.writeLcd();

	def disp1(self):
		self.values[CS1] = 0;
		self.values[CS2] = 1;

	def disp2(self):
		self.values[CS1] = 1;
		self.values[CS2] = 0;

	def disp12(self):
		self.values[CS1] = 0;
		self.values[CS2] = 0;

	def setData(self, newDataValue):
		self.lcdWaitReady();
		if debug >= 2:
			print(str(datetime.now()) + ' Putting data...');

		self.dataValues[0] = newDataValue & 0x01 ;
		self.dataValues[1] = newDataValue & 0x02 ;
		self.dataValues[2] = newDataValue & 0x04 ;
		self.dataValues[3] = newDataValue & 0x08 ;
		self.dataValues[4] = newDataValue & 0x10;
		self.dataValues[5] = newDataValue & 0x20;
		self.dataValues[6] = newDataValue & 0x40;
		self.dataValues[7] = newDataValue & 0x80;

		self.values[DI]  = 1; # data
		self.values[RW]  = 0;

		self.writeLcd();

	def lcdWaitReady(self):
		for i in dataPins:
			GPIO.setup(i, GPIO.IN, pull_up_down=GPIO.PUD_DOWN);
		GPIO.output(RW,1);
		GPIO.output(DI,0);

		time.sleep(Epulse);
		GPIO.output(E,1);
		time.sleep(Edelay);

		while(GPIO.input(dataPins[7])):
			time.sleep(Epulse);

		GPIO.output(E,0);
		time.sleep(Epulse);

		for i in dataPins:
			GPIO.setup(i,GPIO.OUT);
		if debug >= 2:
			print(str(datetime.now()) + ' LCD READY');

	def writeLcd(self):
		if debug >= 2:
			print(str(datetime.now()) + ' Starting write');
		self.dump();
		self.writeCommandPins();
		self.writeDataPins();

		time.sleep(Epulse);
		GPIO.output(E,1);
		time.sleep(Edelay);
		GPIO.output(E,0);
		time.sleep(Epulse);

if __name__ == '__main__':
	print('RPI REVISION: ' + str(GPIO.RPI_REVISION));
	print('RPI SW VERSION: ' + str(GPIO.VERSION));
	print('setting board mode');
	GPIO.setmode(GPIO.BOARD);

	l = lcd();
	l.disp12();
	l.setOnOff(1);
	l.clearScreen();
	l.setYAddress(0);
	l.setXAddress(0);
	l.setZAddress(0);
	for j in range(0,8):
		l.setXAddress(j);
		for i in range(0,8):
			l.setData(0xC3);
	exit(0);
