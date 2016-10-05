#include "msp430g2553.h"

/*****************************************************************************
  P1.1 - LED 1
  P1.6 - LED 2
  P2.1 - INPUT LEFT
  P2.2 - INPUT RIGHT
  P2.3 - INPUT BUTTON
******************************************************************************/


// P2IN READ BITS
#define ROT_LEFT   BIT0;
#define ROT_RIGHT  BIT1;
#define ROT_ENABLE BIT2; // output pin serving +3v

// ROTARY-SPECIFIC DATA
#define CIRCLE_PULSES 24

// DEBUG BLINING
#define LED_RED     BIT0         // LED1 is red
#define LED_GREEN   BIT6         // LED2 is green
#define LED_BOTH    BIT0 | BIT6


#define F_CPU 16000000                                       // CPU frequency, one cycle is 62 ns
#define lcdDelay1Ms() __delay_cycles(1008)                   // 1ms delay
#define lcdDelayNs(__ns) __delay_cycles( (double)__ns / 62 ) // one cycle is 62ns @ 16MHz

#define setPinLow(__pin) P2OUT &= ~__pin
#define setPinHigh(__pin) P2OUT |= __pin

/* management for long-time running funcs */
volatile int leftCount = 0;
volatile int rightCount = 0;
volatile int enabled = 0;
volatile int dir = 0; /* -1 / +1 */
volatile int prevPos = 0x00;
volatile int delayValue = 50;

void delayMs(int ms) {
	int i;
	for (i=0; i < ms; i++)
		lcdDelay1Ms();
}

int main(void) {
	WDTCTL   = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL3 |= LFXT1S_2;        // Enable basic clock system control
	IFG1    &= ~OFIFG;          // clear interrupt register in case clock triggered it

	BCSCTL1 = CALBC1_1MHZ;     // calibrate clocks to run @ 16 MHz
	DCOCTL  = CALDCO_1MHZ;     // calibrate clocks to run @ 16 MHz

	P2SEL &= ~BIT6;             // disable to switch XIN & XOUT from crystal mode to GPIO
	P2SEL &= ~BIT7;

	P1DIR =  0xff & ~BIT4;      // everyone is output except button S2
	P2DIR =  0xFC;              // P2.2 - P2.7 are output

	P1OUT =  0x00;
	P2OUT &= 0x00;

	P1OUT |= LED_GREEN;
	delayMs(50);
    P1OUT &= ~LED_GREEN;
    delayMs(50);
	P1OUT |= LED_GREEN;
	delayMs(50);
    P1OUT &= ~LED_GREEN;
    delayMs(50);
	P1OUT |= LED_RED;
	delayMs(50);
    P1OUT &= ~LED_RED;
    delayMs(50);

	P2IE |= BIT0 ;   // Interrupt on Input Pin P2.0 & P2.1
	P2IE |= BIT1 ;   // Interrupt on Input Pin P2.0 & P2.1
	P1IE |= BIT4 ;   // interrupt on button press
	_BIS_SR(GIE);

    while (1) {
        if (delayValue > 0) {
            P1OUT ^= LED_GREEN;
            delayMs(delayValue);
        } else {
            P1OUT ^= LED_RED;
            delayMs(-delayValue);
        }
    }

	return 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void InterruptVectorPort2() {
    int curPos = 0x00;

	if (P2IFG & BIT0) { /* if it was pin#0 who interrupted */
		P2IFG &= ~BIT0; /* Clear Interrupt Flag */
	} else { /* it was BIT1 */
		P2IFG &= ~BIT1;
	}

    if (P2IN & BIT0)
        curPos |= 0x01;
    if (P2IN & BIT1)
        curPos |= 0x10;

    if (curPos == 0x00) { // both active low -> calculate direction
        if (prevPos == 0x10) {
            delayValue -= 50;
        }
        else if (prevPos == 0x01) {
            delayValue += 50;
        }
    }
    else if (curPos == 0x01 || curPos == 0x10) { // save position
        prevPos = curPos;
    }
    else { // both active high - end of movement
        prevPos = 0x00;
    }
}
