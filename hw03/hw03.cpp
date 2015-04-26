/*
 * Program Example 7.9: 
 * Sets the mbed up for async communication, and exchanges data with
 * a similar node, sending its own switch positions, and displaying 
 * those of the other.
*/
#include "mbed.h"

Serial async_port( p9, p10);    // Set up TX and RX on pins 9 and 10.

DigitalOut red_led( p25);       // Red LED.
DigitalOut green_led( p26);     // Green LED.
DigitalOut strobe( p7);         // A strobe to trigger the scope.

DigitalIn switch_ip5_red( p5);
DigitalIn switch_ip6_green( p6);

uint8_t switch_word;               // The word we will send.
uint8_t recd_val;                  // The received value.

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

#define PIN5BITRED 0x02
#define PIN6BITGREEN 0x01
#define SPACE_CHAR ' '

int main() 
{
    // Accept default format, of 8 bits, no parity;
    async_port.baud( 9600);     // Set baud rate to 9600 (ie default).
    
    while (1)
    {
        //Set up the word to be sent, by testing switch inputs.
                                   // Alternate between '!', '"', '#'.
        switch_word = SPACE_CHAR;  // Set up a recognizable output pattern.

        if (switch_ip5_red == 1)
            switch_word |= PIN5BITRED;
            
        if (switch_ip6_green == 1)
            switch_word |= PIN6BITGREEN;
        
        // Cycling the switch_word for testing.
        // switch_word = (switch_word + 1) % 4;
            
        // Print what we're sending.  Compare this to 
        // what the other MPU prints for what it's receiving.
        printf( "%x: %c: ", switch_word, switch_word);
        printf( "%x: ", switch_word);
        printBin( switch_word);
        printf( "    ");
    
        // My DSO Quad O'scope doesn't have an External Trigger Input.
        // strobe = 1;                 // short strobe pulse.

        wait_ms( 500);
        // strobe = 0;
        
        async_port.putc( switch_word);          // transmit switch_word.
        
        recd_val = SPACE_CHAR;
        
        if (async_port.readable() == 1)         // Is there a character to be read?
            recd_val = async_port.getc();         // If yes, then read it.
        
        // Print what we're receiving.
        // Compare this to what the other MCU prints out for what it's sending. 
        printf( "%x: %c: ", recd_val, recd_val);
        printBin( recd_val);
        printf( "\r\n");

        // Set leds according to the incoming word from the other MCU.
        //
        red_led = 0;                            // Preset both to 0.
        green_led = 0;
        recd_val = recd_val & 0x03;             // AND out unwanted bits.

        // Set our lights accordingly.
        if (recd_val & PIN5BITRED)
            red_led = 1;

        if (recd_val & PIN6BITGREEN)
            green_led = 1;
    }
}
