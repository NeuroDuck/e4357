#include "mbed.h"
#include "rtos.h"
#include "SerialRPCInterface.h"
#include "RPCVariable.h"

DigitalOut led1(LED1);
DigitalOut led2(LED2);

void led2_thread(void const *args)
{
    while (true) {
        led2 = !led2;
        Thread::wait( 1000);
    }
}

float f = 42;
int   i = 43;
char  c = 'b';
RPCVariable<float> rpc_f(&f, "f");
RPCVariable<int> rpc_i( &i, "i");
RPCVariable<char> rpc_c(& c, "c");

int main()
{
    SerialRPCInterface rpc( USBTX, USBRX);
    Thread thread(led2_thread);

    while (true) {
        led1 = !led1;
        Thread::wait(500);
    }
}
