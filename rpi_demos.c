#include <msp430g2553.h>
#include <intrinsics.h>
#include "chars.h"

/*****************************************************************************

 To communicate with LCD we need 8 data pins and 4 command pins out of 16 av.
 DB0 - DB7     : p1.0 - p1.7  // data pins
 RS            : p2.0         // L - reset / H - working
 R/W           : p2.1         // H - read / L - write
 CS1           : p2.2         // Select column 1  ~ 64
 CS2           : p2.3         // Select column 65 ~ 128
 D/I           : p2.4         // H - data / L - instruction
 E             : p2.5         // H - enable signal

 shiftReg      : p2.6 / #19
 shiftRegClock : p2.7 / #18

 GPIO registers contents:
  P1OUT = [ [DB7] [DB6] [DB5] [DB4] [DB3] [DB2] [DB1] [DB0] ]
  P2OUT = [ [ - ] [ - ] [E  ] [D/I] [CS2] [CS1] [R/W] [RS ] ]

******************************************************************************

 Display size:
	2 displays, each one has
	X: vertical pages: 8 pages, 8 pixels each
	Y: horizontal lines, 64 lines

******************************************************************************/

// P2OUT WRITE BITS
#define LCD_RS  BIT0;
#define LCD_RW  BIT1;
#define LCD_CS1 BIT2;
#define LCD_CS2 BIT3;
#define LCD_DI  BIT4;
#define LCD_E   BIT5;

// DELAYS in Nano Seconds
#define LCD_tAS    186     /* Address setup time (ctrl line changes to E HIGH    */
#define LCD_tDDR   248     /* Data setup time (data lines setup to dropping E)   */
#define LCD_tDH    186     /* common data hold delay after E drop down           */

// DIRECTIONS
#define LCD_DATA_INPUT  0x00
#define LCD_DATA_OUTPUT 0xff

// DEBUG BLINING
#define LCD_GRN_BLINK  0x40  // LED1 is red
#define LCD_RED_BLINK  0x01  // LED2 is green
#define LCD_BOTH_BLINK 0x41  // use both

// STATUS READ BITS
#define LCD_BUSY  BIT7
#define LCD_ONOFF BIT5
#define LCD_RESET BIT4

// COMMANDS
#define CMD_SETX   0xB8
#define CMD_SETY   0x40
#define CMD_SETZ   0xC0
#define CMD_ON     0x3F
#define CMD_OFF    0x3E

#define F_CPU 16000000 // one cycle is 62 ns
#define lcdDelay1Ms() __delay_cycles(16130) // 1ms delay
#define lcdDelayNs(__ns) __delay_cycles( (double)__ns / 62 ) // one cycle is 62ns @ 16MHz

#define setPinLow(__pin) P2OUT &= ~__pin
#define setPinHigh(__pin) P2OUT |= __pin

volatile int  charIndex = 0;     // index for the encoder - which letter to show ATM

/* data input */
volatile char shiftReg = 0x00;   // new bits are received into here and put from the right
volatile int  shiftRegClock = 0; // counter - as soon as it gets to 8, the shift register is treated as a command value

/* management for long-time running funcs */
volatile char interrupted = 0;   // long-time runnig functions will check for this var and exit if needed
volatile int  isRunning = 0;     // long-time running function will set this flag while running

void lcdDelayMs(int ms) {
	int i;
	for (i=0; i < ms; i++)
		lcdDelay1Ms();
}

void enable() {
	lcdDelayNs(LCD_tDH);
	setPinHigh(LCD_E);
	lcdDelayNs(LCD_tDDR);
    setPinLow(LCD_E);
	lcdDelayNs(LCD_tDH);
}

void useDisplay(int dn) {
	switch(dn) {
		case 0: setPinLow(LCD_CS1); setPinHigh(LCD_CS2); break;
		case 1: setPinLow(LCD_CS2); setPinHigh(LCD_CS1); break;
		default: setPinLow(LCD_CS1); setPinLow(LCD_CS2);
	}
}

void waitReady() {
	P1OUT = 0x00;
	P1DIR = LCD_DATA_INPUT;

	setPinLow(LCD_DI);
	setPinHigh(LCD_RW);

	lcdDelayNs(LCD_tAS);

	setPinHigh(LCD_E);

	lcdDelayNs(LCD_tDDR);
	while (P1IN & LCD_BUSY) { // while display is busy
		;
	}
	setPinLow(LCD_E);
	lcdDelayNs(LCD_tDH);

	P1DIR |= 0xff;
	P1OUT = 0x00;
}

void writeCommand(char cmd) {
	waitReady();

	P1OUT = cmd;
	setPinLow(LCD_DI);
	setPinLow(LCD_RW);

	enable();
}

void goToXY(int x, int y) {
	if (x < 0 || x > 63 || y < 0 || y > 63) return;
	// x : 0 - 7 pages /8
	// y : 0 - 63 lines
	int page = x / 8;
	writeCommand(CMD_SETX | page);
	writeCommand(CMD_SETY | y);
}

void writeData(char data) {
	waitReady();

	setPinHigh(LCD_DI);
	setPinLow(LCD_RW);
	P1OUT = data;

	enable();
}

