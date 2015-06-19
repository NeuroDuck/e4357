#include <Arduino.h>

#define SCLPIN_GREEN_PROBE A0
#define SDAPIN_BLUE_PROBE  A1

#include <UsefulFunctions.h>
#include <SoftI2CMaster.h>

// We do the setup in setup(), to allow i2cSM.setPins()'s debugging print
// statements to print successfully.
//
SoftI2CMaster i2cSM;  

#include <LSM303D.h>

// LSM303D compassD;

LSM303D::vector<int16_t> 
  running_min = { 32767,  32767,  32767}, 
  running_max = {-32768, -32768, -32768};

// Read a single byte from address and return it as a byte.
//
/*
byte readRegister(uint8_t address)
{
  byte data;

//  i2cSM.beginTransmission( MMA8452_ADDRESS);
  i2cSM.write( address);
  i2cSM.endTransmission();

//  i2cSM.requestFrom( MMA8452_ADDRESS);
  data = i2cSM.readLast();
  i2cSM.endTransmission();
  
  return data;
}
*/
void setup()
{
  Serial.begin( 57600);
  Serial.println( "setup()");

  i2cSM.setPins( 
    SCLPIN_GREEN_PROBE, 
    SDAPIN_BLUE_PROBE, 
    SoftI2CMaster::i2c_internal_pullups,  // Trigger won't happen when SCL
    SoftI2CMaster::i2c_scl_not_pulled_up  // is Pulled-Up.
  );

// From:
// http://forum.arduino.cc/index.php?PHPSESSID=77d80gbsl7bdpl72j9gnq5lo65&topic=126021.msg947589#msg947589
// "2) capacitance: In the 2nd block of code, the port is (presumed) 
// high before it was turned into input."
//
// In other words, the short positive pulse I see after issuing
// i2cSM.i2c_scl_hi(), may be due to that capacitance bleeding off.
/*
  while (1)
  {
    i2cSM.i2c_sda_lo();
    _delay_us( i2cIntraBitDelayUs);
    i2cSM.i2c_scl_lo();
    _delay_us( 10);
return;
    i2cSM.i2c_scl_hi();
    i2cSM.i2c_sda_hi();
    _delay_us( 0);
  }
*/
    uint8_t ackBit =
      i2cSM.beginTransmission( 
//        0b1010101,
        LSM303D::D_SA0_LOW_ADDRESS,
        SoftI2CMaster::i2c_rw_bit_is_write);

//    i2cSM.beginTransmission( 
//        0b0101010, SoftI2CMaster::i2c_rw_bit_is_read);

//    i2cSM.endTransmission();

    printEnclosedBinData( "ackBit", ackBit);
/*
    i2cSM.i2c_scl_hi();
    i2cSM.i2c_sda_hi();
    _delay_us( 5);

    i2cSM.i2c_scl_lo();
    i2cSM.i2c_sda_lo();
    _delay_us( 5);
*/
  
//    i2cSM.i2c_both_hi();
//    i2cSM.i2c_sda_hi();
//    _delay_us( 5);

//  compassD.init(); // LSM303D::device_D, LSM303D::sa0_high);
//  compassD.enableDefault();

  int found = false;
  byte data;
   
  return;
/*
  Serial.println( "Scanning ...");

  // Reading works, as done by these steps...
  //
  for (uint8_t address = 0x1e ; address <= 0x1e ; address += 2)
  {    
    uint8_t ackBit = 
      i2cSM.beginTransmission( address, SoftI2CMaster::i2c_rw_bit_is_read);
      
    if (ackBit == SoftI2CMaster::i2c_nak)
      continue;

    found = true;

    Serial.print( "Answer received from: ");
    Serial.println( address, HEX);

    // We're trying to complete the LSM303D datasheet's Table 14 
    // protocol, so no need for this:
    //
    i2cSM.endTransmission();

return;

    Serial.print( "subAddress is: ");
    Serial.println( LSM303D::WHO_AM_I, HEX);
    
    ackBit = 
      i2cSM.i2c_writeSubAddress( 
        LSM303D::WHO_AM_I, SoftI2CMaster::i2c_no_auto_inc_sub);

    if (ackBit == SoftI2CMaster::i2c_ack)
      Serial.println( "SAK received.");
    else
      Serial.println( "No SAK received.");
  }

  Serial.println( "Done.\n\n");
*/
}

char report[80];

void loop()
{
    delay(100);
return;
  Serial.println( "loop()");
/*
  compassD.read();
  
  running_min.x = min( running_min.x, compassD.m.x);
  running_min.y = min( running_min.y, compassD.m.y);
  running_min.z = min( running_min.z, compassD.m.z);

  running_max.x = max(running_max.x, compassD.m.x);
  running_max.y = max(running_max.y, compassD.m.y);
  running_max.z = max(running_max.z, compassD.m.z);
  
  snprintf(
    report, sizeof(report), 
    "min:{%+6d, %+6d, %+6d}  max:{%+6d, %+6d, %+6d}",
    running_min.x, running_min.y, running_min.z,
    running_max.x, running_max.y, running_max.z);
  Serial.println( report);
*/
}
