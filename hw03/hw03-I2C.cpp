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
I2C i2c( p28,         p27); 
//       I2C_SDA,     I2C_SCL); 
//       ^^^ P0[27]   ^^^ P0[28].
//Should ==  p28      ==  p27.
//       ORANGE_WIRE  ==  GREEN_WIRE
//Oscope Blue Clip,       Green Clip.

const int SLAVE_ADDRESS7BIT = 0x2a;                   // 7 bit I2C address
const int SLAVE_ADDRESS8BIT = SLAVE_ADDRESS7BIT << 1; // 8 bit I2C address.

enum {
    CMD_ID = 1,
    CMD_READ_ORANGE_D5 = 2,
    CMD_READ_GREEN_D8 = 3
};
char cmdCodes[3] = {
    CMD_ID, 
    CMD_READ_ORANGE_D5, 
    CMD_READ_GREEN_D8 
};
char* cmdNames[3] = {
    "ID",
    "ORANGE_D5",
    "GREEN_D8"
};

char result[10];

int sendCommand( const char cmdNdx, const int responseSize)
{
    //       ( int address, const char *data, int length);
    i2c.write( SLAVE_ADDRESS8BIT, &(cmdCodes[cmdNdx]), 1);
    
    wait_ms( 2);
    
    //                ( int address, char *data, int length)
    int ack = i2c.read( SLAVE_ADDRESS8BIT, result, responseSize);

    printf( "cmdCode = %d, ", cmdCodes[cmdNdx]);
    printf( "%s = %x\r\n", cmdNames[cmdNdx], result[0]);
    
    return ack;
  }

int main() 
{        
    int cmdNdx = 2;
    
    int ack = sendCommand( cmdNdx++, 1);
  
    if (cmdNdx == 0)
    {
        if (ack == 0)
            printf( "Slave is ID: %x\r\n", result[0]);
        else
            printf( "ack = %x, No response to ID request.", ack);
    }
          
return 0;
          
    sendCommand( cmdNdx++, 1);
    sendCommand( cmdNdx++, 1);
    
    return 0;
}
