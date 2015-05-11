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
*This class provides an object to which a variable can be attached. Any type 
*for which a parse_args function specilisation exists can be attached. This includes 
*all of the standard types.
*
*/
 #ifndef RPCVARIABLE_H_  
 #define RPCVARIABLE_H_

#include "mbed.h"
#include "platform.h"
#include "rpc.h"
 /**
 *Class to read and set an attached variable using the RPC
 *
 */
template<class T>
class RPCVariable : public Base{
public:
    /**
    * Constructor
    * 
    *@param ptr Pointer to the variable to make accessible over RPC. Any type of 
    *variable can be connected
    *@param name The name of that this object will be over RPC
    */
    template<class A>
    RPCVariable(A * ptr, const char * name) : Base(name){
        _ptr = ptr;
    }
    /**
    *Read the variable over RPC.
    * 
    *@return The value of the variable
    */
    T read(){
        return(*_ptr);
    }
    /**
    *Write a value to the variable over RPC
    * 
    *@param The value to be written to the attached variable.
    */
    void write(T value){
        *_ptr = value;
    }
		
    #ifdef MBED_RPC
    virtual const struct rpc_method *get_rpc_methods();    
    static struct rpc_class *get_rpc_class();
    #endif

private:
    T * _ptr;
                                           
};

//Set up RPC methods

#ifdef MBED_RPC
template <class T>
    const rpc_method *RPCVariable<T>::get_rpc_methods() 
		{
       static const rpc_method rpc_methods[] = {
        { "read", rpc_method_caller<T, RPCVariable, &RPCVariable::read> },
        { "write", rpc_method_caller<RPCVariable, T, &RPCVariable::write> },
        RPC_METHOD_SUPER(Base)
      };
      return rpc_methods;
    }       
		
    template <class T>
    rpc_class *RPCVariable<T>::get_rpc_class() 
		{
        static const rpc_function funcs[] = {
					"new", rpc_function_caller<
						const char*, T, const char* , 
						&Base::construct<RemoteVar, T, const char*>
					>, RPC_METHOD_END
				};
        static rpc_class c = { "RPCVariable", funcs, NULL };
        return &c;
    }
#endif

// There could be specialisation for integer, to also give increment and 
// decrements.


#endif  //RPCVARIABLE_H_