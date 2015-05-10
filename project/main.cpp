#include "mbed.h"
// #include "m3pi.h"
#include "SerialRPCInterface.h"

SerialRPCInterface rpcInterface( USBTX, USBRX);

void ReadRange( char* input, char* output);
RPCFunction RangeFinder( &ReadRange, "RangeFinder");

void ReadRange( char* input, char* output)
{
    //Format the output of the srf08 into the output string
    sprintf( output, "%f", 42.42);
}

// First create the variables you wish to use.
//
float f = 42;
int   i = 43;
char  c = 'b';

// Then attach them to an RPCVariable Object.
//
RPCVariable<float> rpc_f( &f, "f");
RPCVariable<int>   rpc_i( &i, "i");
RPCVariable<char>  rpc_c( &c, "c");

int main() 
{
	rpcInterface.Enable();
	
	int a = 0;		// To have a line to set a breakpoint on.
	  
}
