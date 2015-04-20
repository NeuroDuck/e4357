// HW01
// Assigned Dip pin# 27
// port 0, pin 11

#include <stdint.h>
#include "mbed.h"
/*
#define LPC_GPIO_BASE         (0x2009C000UL)

#define     __IO    volatile             
#define     __O    volatile             

typedef struct
{
  union {
    __IO uint32_t FIODIR;
    struct {
      __IO uint16_t FIODIRL;
      __IO uint16_t FIODIRH;
    };
    struct {
      __IO uint8_t  FIODIR0;
      __IO uint8_t  FIODIR1;
      __IO uint8_t  FIODIR2;
      __IO uint8_t  FIODIR3;
    };
  };
  uint32_t RESERVED0[3];
  union {
    __IO uint32_t FIOMASK;
    struct {
      __IO uint16_t FIOMASKL;
      __IO uint16_t FIOMASKH;
    };
    struct {
      __IO uint8_t  FIOMASK0;
      __IO uint8_t  FIOMASK1;
      __IO uint8_t  FIOMASK2;
      __IO uint8_t  FIOMASK3;
    };
  };
  union {
    __IO uint32_t FIOPIN;
    struct {
      __IO uint16_t FIOPINL;
      __IO uint16_t FIOPINH;
    };
    struct {
      __IO uint8_t  FIOPIN0;
      __IO uint8_t  FIOPIN1;
      __IO uint8_t  FIOPIN2;
      __IO uint8_t  FIOPIN3;
    };
  };
  union {
    __IO uint32_t FIOSET;
    struct {
      __IO uint16_t FIOSETL;
      __IO uint16_t FIOSETH;
    };
    struct {
      __IO uint8_t  FIOSET0;
      __IO uint8_t  FIOSET1;
      __IO uint8_t  FIOSET2;
      __IO uint8_t  FIOSET3;
    };
  };
  union {
    __O  uint32_t FIOCLR;
    struct {
      __O  uint16_t FIOCLRL;
      __O  uint16_t FIOCLRH;
    };
    struct {
      __O  uint8_t  FIOCLR0;
      __O  uint8_t  FIOCLR1;
      __O  uint8_t  FIOCLR2;
      __O  uint8_t  FIOCLR3;
    };
  };
} LPC_GPIO_TypeDef;

#define LPC_GPIO_PORT_OFFSET        (0x20UL)
#define LPC_GPIO1_BASE        (LPC_GPIO_BASE + 0x00020)
#define LPC_GPIO2_BASE        (LPC_GPIO_BASE + 0x00040)
#define LPC_GPIO3_BASE        (LPC_GPIO_BASE + 0x00060)
#define LPC_GPIO4_BASE        (LPC_GPIO_BASE + 0x00080)

#define LPC_GPIO1             ((LPC_GPIO_TypeDef      *) LPC_GPIO1_BASE    )
#define LPC_GPIO2             ((LPC_GPIO_TypeDef      *) LPC_GPIO2_BASE    )
#define LPC_GPIO3             ((LPC_GPIO_TypeDef      *) LPC_GPIO3_BASE    )
#define LPC_GPIO4             ((LPC_GPIO_TypeDef      *) LPC_GPIO4_BASE    )

#define LPC_GPIO0_BASE        (LPC_GPIO_BASE + 0x00000)
#define LPC_GPIO0             ((LPC_GPIO_TypeDef      *) LPC_GPIO0_BASE    )
*/
unsigned char buffer[1024];

#define MYPORTPINVALUES LPC_GPIO0->FIOPIN

void sample( unsigned char *buf)
{
    register unsigned char reg1, reg2, reg3, reg4, reg5, reg6, reg7, reg8;

    reg1 = MYPORTPINVALUES;
    reg2 = MYPORTPINVALUES;
    reg3 = MYPORTPINVALUES;
    reg4 = MYPORTPINVALUES;
    reg5 = MYPORTPINVALUES;
    reg6 = MYPORTPINVALUES;
    reg7 = MYPORTPINVALUES;
    reg8 = MYPORTPINVALUES;
 
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

int main( void)
{ 
/*
    int c;
*/
    Serial pc( USBTX, USBRX); // tx, rx 
    pc.baud( 9600);          //or pc.baud( 115200);

    pc.printf( "\r\nHello World!\r\n"); 

    unsigned char *p = buffer;
 
    while (false && p < &buffer[1023])
    {
        pc.printf( "\r\nHello World!\r\n"); 
        
        sample( p);
        p += 8;
    }
 
    return 0;
}
/*
        while(1) { 
            pc.printf("\r\nEnter:"); 
            ch = pc.getc(); 
            pc.putc(ch); 
            DisplayLed(ch); 
*/