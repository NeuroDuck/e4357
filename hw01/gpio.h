#ifndef GPIO_H
#define GPIO_H
#include <inttypes.h>

#define GPIO_CFG_BASE 0x40013000
#define GPIO_OUT_BASE 0x40013088

#define FIO_SEL_BASE      0x4002C000

#define FIO_DIR_BASE      0x2009C000
#define FIO_MASK_BASE     0x2009C010
#define FIO_PIN_BASE      0x2009C014

#define FIO_MODE_BASE     0x4002C040
#define FIO_MODE_OD_BASE  0x4002C068

void initGPIO( uint8_t myPort, uint8_t myPin, uint8_t outputNotInput);
void setGPIO( uint8_t myPort, uint8_t myPin, uint8_t highNotLow);

#endif //GPIO_H
