#include <msp430g2553.h>
#include <intrinsics.h>

volatile int delayVar;
/*****************************************************************************

 To communicate with LCD we need 8 data pins and 4 command pins out of 16 av.
 DB0 - DB7 : p1.0 - p1.7  // data pins
 RS        : p2.0         // L - reset / H - working
 R/W       : p2.1         // H - read / L - write
 CS1       : p2.2         // Select column 1  ~ 64
 CS2       : p2.3         // Select column 65 ~ 128
 D/I       : p2.4         // H - data / L - instruction
 E         : p2.5         // H - enable signal

 GPIO registers contents:
  P1OUT = [ [DB7] [DB6] [DB5] [DB4] [DB3] [DB2] [DB1] [DB0] ]
  P2OUT = [ [ - ] [ - ] [E  ] [D/I] [CS2] [CS1] [R/W] [RS ] ]

******************************************************************************/

// P2OUT WRITE BITS
#define LCD_RS  BIT0;
#define LCD_RW  BIT1;
#define LCD_CS1 BIT2;
#define LCD_CS2 BIT3;
#define LCD_DI  BIT4;
#define LCD_E   BIT5;

// DELAYS in Nano Seconds
#define LCD_tAS    124     /* Address setup time (ctrl line changes to E HIGH    */
#define LCD_tDDR   186     /* Data setup time (data lines setup to dropping E)   */
#define LCD_tDH    124     /* common data hold delay after E drop down           */

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

// we have only 512 bytes RAM, but it's enough to keep chars and print them in
// loop, 16k might be enough to keep vocabulary

// duty cycle order:
// 0. E is set to 0
// 1. R/W , RST , CS1 , CS2 are set
// 2. E is set to 1
// 3. DB0 - DB7 are set
// 4. E is set to 0
// 

#define setPinLow(__pin) P2OUT &= ~__pin
#define setPinHigh(__pin) P2OUT |= __pin

void clearRegisters() {
	P1OUT = 0x00;
	P2OUT &= 0xC0;
}

void enable() {
	lcdDelayNs(LCD_tAS);
	setPinHigh(LCD_E);
	lcdDelayNs(LCD_tDDR);
    setPinLow(LCD_E);
	lcdDelayNs(LCD_tDH);
//	clearRegisters();
}

void lcdDelayMs(int ms) {
	int i;
	for (i=0; i < ms; i++)
		lcdDelay1Ms();
}

void blinkLED(int count, char led1) {
	int delay = 50;
	char saveP1DIR = P1DIR;

	if (led1 != LCD_RED_BLINK && led1 != LCD_GRN_BLINK && led1 != LCD_BOTH_BLINK) {
		led1 = LCD_RED_BLINK;
	}

	P1DIR |= led1; // set output direction for led used
	int i;
	for (i=0; i < count; i++) {
		P1OUT |= led1;
		lcdDelayMs(delay);
		P1OUT &= ~led1;
		lcdDelayMs(delay);
	}
	P1DIR = saveP1DIR;
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

int isDisplayEnabled(int displayNumber) { // 1 = enabled , 0 = disabled
	useDisplay(displayNumber);
	int ret = 0;
	P1OUT = 0x00;

	P1DIR = LCD_DATA_INPUT;
	setPinLow(LCD_DI);
	setPinHigh(LCD_RW);

	lcdDelayNs(LCD_tAS);
	setPinHigh(LCD_E);
	lcdDelayNs(LCD_tDDR);
	if (P1IN & LCD_ONOFF) { // if LCD_ONOFF == 1 , it's disabled
		ret = 0;
	} else {
		ret = 1;
	}
	setPinLow(LCD_E);
	lcdDelayNs(LCD_tDH);

	P1DIR |= 0xff;
	P1OUT = 0x00;
	return ret;
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

void clearScreen() {
	int i,j;

	writeCommand(CMD_SETZ | 0x00);
	writeCommand(CMD_SETX | 0x00);
	writeCommand(CMD_SETY | 0x00);

	for (i=0; i<8; i++) {
		writeCommand(CMD_SETX | i);
		for (j=0; j<8; j++) {
			writeData(0);
		}
	}
}

int main(void) {
	WDTCTL   = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL3 |= LFXT1S_2;        // Enable basic clock system control
	IFG1    &= ~OFIFG;          // clear interrupt register in case clock triggered it

	BCSCTL1 = CALBC1_16MHZ; // calibrate clocks to run @ 16 MHz
	DCOCTL  = CALDCO_16MHZ; // calibrate clocks to run @ 16 MHz

	P1DIR |= 0xff;  // everyone is output
	P2DIR |= 0x1f;  // P2.0 - P2.5 are output
	P1OUT = 0x00;
	P2OUT &= 0xC0;  // reset only first five pins (output) to zero

/*	if (isDisplayEnabled(1) == 0) { // if it's off
		blinkLED(5,LCD_GRN_BLINK);
		writeCommand(CMD_ON);
	} else { // if it's on
		blinkLED(5,LCD_RED_BLINK);
		writeCommand(CMD_OFF);
	}
	return 0;*/
	
	lcdDelayMs(200);

	useDisplay(0);
	writeCommand(CMD_ON);

	blinkLED(5, LCD_GRN_BLINK);
	lcdDelayMs(500);

	useDisplay(1);
	writeCommand(CMD_OFF);
	return 0;

	clearScreen();

	writeCommand(CMD_SETZ | 0x00);
	writeCommand(CMD_SETX | 0x20);
	writeCommand(CMD_SETY | 0x08);

	blinkLED(3,LCD_RED_BLINK);

	int y;
	for (y = 0; y < 8; y++) writeData(0xFF);

	blinkLED(3,LCD_GRN_BLINK);
	return 0;
}
