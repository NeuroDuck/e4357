#include "mbed.h"
#include "CircularBuffer.h"
/*
DigitalOut myled( LED1);

int main() 
{	
    while (1) 
    {
//        myled = 1;
        wait( 1.8);
        myled = 0;
        wait( 0.005);
    }
}

inline int CircularBufferIsFull( SmallCircularBuffer* que) 
{
	return que->isFull();	
}

inline int CircularBufferIsEmpty( SmallCircularBuffer* que) 
{	
		return que->isEmpty();
}
*/

/* 
* Enter a hex number [0-9a-fA-F]; Decode it in 4-bit binary format and display them on 4 on board leds. 
*/ 

#include "mbed.h" 

Serial pc(USBTX, USBRX); // tx, rx 
DigitalOut ledArr[4] = {DigitalOut(LED1), DigitalOut(LED2), DigitalOut(LED3), DigitalOut(LED4)}; 
void DisplayLed(int ch) 
{ 
	int i=0;

	if (ch>='0' && ch<='9')
		ch-='0';
	else if (ch>='A' && ch<='F') {
		ch-='A';
		ch+=10;
	} else if (ch>='a' && ch<='f') {
		ch-='a';
		ch+=10;
	} else
		ch=0;

	for (i=0; i<4; i++) { 
		if(ch& (1<<i))
			ledArr[i] = 1; 
		else
			ledArr[i] = 0;
	} 
} 

int main(void) { 
	int ch; 
	pc.baud(9600); 
	pc.printf("\r\nHello World!"); 
	while(1) { 
		pc.printf("\r\nEnter:"); 
		ch = pc.getc(); 
		pc.putc(ch); 
		DisplayLed(ch); 
	} 
}
