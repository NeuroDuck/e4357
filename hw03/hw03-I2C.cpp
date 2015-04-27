/* mbed Example Program
 * Copyright (c) 2006-2014 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 // Smashed together with the code from here:
 // http://www.gammon.com.au/forum/?id=10896
 //
#include "mbed.h"

          // vvv      vvv These are actually SDA2 & SCL2.
I2C i2c( p28, p27); // I2C_SDA, I2C_SCL); 
//       ^^^ P0[27]   ^^^ P0[28].
//Should ==  p28      ==  p27.
//       ORANGE_WIRE  == GREEN_WIRE

const int SLAVE_ADDRESS7BIT = 0x2a;                   // 7 bit I2C address
const int SLAVE_ADDRESS8BIT = SLAVE_ADDRESS7BIT << 1; // 8 bit I2C address.

enum {
    CMD_ID = 1,
    CMD_READ_ORANGE_D5 = 2,
    CMD_READ_GREEN_D8 = 3
};
char cmds[3] = {
    CMD_ID, 
    CMD_READ_ORANGE_D5, 
    CMD_READ_GREEN_D8 
};
char result[10];

int sendCommand( const char* cmd, const int responseSize)
{
    //       ( int address, const char *data, int length);
    i2c.write( SLAVE_ADDRESS8BIT, cmd, 1);
    
    wait_ms( 500);
    
    //                ( int address, char *data, int length)
    int ack = i2c.read( SLAVE_ADDRESS8BIT, result, responseSize);

    printf( "cmd = %x, ", cmd);
    printf( "result = %x\r\n", result);
    
    return ack;
  }

int main() 
{        
    int cmdsNdx = 0;
    
    int ack = sendCommand( &(cmds[cmdsNdx++]), 1);
  
    if (ack == 0)
        printf( "Slave is ID: %x\r\n", result[0]);
    else
        printf( "ack = %x, No response to ID request.", ack);
        
return 0;
    
    while (1) 
    {
       sendCommand( &(cmds[cmdsNdx]), 1);
       
       cmdsNdx = (cmdsNdx + 1) % (sizeof( cmds) / sizeof( char));
    }
    
    return 0;
}
