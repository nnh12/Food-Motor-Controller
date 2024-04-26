
#include <msp430.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

char DISTANCE[] = "Current Distance: ";    // Define the character array for Serial Output
char cm[] = " cm";                         // create cm output
char d[] = ".";                            // Define character array for dot for Serial Output
char z[] = "0";                            // Define character array for zero
volatile int hold[2];                       // Array to hold the previous distance value

char nextline[] = " \r\n";

volatile float difference;                  // A variable  to get the difference 
int current_distance;
int dst_flt;

char dst_char[5];
char dst_flt_char[5];
volatile float distance;
volatile unsigned int i=0;

void ser_output(char *str){
    while(*str != 0) {
        while (!(IFG2&UCA0TXIFG));
        UCA0TXBUF = *str++;
        }
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void Timer_A(void){
        hold[i] = TA1CCR1;
        i += 1;
        TA1CCTL1 &= ~CCIFG ;
        if (i==2) {
            difference=hold[i-1]-hold[i-2];
            i=0;
        }

}

// Enable Serial communication via UART for testing purposes
void inititalize_UART() {
    UCA0CTL1 |= UCSWRST+UCSSEL_2;
    UCA0BR0 = 52;
    UCA0BR1 = 0;
    UCA0MCTL = UCBRS_0;
    UCA0CTL1 &= ~UCSWRST;
    TA0CTL = TASSEL_2|MC_1 ;
}


void init_ultrasound() {
    P1DIR |= BIT6 | BIT0;
    P2SEL = BIT1;
    P1SEL = BIT1|BIT2|BIT6;
    P1SEL2 = BIT1|BIT2;
}

// Inititlzie the duty cycle of the timer
void init_timer() {
    TA0CCR0 = 0xFFFF;
    TA0CCR1 = 0x000A;
    TA0CCTL1 = OUTMOD_7;

    TA1CTL = TASSEL_2|MC_2 ;
    TA1CCTL1 = CAP | CCIE | CCIS_0 | CM_3 | SCS ;
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    BCSCTL1= CALBC1_1MHZ;       // Inititalize clock frequnecy
    DCOCTL = CALDCO_1MHZ;       // Inititalize clock frequency

    inititalize_UART();         // Initialize the UART communication
    init_ultrasound();          // Initialize the ultrasound
    init_timer();               // Initialize the timer

    _enable_interrupts();       //Enable Interrupts
    
    while(1){
        current_distance = floor(difference/58);  //distance the sensor is detecting

        ltoa(current_distance, dst_char, 10); //converts current_distance, which is a long, into a string

        // If the current distance is greater than 10, then we set one of the GPIO pins to HIGH
        if (current_distance > 10) {
            P1OUT |= BIT0; //sets pin 1.0 to high
        }

        else {
            P1OUT &= ~BIT0; //sets pin 1.0 to low
        }

        ltoa(dst_flt,dst_flt_char, 10);  //converts dst_flt into char

        // Output the distance via UART for testing
        ser_output(DISTANCE); //will print "Current Distance:"
        ser_output(dst_char);  //prints the current distance being detected
        ser_output(d);        //prints a .
        ser_output(z);        //prints a 0
        ser_output(z);        //prints a 0
        ser_output(dst_flt_char); ser_output(cm);  //output message will look like
                                                   // Current Distance: X.00 cm
 
        ser_output(nextline);     //moves to the next line in the terminal
        __delay_cycles(500000);   // Enable the low power mode
    }
}

