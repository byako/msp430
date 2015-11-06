#include <msp430g2452.h>
#include <intrinsics.h>

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
#define lcdDelay1Ms() __delay_cycles(16130)                  // 1ms delay
#define lcdDelayNs(__ns) __delay_cycles( (double)__ns / 62 ) // one cycle is 62ns @ 16MHz

#define setPinLow(__pin) P2OUT &= ~__pin
#define setPinHigh(__pin) P2OUT |= __pin

/* management for long-time running funcs */
volatile int  leftCount = 0;
volatile int  rightCount = 0;
volatile int  enabled = 0;

void delayMs(int ms) {
	int i;
	for (i=0; i < ms; i++)
		lcdDelay1Ms();
}

int main(void) {
	WDTCTL   = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL3 |= LFXT1S_2;        // Enable basic clock system control
	IFG1    &= ~OFIFG;          // clear interrupt register in case clock triggered it

	BCSCTL1 = CALBC1_16MHZ;     // calibrate clocks to run @ 16 MHz
	DCOCTL  = CALDCO_16MHZ;     // calibrate clocks to run @ 16 MHz

	P2SEL &= ~BIT6;             // disable to switch XIN & XOUT from crystal mode to GPIO
	P2SEL &= ~BIT7;

	P1DIR =  0xff & ~BIT4;      // everyone is output except button S2
	P2DIR =  0xFC;              // P2.2 - P2.7 are output

	P1OUT =  0x00;
	P2OUT &= 0x00;

	P1OUT |= LED_GREEN;
	delayMs(500);
    P1OUT &= ~LED_GREEN;
    delayMs(500);
	P1OUT |= LED_GREEN;
	delayMs(500);
    P1OUT &= ~LED_GREEN;
    delayMs(500);
	P1OUT |= LED_RED;
	delayMs(500);
    P1OUT &= ~LED_RED;
    delayMs(500);

	P2IE |= BIT0 ;   // Interrupt on Input Pin P2.0 & P2.1
	P2IE |= BIT1 ;   // Interrupt on Input Pin P2.0 & P2.1
	P1IE |= BIT4 ;   // interrupt on button press
	_BIS_SR(GIE);

	P2OUT = BIT2;
	return 0;
}

#pragma vector = PORT2_VECTOR
__interrupt void InterruptVectorPort2() {
	if (P2IFG & BIT0) { /* if it was pin#0 who interrupted */
		P2IFG &= ~BIT0; /* Clear Interrupt Flag */
        if (P1OUT & LED_GREEN) {
			P1OUT &= ~LED_GREEN;
		} else {
			P1OUT |= LED_GREEN;
		}
	} else { /* it was BIT1 */
		P2IFG &= ~BIT1;
        if (P1OUT & LED_RED) {
			P1OUT &= ~LED_RED;
		} else {
			P1OUT |= LED_RED;
		}
	}
}

#pragma vector = PORT1_VECTOR
__interrupt void InterruptVectorPort1() {
	if (P1IFG & BIT4) { /* if it was pin#0 who interrupted */
		P1IFG &= ~BIT4; /* Clear Interrupt Flag */
		if (enabled) {
			enabled = 0;
			P2OUT &= ~BIT2;
			P1OUT &= ~LED_GREEN;
		} else {
			enabled = 1;
			P2OUT |= BIT2;
			P1OUT |= LED_GREEN;
		}
	} else {
		P1OUT &= LED_RED;
	}
}
