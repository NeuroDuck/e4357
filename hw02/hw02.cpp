// HW01
// Assigned Dip pin# 27
// port 0, pin 11
// 
// To access it, we'll use FIOPIN1, bit 2, so less "and"'ing needs to be done.

#include <stdint.h>
#include "mbed.h"

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
#define WAITTIMEINuS 3
#define SAMPLEINTOREG( reg) reg = MYPORTPINVALUES; wait_us( WAITTIMEINuS)

#define MYPORTNUM 0
#define MYPORTPIN 11
#define MYPORTPINFP1 2

// My Pin above is in FIOPIN1, bit 2.
//
#define MYPORTPINVALUES LPC_GPIO0->FIOPIN1

const uint8_t myFP1PortPinMask = 1 << MYPORTPINFP1;

void sample( uint8_t *buf)
{
    register uint8_t reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8;

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

// This only works for GPIO Pins in Port #0 for now.
// Later I can extend it to work for the other Ports as well.
//
void initGPIO( uint8_t myPort, uint8_t myPin, uint8_t outputNotInput)
{
    // Pin Function Selection is in User's Manual, 
    // starting in Section 8.1, pg. 113.
    
    // My Pin is P0[11], aka p27 on the LPC1768 mbed Development Board.
    //
    // Set our pin to be "GPIO Port 0.11", which is what the User's 
    // Manual calls it.
    
    // LPC_PINCON = 0x 400 2 C000
    // PINSEL0 is the first uint32_t inside the LPC_PINCON struct.
    
    // 00 = GPIO Port 0.11.  This is the value that we always want.
    // Each Pin gets 2 bits, so we multiply our shift value * 2.
    // 
    // We want our bits to be 00, so we'll "&" them with 00, 
    // and "&" all the rest with 11, to leave those other 
    // bits unchanged.
    //
    uint32_t funcMask = 3 << myPin * 2;
    LPC_PINCON->PINSEL0 &= ~funcMask;
    
    // GPIO Register descriptions start in the User's Manual
    // Section 9.5, pg. 131.
    
    // Let's set our Pin's direction.
    //
    // LPC_GPIO[n] = LPC_GPIO_BASE + n * 0x20;
    //
    // To specify direction, we can use any of:
    // LPC_GPIO0->
    //   FIODIR (32-bits), FIODIRL (16-bits), FIODIRH (16-bits),
    //   FIODIR0 (8-bits), FIODIR1 (8-bits), 
    //   FIODIR2 (8-bits), FIODIR3 (8-bits)
    //
    // Ditto for FIOMASK, FIOPIN, FIOSET, FIOCLR.
    
    // See Section 9.5.5, pg. 138.
    // We want our Pin's Mask value to become 0, 
    // and to leave all the other Mask bits alone,
    // So we "&" our bit with 0, and "&" all the 
    // other bits with 1.
    //
    uint32_t maskMask = 1 << myPin;
    LPC_GPIO0->FIOMASK &= ~maskMask;
    
    // See Section 9.5.1, pg. 132.
    // Finally, let's set our Pin's direction.
    //
    uint32_t dirMask = 1 << myPin;
    
    if (outputNotInput != 0)            // We want it to be 1.
        LPC_GPIO0->FIODIR |= dirMask;
    else                                // We want it to be 0.
        LPC_GPIO0->FIODIR &= ~dirMask;

    // 10 = neither pull-up nor pull-down resistor are enabled.
    // Default = pull-up resiter is enabled.
    // Just ignore this for now.
//  uint32_t* pullCfg = 
//      (uint32_t*)(FIO_MODE_BASE + myPort * sizeof( uint8_t) * 0x20);

    // 0 = default, normal (not open drain) mode.
    // Just ignore this for now.
//  uint32_t* odCfg = 
//      (uint32_t*)(FIO_MODE_OD_BASE + myPort * sizeof( uint8_t) * 0x20);
}

void printBin( uint8_t value)
{
    uint8_t mask = 1 << 7;
    
    for (int i = 7 ; i >= 0 ; i--)
    {
        printf( (value & mask) ? "1" : "0");
        
        mask >>= 1;
        
        if (i % 4 == 0)
            printf( " ");
    }
}

#define MAXREGSPERPASS 8

void analyze( uint8_t *buf)
{    
    for (int i = 0 ; i < MAXREGSPERPASS ; i++)
    {
        printf( "%x: ", buf[i]);
        printBin( buf[i]);
        printf( ": %s\r\n", (buf[i] & myFP1PortPinMask) ? "1 " : "0 ");
    }
    printf( "\r\n");
}

#define BUFLEN 1024
uint8_t buffer[BUFLEN];
const unsigned int lastNdx = BUFLEN - 1;

int main( void)
{ 
    // int c;
    // Serial pc( USBTX, USBRX); // tx, rx 
    // pc.baud( 9600);          //or pc.baud( 115200);

    printf( "\r\nHello World!\r\n"); 
    
   uint32_t myPort = MYPORTNUM;
   uint32_t myPin  = MYPORTPIN;
   initGPIO( myPort, myPin, 0);    // 0 == We want it to be Input.

    uint8_t *p = buffer;
 
    while (p < &(buffer[lastNdx]))
    {
        sample( p);
        
        analyze( p);
        
        p += 8;
        
//        break; 
    }
    
 
    return 0;
}
