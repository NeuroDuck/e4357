#ifndef GPIO_H
#define GPIO_H
#include <inttypes.h>
#include <LPC17xx.h>

void initGPIO( uint8_t myPort, uint8_t myPin, uint8_t outputNotInput);
void setGPIO( uint8_t myPort, uint8_t myPin, uint8_t highNotLow);

#endif //GPIO_H