void clearScreen(int full) {
	int i,j;

	if (full == 0x01) {
		useDisplay(2);
	}

	writeCommand(CMD_SETZ | 0x00);
	writeCommand(CMD_SETX | 0x00);
	writeCommand(CMD_SETY | 0x00);

	for (i=0; i<8; i++) {
		writeCommand(CMD_SETX | i);
		writeCommand(CMD_SETY | 0);
		for (j=0; j<64; j++) {
			writeData(0x00);
		}
	}
}

void printChar(char c) {
	// find char in charSet var and print it from charMap
	int i,j;
	i=0;
	while (i < charIndexMax) {
		if (charSet[i] != c) {
			i++;
		} else break;
	}
	if (i == charIndexMax) i=31;

	for (j=0; j<8; j++) {
		writeData(charMap[i][j]);
	}
}

void manager() {

}

void demo1() { // print 64
	int x,y,z = 0;
	clearScreen(0); /* will use both displays */
	writeCommand(CMD_SETX | 0);
	writeCommand(CMD_SETY | 0);

	for (x=0; x<8; x++) {
		writeCommand(CMD_SETX | x);
		writeCommand(CMD_SETY | 0);
		for (y=0; y<8; y++)
			printChar(charSet[8*x + y]);
	}
	lcdDelayMs(500);
}

void demo2() { // filling half of display with horizontal lines
	int x,y,z = 0;
	useDisplay(0);
	while(z < 3) {
		clearScreen(0);
		writeCommand(CMD_SETX | 0);
		writeCommand(CMD_SETY | 0);

		for (x=0; x<8; x++) {
			writeCommand(CMD_SETX | x);
			for (y=0; y<64; y++) {
				
				writeData(0xFF);
			}
		}
		z++;
	}
}

void refresh() {
	int command = shiftReg;
	int shiftRegClock_ = shiftRegClock;

	if (shiftRegClock == 8) {
		shiftRegClock = 0;
	}
/* just in case */
/*	else if (shiftRegClock > 8) {
		shiftRegClock = 0;
		shiftReg = 0x00;
		return;
	}*/
/* this prints the input byte and clock counter on the first line of LCD */
	else {
		writeCommand(CMD_SETX | 0);
		writeCommand(CMD_SETY | 0);
		int mask = BIT7;
		int c;
		for (c = 0; c < 8; c++) {
			if (command & mask) {
				printChar('1');
			} else {
				printChar('0');
			}
			mask = mask >> 1;
		}
		writeCommand(CMD_SETX | 1);
		writeCommand(CMD_SETY | 0);
		switch(shiftRegClock_) {
			case 0: printChar('0'); break;
			case 1: printChar('1'); break;
			case 2: printChar('2'); break;
			case 3: printChar('3'); break;
			case 4: printChar('4'); break;
			case 5: printChar('5'); break;
			case 6: printChar('6'); break;
			case 7: printChar('7'); break;
			default : printChar('?');
		}
		return;
	}*/
	switch (command & 0x0F) {
		case 0x00: demo2(); break;
		case 0x01: demo1(); break;
		case 0x02: clearScreen(0x02); break;
		default: break;
	}
}

int main(void) {
	WDTCTL   = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL3 |= LFXT1S_2;        // Enable basic clock system control
	IFG1    &= ~OFIFG;          // clear interrupt register in case clock triggered it

	BCSCTL1 = CALBC1_16MHZ; // calibrate clocks to run @ 16 MHz
	DCOCTL  = CALDCO_16MHZ; // calibrate clocks to run @ 16 MHz

	P2SEL &= ~BIT6;
	P2SEL &= ~BIT7;

	P1DIR |= 0xff;  // everyone is output
	P2DIR =  0x3F;   // P2.0 - P2.5 are output
	P1OUT =  0x00;
	P2OUT &= 0xC0;  // reset only first six pains (output) to zero

	P2IE |= BIT7 ;  // Interrupt on Input Pin P2.6 & P2.7
	_BIS_SR(GIE);

	lcdDelayMs(50);

	useDisplay(0);
	clearScreen(0x02);
	writeCommand(CMD_SETX | 0);
	writeCommand(CMD_SETY | 0);
	writeCommand(CMD_SETZ | 0);
	writeCommand(CMD_ON);
	lcdDelayMs(50);

	useDisplay(1);
	clearScreen();
	writeCommand(CMD_SETX | 0);
	writeCommand(CMD_SETY | 0);
	writeCommand(CMD_SETZ | 0);
	writeCommand(CMD_OFF);
	lcdDelayMs(50);

	useDisplay(0);

	demo1();
	clearScreen();

	shiftReg = 55;
	shiftRegClock = 0;

	while(1) {
		lcdDelayMs(100);
		refresh();
	}

	return 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void InterruptVectorPort2() {
	if (P2IFG & BIT7) {
		P2IFG &= ~BIT7; // Clear Interrupt Flag
		shiftReg = shiftReg << 1;
		if (P2IN & BIT6) {
			blinkLED(1,0);
			shiftReg |= BIT0;
		} else {
			shiftReg &= ~BIT0;
		}
		shiftRegClock++;
	}
}
