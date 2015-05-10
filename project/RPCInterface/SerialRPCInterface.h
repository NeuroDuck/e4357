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
*This class sets up RPC communication over serial.
*/
#ifndef INTERFACE
#define INTERFACE

/**
*Includes
*/
#include "mbed.h"
#include "platform.h"
#include "rpc.h"
#include "RPCFunction.h"
#include "RPCVariable.h"


namespace mbed{
/**
*Provides an Interface to mbed over RPC. 
* 
*For the chosen communication type this class sets up the necessary interrupts to receive RPC messages. Receives the messages, passes them to the rpc function and then returns the result.
*/
class SerialRPCInterface{
public:
    /**
    *Constructor
    *
    *Sets up RPC communication using serial communication.
    *
    *@param tx The transmit pin of the serial port.
    *@param rx The receive pin of the serial port. 
    *@param baud Set the baud rate, default is 9600.
    */
    SerialRPCInterface(PinName tx, PinName rx, int baud = 9600);
 
    /**
    *Disable the RPC. 
    * 
    *This will stop RPC messages being recevied and interpreted by this library. This might be used to prevent RPC commands interrupting an important piece of code on mbed.
    */
    void Disable(void);
    
    /**
    *Enable the RPC
    * 
    *This will set this class to receiving and executing RPC commands. The class starts in this mode so this function only needs to be called if you have previosuly disabled the RPC.
    *
    */
    void Enable(void);
    
    //The Serial Port
    Serial pc;
    

private:
    //Handle messgaes and take appropriate action
    void _MsgProcess(void);
    void _RegClasses(void);
    void _RPCSerial();
    bool _enabled;
    char _command[256];
    char _response[256];
    bool _RPCflag;
};
}
#endif