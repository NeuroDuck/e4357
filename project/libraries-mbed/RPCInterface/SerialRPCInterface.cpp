 /**
* @section LICENSE
*Copyright (c) 2010 ARM Ltd.
* 
*Permission is hereby granted, free of charge, to any person obtaining a copy
*of this software and associated documentation files (the "Software"), to deal
*in the Software without restriction, including without limitation the rights
*to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*copies of the Software, and to permit persons to whom the Software is
*furnished to do so, subject to the following conditions:
* 
*The above copyright notice and this permission notice shall be included in
*all copies or substantial portions of the Software.
* 
*THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*THE SOFTWARE.
*
* 
* @section DESCRIPTION
*
*This class sets up RPC communication. This allows objects on mbed to be controlled. Objects can be created or existing objects can be used
*/
#include "SerialRPCInterface.h"

using namespace mbed;

// Requires multiple constructors for each type, 
// serial to set different pin numbers, TCP for port.
//
SerialRPCInterface::SerialRPCInterface( PinName tx, PinName rx, int baud) :
	pc( tx, rx) 
{
    _RegClasses();
	
    _enabled  = true;
	
     pc.attach( this, &SerialRPCInterface::_RPCSerial, Serial::RxIrq);
	
     if (baud != 9600)
			 pc.baud( baud);
}

void SerialRPCInterface::_RegClasses( void)
{
    // Register classes with base.
    //
    // Base::add_rpc_class<AnalogIn>();
 //   Base::add_rpc_class<DigitalIn>();
 //   Base::add_rpc_class<DigitalOut>();
/*
    Base::add_rpc_class<DigitalInOut>();
    Base::add_rpc_class<PwmOut>();
    Base::add_rpc_class<Timer>();
    Base::add_rpc_class<BusOut>();
    Base::add_rpc_class<BusIn>();
    Base::add_rpc_class<BusInOut>();
*/
//    Base::add_rpc_class<Serial>();

    // AnalogOut not avaliable on mbed LPC11U24 so only compile for other devices
    #if !defined(TARGET_LPC11U24) 
//    Base::add_rpc_class<AnalogOut>();
    #endif
}

void SerialRPCInterface::Disable( void)
{
     _enabled = false;
}
void SerialRPCInterface::Enable( void)
{
    _enabled = true;
}
void SerialRPCInterface::_MsgProcess( void)
{
    if (_enabled == true)
		{
        rpc( _command, _response);
    }
}

void SerialRPCInterface::_RPCSerial() 
	{
    _RPCflag = true;
		
    if (_enabled == true)
		{
        pc.gets( _command, 256);
				
        _MsgProcess();
				
        pc.printf( "%s\n", _response);
    }
		
    _RPCflag = false;
}
