#include<msp430g2553.h>

unsigned char rxvd_data = 0;                  // Byte received via UART

int main(void)
{

    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    /*** CONFIGURATION FOR BLUETOOTH SETUP ***/


    // Setting up the internal oscillator for UART and all peripherals
    DCOCTL = 0;                             // Select lowest DCOx and MODx settings
    BCSCTL1 = CALBC1_1MHZ;                  // Set 1 MHz
    DCOCTL = CALDCO_1MHZ;                   // Set DCO

    // Setting up the PINS
    P1SEL = BIT1 + BIT2;                    // P1.1 = RXD, P1.2=TXD
    P1SEL2 = BIT1 + BIT2;                   // P1.1 = RXD, P1.2=TXD
    P2DIR |= BIT3 + BIT4;                   // P2.3 & P2.4 are set as output (Enable pins used on the motor driver)
    P2OUT &= ~(BIT3 + BIT4);                // P2.3 & P2.4 are set low

    // Setting up Universal Serial Communication Interface ( USCI A )
    UCA0CTL1 |= UCSSEL_2;                   // Select SMCLK as the clock source
    UCA0BR0 = 104;                          // Selecting Baud Rate 1MHz 9600
    UCA0BR1 = 0;                            // Selecting Baud Rate 1MHz 9600
    UCA0MCTL = UCBRS0;                      // Modulation UCBRSx = 1
    UCA0CTL1 &= ~UCSWRST;                   // Initialize UART Model
    IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt
    _enable_interrupts();                   // Enable Interrupts

    // Configuring Motor Pins
    P1DIR |= ( BIT4 | BIT5);             // Left Motor
    P2DIR |= ( BIT1 | BIT2 | BIT3 | BIT4);             // Right Motor + Enable Pins

    __bis_SR_register(LPM0_bits);  // Enter LPM0, interrupts enabled

    }



// Timer Interrupt Handler
#pragma vector = TIMER0_A1_VECTOR
 __interrupt void TimerA(void)
 {
    switch(TAIV)
    {
        case 0x002: // CCR1 interrupt
                    P2OUT |=  BIT3;    // Left Motor is started
                    P2OUT |=  BIT4;    // Right Motor is started
                    break;


        case 0x00A: // TAR overflow interrupt
                     P2OUT &= ~BIT3;     // Left Motor is stopped
                     P2OUT &= ~BIT4;     // Right Motor is stopped
                     break;
     }
  }



 //  USCI A interrupt handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCIAB0RX_VECTOR))) USCI0RX_ISR (void)
#else
#error Compiler not supported!
#endif
{
    rxvd_data = UCA0RXBUF;                    // The received byte is stored in Rx_Data
  // Based on byte received operation to be executed

                if(rxvd_data == 'F')                          // Forward Motion
                    {
                    P2OUT |= BIT3; // Enable pins
                    P2OUT |= BIT4;
                    P1OUT &=~BIT4; //Left forward
                    P1OUT |= BIT5;
                    P2OUT &=~BIT1;  // Right Forward
                    P2OUT |= BIT2;
                    }

                 else if(rxvd_data == 'B')                          // Backward Motion
                    {
                    P2OUT |= BIT3;
                    P2OUT |= BIT4;
                    P1OUT &=~BIT5; //Left Reverse
                    P1OUT |= BIT4;
                    P2OUT &=~BIT2;  //Right Reverse
                    P2OUT |= BIT1;
                    }

                else if(rxvd_data =='L')                          // Left Motion
                    {
                    P2OUT |= BIT3;
                    P2OUT |= BIT4;
                    P1OUT &=~BIT4; //Left STOP
                    P1OUT &=~BIT5;
                    P2OUT &=~BIT1;  // Right Forward
                    P2OUT |= BIT2;
                    }

                else if(rxvd_data =='R')                         // Right Motion
                    {
                    P2OUT |= BIT3;
                    P2OUT |= BIT4;
                    P1OUT &=~BIT4; //Left forward
                    P1OUT |= BIT5;
                    P2OUT&=~BIT2;  // Right stop
                    P2OUT&=~BIT1;
                    }

                else if(rxvd_data =='G')                          // Forward Motion with PWM Modification
                    {
                    // Timer 0 Configuration
                   TACTL |= MC_0;                   // Stop Timer0_A3
                   TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                   CCTL1  = CCIE;                   // Enable interrupt for CCR1
                   CCR1   = 40000;                // Load value
                   TACTL |= MC_2;                   // Start Timer0 in Continous Mode

                   P1OUT &=~BIT4; //Left forward
                   P1OUT |= BIT5;
                   P2OUT &=~BIT1;  // Right Forward
                   P2OUT |= BIT2;
                   }

                else if(rxvd_data =='I')                          // Forward Motion with PWM Modification-2
                    {
                    // Timer 0 Configuration
                    TACTL |= MC_0;                   // Stop Timer0_A3
                    TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
                    CCR1   = 50000;                // Load value
                    TACTL |= MC_2;

                    P1OUT &=~BIT4; //Left forward
                    P1OUT |= BIT5;
                    P2OUT &=~BIT1;  // Right Forward
                    P2OUT |= BIT2;
                   }

                else if(rxvd_data =='H')                          // Backward Motion with PWM Modification
                    {
                    // Timer 0 Configuration
                    TACTL |= MC_0;                   // Stop Timer0_A3
                    TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
                    CCR1   = 40000;                // Load value
                    TACTL |= MC_2;                   // Start Timer0 in Continuous Mode

                    P1OUT &=~BIT5; //Left Reverse
                    P1OUT |= BIT4;
                    P2OUT &=~BIT2;  //Right Reverse
                    P2OUT |= BIT1;
                   }

                else if(rxvd_data =='J')                          // Backward Motion with PWM Modification-2
                    {
                    // Timer 0 Configuration
                    TACTL |= MC_0;                   // Stop Timer0_A3
                    TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
                    CCR1   = 50000;                // Load value
                    TACTL |= MC_2;

                    P1OUT &=~BIT5; //Left Reverse
                    P1OUT |= BIT4;
                    P2OUT &=~BIT2;  //Right Reverse
                    P2OUT |= BIT1;
                   }

                else if(rxvd_data =='S')                       // Stop
                    {
                    // Timer 0 Configuration
                    TACTL |= MC_0;                   // Stop Timer0_A3
                    TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
                    CCR1   = 6000;                // Load value
                    TACTL |= MC_2;                   // Start Timer0 in Continous Mode

                    P1OUT &=~BIT4; //undo backward
                    P1OUT &=~BIT5; //undo forward
                    P2OUT&=~BIT2;  // undo left
                    P2OUT&=~BIT1;  // undo right
                   }

                else                      // Stop
                    {
                    /* Timer 0 Configuration
                    TACTL |= MC_0;                   // Stop Timer0_A3
                    TACTL  = TASSEL_2 + TAIE;        // Choose Clock Source SMCLK,Enable Timer Interrupt
                    CCTL1  = CCIE;                   // Enable interrupt for CCR1
                    CCR1   = 6000;                // Load value
                    TACTL |= MC_2;                   // Start Timer0 in Continous Mode
*/
                    P1OUT &=~BIT4; //undo backward
                    P1OUT &=~BIT5; //undo forward
                    P2OUT&=~BIT2;  // undo left
                    P2OUT&=~BIT1;  // undo right
                        }
}
