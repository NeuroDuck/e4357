
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
} 

void requestHandler()
{
  switch (command)
  {
  case CMD_ID:      
    Wire.write( 0x55); 
    break;   // send our ID 
  case CMD_READ_ORANGE_D5: 
    Wire.write( digitalRead( ORANGE_PIN));
    break;  // send ORANGE_PIN's state.
  case CMD_READ_GREEN_D8: 
    Wire.write( digitalRead( GREEN_PIN));
    break;   // send GREEN_PIN's state.
  }
}

#define NO_DEBUG

void loop()
{
  // All done by interrupts.
}

// Code for Slave Sender - Program for Arduino 2
// Wire Slave Sender
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Sends data as an I2C/TWI slave device
// Refer to the "Wire Master Reader" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

/*
#include <Wire.h>

void setup()
{
  Wire.begin(2);                // join i2c bus with address #2
  Wire.onRequest(requestEvent); // register event
}

void loop()
{
  delay(100);
}

// function that executes whenever data is requested by master
// this function is registered as an event, see setup()
void requestEvent()
{
  Wire.write("hello "); // respond with message of 6 bytes
                       // as expected by master
}
*/
