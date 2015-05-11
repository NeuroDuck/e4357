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
#include "RPCFunction.h"
#include "rpc.h"

//Parse a char argument without delimiting by anything that is non alphanumeric - based on version in rpc.h line 153
 char *parse_arg_char(const char *arg, const char **next) {
        const char *ptr = arg;
        char *res = NULL;
        if(*arg == '"') {
            /* quoted string */
            ptr = ++arg;
            int len = 0;
            /* find the end (and length) of the quoted string */
            for(char c = *ptr; c != 0 && c != '"'; c = *++ptr) {
                len++;
                if(c == '\\') {
                    ptr++;
                }
            }  
            /* copy the quoted string, and unescape characters */
            if(len != 0) {
                res = new char[len+1];
                char *resptr = res;
                while(arg != ptr) {
                    *resptr++ = parse_char(arg, &arg);
                }
                *resptr = 0;
            }
        } else {
            /* unquoted string */
            while(isalnum(*ptr) || isgraph(*ptr) || *ptr=='_' || *ptr == ' ') {             //Edit this line to change which types of characters are allowed and which delimit
                ptr++;
           }
            int len = ptr-arg;
            if(len!=0) {                                //Chnages made to just pass whole string with no next arg or delimiters, these changes just removes space at the beginning
                res = new char[len];                    //was len+1
                memcpy(res, arg + 1, len - 1);          // was arg, len
                res[len-1] = 0;                         //was len 
            }
        }
      
        if(next != NULL) {
            *next = ptr;
        } 
      return res;
    }  
    
    //Custom rpc method caller for execute so that the string will not be delimited by anything
    //See line 436 of rpc.h
    void rpc_method_caller_run(Base *this_ptr, const char *arguments, char *result) {
    
        const char *next = arguments;
        char* arg1 = parse_arg_char(next,NULL);
        
        char * res = (static_cast<RPCFunction*>(this_ptr)->run)(arg1);
        if(result != NULL) {
            write_result<char*>(res, result);
        }
        delete arg1;    // Seems to stop a memory leak issue which prevented an RPCFunction being run more than ~500 times
    }

   RPCFunction::RPCFunction(void(*f)(char*, char*), const char* name) : 
		 Base(name)
	 {
        _ftr = f;   
   }
    

    //Just run the attached function using the string thats in private memory - or just using null values, 
    char * RPCFunction::run(char * input)
		{
        strcpy( _input, input);
			
        (*_ftr)( _input, _output);
			
        return( _output);
    }
    
    //Just read the output string
    char* RPCFunction::read()
		{
        return( _output);
    }
  
    #ifdef MBED_RPC
		const rpc_method *RPCFunction::get_rpc_methods() 
		{
			static const rpc_method rpc_methods[] = {
				{ "run", rpc_method_caller<char*, RPCFunction, char*, &RPCFunction::run> },
//        { "read", rpc_method_caller<RPCFunction, float, &RPCFunction::read> },
		        RPC_METHOD_SUPER(Base)
      };
			
      return rpc_methods;
		}
		#endif
/*
    #ifdef MBED_RPC
    const rpc_method *RPCFunction::get_rpc_methods() 
    {
       static const rpc_method rpc_methods[] = { 
        { "run", rpc_method_caller_run }, // Run using custom caller, all characters accepted in string.
        { "read", rpc_method_caller<char*, RPCFunction, &RPCFunction::read> },
        RPC_METHOD_SUPER(Base)
      };
      return rpc_methods;
    }       
		#endif
	*/
