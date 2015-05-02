#include "mbed.h"

DigitalOut myled(LED1);

int main() 
{
    while(1) 
    {
//        myled = 1;
        wait( 1.2);
        myled = 0;
        wait( 0.005);
    }
}
