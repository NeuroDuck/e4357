// Wire Master Reader
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Reads data from an I2C/TWI slave device
// Refer to the "Wire Slave Sender" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

#include <Wire.h>

// From:
//   http://www.gammon.com.au/forum/?id=10896
//
const byte SLAVE_ADDRESS = 0x2a;
const byte LED = 13;
const byte ORANGE_PIN = 5;
const byte GREEN_PIN  = 8;

// Adapted from:
//   http://www.gammon.com.au/forum/?id=10896
//
// various commands we might get
enum {
    CMD_ID = 1,
    CMD_READ_ORANGE_D5 = 2,
    CMD_READ_GREEN_D8 = 3
};
char command;

void setup()
{
  Serial.begin( 9600);  // start serial for output
  
  command = 0;
  pinMode( LED, OUTPUT);
  
  // SDA = A4 = ORANGE_WIRE.
  // SLC = A5 = GREEN_WIRE.
  //
  // I am a slave.
  Wire.begin( SLAVE_ADDRESS); // Join i2c bus (address optional for master).
  
  Wire.onReceive( receiveHandler);  // Interrupt handler for incoming messages. 
  Wire.onRequest( requestHandler);  // Interrupt handler for when data is wanted.
}

void receiveHandler( int howMany)
{
  command = Wire.read();  // remember command for when we get request
  
  printHexCharWithMsg( "Received command = 0x", command);
} 

void printHexCharWithMsg( char* msg, char hexChar)
{
  Serial.print( msg);
  Serial.print( hexChar, HEX);
  Serial.print( "\n");
}

void requestHandler()
{
    printHexCharWithMsg( "Answering with results for command = 0x", command);

  char myID = 0x55;
  char orangeResult, greenResult;

  switch (command)
  {
  case CMD_ID:      
    Wire.write( myID); 
    printHexCharWithMsg( "ID = 0x", myID);
    break;   // send our ID 
    
  case CMD_READ_ORANGE_D5:
    orangeResult = digitalRead( ORANGE_PIN);
    printHexCharWithMsg( "ID = 0x", orangeResult);
    Wire.write( orangeResult);
    break;  // send ORANGE_PIN's state.
    
  case CMD_READ_GREEN_D8: 
    greenResult = digitalRead( GREEN_PIN);
    Wire.write( greenResult);
    break;   // send GREEN_PIN's state.
  }
}

#define NO_DEBUG

void loop()
{
  // All done by interrupts.
}
