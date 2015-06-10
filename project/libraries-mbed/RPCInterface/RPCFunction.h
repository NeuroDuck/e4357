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
* @section Description
*This class provides an object which can be called over RPC to run the function which is attached to it.
* 
*/     
#ifndef RPCFUNCTION_RPC
#define RPCFUNCTION_RPC
/**
*Includes
*/
#include "mbed.h"
#include "platform.h"
#include "rpc.h"
#define STR_LEN 64
#include "platform.h"
 
#ifdef MBED_RPC
#include "rpc.h"
#endif
/**
*
*Class to call custom functions over RPC
*
*/
class RPCFunction : public Base{
public:
    /**
    * Constructor
    * 
    *@param f Pointer to the function to call. the function must be of the form void foo(char * input, char * output)
    *@param name The name of this object
    */
    RPCFunction(void(*f)(char*, char*), const char* = NULL);

    /** 
    *run 
    *
    *Calls the attached function passing the string in but doesn't return the result.
    *@param str The string to be passed into the attached function. This string can consist of any ASCII characters apart from escape codes. The usual limtations on argument content for RPC strings has been removed
    *@return A string output from the function
    */
    char * run(char* str);
    
    /**
    *Reads the value of the output string.
    *
    *@returns the string outputted from the last time the function was called
    */
    char * read();


     #ifdef MBED_RPC
    virtual const struct rpc_method *get_rpc_methods();      
     #endif

private:
    void (*_ftr)(char*, char*);
    
    char _input[STR_LEN];
    char _output[STR_LEN];
    
};
#endif