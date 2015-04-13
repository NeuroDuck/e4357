#include "gpio.h"

uint32_t* GPIO_OUT = (uint32_t*)GPIO_OUT_BASE;

void initGPIO( uint8_t myPort, uint8_t myPin, uint8_t outputNotInput)
{
	// 00 = default, usually GPIO.
	uint32_t* selCfg = (uint32_t*)(FIO_SEL_BASE + myPort * sizeof( uint32_t));

	// 00 = GPIO Port 0.11.  This is the value that we always want.
	uint32_t cfgMask = 3 << myPin * 2;
	*selCfg &= ~cfgMask;
	
	// FIODIR.
	// 0 = input, 1 = output.
	uint32_t* dirCfg = 
		(uint32_t*)(FIO_DIR_BASE + myPort * sizeof( uint8_t) * 0x20);

	uint32_t dirMask = 1 << myPin;
	
	if (outputNotInput != 0)	// We want it to be 1.
		*dirCfg |= dirMask;
	else						// We want it to be 0.
		*dirCfg &= ~dirMask;

	// 10 = neither pull-up nor pull-down resistor are enabled.
	// Default = pull-up resiter is enabled.
	// Just ignore this for now.
	uint32_t* pullCfg = 
		(uint32_t*)(FIO_MODE_BASE + myPort * sizeof( uint8_t) * 0x20);

	// 0 = default, normal (not open drain) mode.
	// Just ignore this for now.
	uint32_t* odCfg = 
		(uint32_t*)(FIO_MODE_OD_BASE + myPort * sizeof( uint8_t) * 0x20);
}

void setGPIO( uint8_t myPort, uint8_t myPin, uint8_t highNotLow)
{
	// FIOMASK
	// 0 = default, no bits are masked out.
	uint32_t* maskCfg = 
		(uint32_t*)(FIO_MASK_BASE + myPort * sizeof( uint8_t) * 0x20);

	uint32_t maskMask = ~(1 << myPin);
	
	*maskCfg = maskMask;

	// FIOPIN.
	// 00 = default, bits not masked out by FIOMASK are cleared.
	uint32_t* pinSet = 
		(uint32_t*)(FIO_PIN_BASE + myPort * sizeof( uint8_t) * 0x20);

	uint32_t setMask = 1 << myPin;

	*pinSet = setMask;
}
