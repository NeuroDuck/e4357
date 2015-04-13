#include "gpio.h"

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
	
	if (outputNotInput != 0)			// We want it to be 1.
		LPC_GPIO0->FIODIR |= dirMask;
	else								// We want it to be 0.
		LPC_GPIO0->FIODIR &= ~dirMask;

	// 10 = neither pull-up nor pull-down resistor are enabled.
	// Default = pull-up resiter is enabled.
	// Just ignore this for now.
//	uint32_t* pullCfg = 
//		(uint32_t*)(FIO_MODE_BASE + myPort * sizeof( uint8_t) * 0x20);

	// 0 = default, normal (not open drain) mode.
	// Just ignore this for now.
//	uint32_t* odCfg = 
//		(uint32_t*)(FIO_MODE_OD_BASE + myPort * sizeof( uint8_t) * 0x20);
}

void setGPIO( uint8_t myPort, uint8_t myPin, uint8_t highNotLow)
{
	// See Section 9.5.4, pg. 136-137.
	// 00 = default, bits not masked out by FIOMASK are cleared.
	//
	uint32_t pinMask = 1 << myPin;
	
	if (highNotLow != 0)				// We want it to be 1.
		LPC_GPIO0->FIOPIN |= pinMask;
	else								// We want it to be 0.
		LPC_GPIO0->FIOPIN &= ~pinMask;
}
