//
// Keil's 32kByte-limit means that I can't compile in all at the same
// time the Libraries/Functions for:
//    m3pi
//    RPCFunction/ReadRange
//    RPCVariable
//
// So I comment out the ones I'm not debugging into, and leave
// uncommented-out just the one that I am debugging.
//
#include "mbed.h"
#include "m3pi/m3pi.h"
#include "SerialRPCInterface.h"
/*
SerialRPCInterface rpcInterface( USBTX, USBRX);

void ReadRange( char* input, char* output);
RPCFunction RangeFinder( &ReadRange, "RangeFinder");

void ReadRange( char* input, char* output)
{
    // Format the output of the srf08 into the output string.
		//
		strcpy( output, "abc");
//    sprintf( output, "%f", 42.42);
}
*/

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


// m3pi mpi;

int main() 
{
	// I'm happy to report that typing in /m3pi/battery in my Serial 
	// Terminal program did print out the battery value, and typing 
	// in /m3pi/left did make the Robot spin left.
	//
	// Meaning that the m3pi Library's approach to register-function-
	// for-RPC is working.
	//
	// Since the SerialRPCInterface Library's RPCFunction.cpp's 
  // registration approach isn't working, I'll modify it to match
	// the mp3i Library's approach, and then hopefully we'll have 
	// a RPCFunction()-registration Class that works.

//		mpi.locate( 0,1);
//		char msg[] = "USB RPC";
//    mpi.print( msg, strlen( msg));
	
	int a = 0;		// To have a line to set a breakpoint on.
	  
}
