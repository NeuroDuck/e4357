 #include "gpio.h"

int main()
{
	uint32_t gpio_status = 0;

 	/* Soft Reset Register */
	// Ignore this for now.
// 	uint32_t * SYSREG_SOFT_RST_CR = (uint32_t *)(0xE0042030);
	
 	/* Reset GPIO hardware */
	// Ignore this for now.
// 	*SYSREG_SOFT_RST_CR |= 0x00004000;
	
 	/* Take GPIO hardware out of reset */
	// Ignore this for now.
//  	*SYSREG_SOFT_RST_CR &= ~(0x00004000);
	
	// p27 = P0[11];
	//
	uint32_t myPort = 0;
	uint32_t myPin = 11;
	
	initGPIO( myPort, myPin, 1);	// 1 == We want it to be Output.
	
	uint8_t newPinValue = 0;
	setGPIO( myPort, myPin, newPinValue);
	
	while (1)
	{
		newPinValue = 1 - newPinValue;
		
		setGPIO( myPort, myPin, newPinValue);
	}
	
	return 0;
}
