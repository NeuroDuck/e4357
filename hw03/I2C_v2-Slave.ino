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

// Various commands we might get:
//
enum {
    CMD_ID = 1,
    CMD_READ_ORANGE_D5 = 2,
    CMD_READ_GREEN_D8 = 3
};
char command;

// Quoting from:
//   http://www.gammon.com.au/forum/?id=10896&reply=9#reply9
// 
// To change the I2C Bus Speed:
//   Choose another speed using TWBR (Two Wire Bit Rate) register:
//   TWBR = 12;  // 400 kHz (maximum)
//   TWBR = 32;  // 200 kHz
//   TWBR = 72;  // 100 kHz (default)
//   TWBR = 152;  // 50 kHz 
//   TWBR = 78;  // 25 kHz 
//   TWSR |= bit( TWPS0);  // change prescaler
//   TWBR = 158;  // 12.5 kHz 
//   TWSR |= bit (TWPS0);  // change prescaler
// Examples are for Atmega328P running at 16 MHz 
// (eg. Arduino Uno, Duemilanove, etc.).
//
// The slave address is a 7-bit address, 
// thus is in the range 0 to 127 decimal (0x00 to 0x7F in hex). 
// However addresses 0 to 7, and 120 to 127 are reserved.
//
// If you see a number larger than 127 (0x7F) quoted, then that 
//  is the 8-bit address, which includes the read/write bit. 
// You need to divide an 8-bit address by two (shift right one) 
// to get the correct address for the Wire library.
//
// For example if a datasheet says to use address 0xC0 for 
// writing and 0xC1 for reading, that includes the read/write 
// bit. Drop the "1" and divide by two, giving the "real" 
// address of 0x60.
//
// If you are not sure, use the I2C scanner from here:
//   http://www.gammon.com.au/forum/?id=10896&reply=6#reply6
// to determine which address your device actually uses.

void setup()
{
  Serial.begin( 9600);  // start serial for output
  
  command = 0;
  pinMode( LED, OUTPUT);
  
  //                         O'scope:
  // SDA = A4 = ORANGE_WIRE. Blue Clip
  // SLC = A5 = GREEN_WIRE.  Green Clip.

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
