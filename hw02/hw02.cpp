// HW01 Solution used as starting point for my HW02.
// Assigned Dip pin# 27
// port 0, pin 11

#include <stdint.h>
#include "mbed.h"

#define BUFLEN 1024
unsigned char buffer[BUFLEN];
const unsigned int lastNdx = BUFLEN - 1;

#define MYPORTPINVALUES LPC_GPIO0->FIOPIN
#define MYPORTPIN 11
#define MAXREGSPERPASS 8

#define WAITTIMEINuS 3
#define SAMPLEINTOREG( reg) reg = MYPORTPINVALUES; wait_us( WAITTIMEINuS)

// Initial input freq. = 4MHz = 4 * 1024 * 1024.
// 50% duty-cycle, = 0.25 uS.
// 
// mbed clock = 96 * 1024 * 1024 = 9.93 nS.
//
// So we can see the full down-up-down-up cycle, 
// want our 8 samples to take >= 0.25 uS,
// i.e., 31.3 nS each.
//
// So to get our 8 samples spread across one 4MHz up+down, 
// we need to add a delay of 21.37 nS in beween each sample.
//
// Since there is no wait_ns() function, only wait_us(), I'll 
// have to settle for fewer data points per measured signal cycle,
// or decrease my input signal frequency by a factor of 50, 
// i.e., to 80 kHz. 
// My Signal Generator in my DSO QUAD 'scope can only output 
// 50 or 100 kHz, so I'll set it 50 to 50kHz.
//
// Re-starting the above, 50 kHz = 20 uS.
// And to cover a bit more than one clock cycle of the input signal,
// I want the 8 samples to take about 25 uS, or ~3.125 uS each.
//
// Meaning that I want to delay 3 uS in between samples, as at this 
// slow speed, the extra time of the actual sampling instructions 
// execution is negligible.
//
void sample( unsigned char *buf)
{
    register unsigned char reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8;

    SAMPLEINTOREG( reg1);
    SAMPLEINTOREG( reg2);
    SAMPLEINTOREG( reg3);
    SAMPLEINTOREG( reg4);
    SAMPLEINTOREG( reg5);
    SAMPLEINTOREG( reg6);
    SAMPLEINTOREG( reg7);
    SAMPLEINTOREG( reg8);
 
    buf[0] = reg1;
    buf[1] = reg2;
    buf[2] = reg3;
    buf[3] = reg4;
    buf[4] = reg5;
    buf[5] = reg6;
    buf[6] = reg7;
    buf[7] = reg8;
 
    return;
}

const unsigned int myPortPinMask = 1 << MYPORTPIN;

void analyze( unsigned char *buf, unsigned char myPortPin)
{    
    for (int i = 0 ; i < MAXREGSPERPASS ; i++)
    {
        printf( (buf[i] & myPortPinMask) ? "1 " : "0 ");
    }
    printf( "\r\n");
}

int main( void)
{ 
    // int c;
    // Serial pc( USBTX, USBRX); // tx, rx 
    // pc.baud( 9600);          //or pc.baud( 115200);

    printf( "\r\nHello World!\r\n"); 

    unsigned char *p = buffer;
 
    while (false && p < &(buffer[lastNdx]))
    {
        sample( p);
        p += 8;
    }
    
    analyze( buffer, MYPORTPIN);
 
    return 0;
}