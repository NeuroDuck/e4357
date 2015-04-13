#include "gpio.h"

int main()
{
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
