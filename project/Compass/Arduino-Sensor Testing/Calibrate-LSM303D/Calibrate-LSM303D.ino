#include <Arduino.h>

#define SCLPIN_GREEN_PROBE A0
#define SDAPIN_BLUE_PROBE  A1

#include <UsefulFunctions.h>
#include <SoftI2CMaster.h>

// When we need SoftI2CMaster's debugging print statements, we do the setup in 
// setup()->i2cSM.setPins(), to allow i2cSM.setPins()'s debugging print statements 
// happen after we've called Serial.begin().
//
SoftI2CMaster i2cSM(
  SCLPIN_GREEN_PROBE, 
  SDAPIN_BLUE_PROBE, 
  SoftI2CMaster::i2c_internal_pullups,  // Trigger won't happen when SCL
  SoftI2CMaster::i2c_scl_not_pulled_up  // is Pulled-Up.
);

#include <LSM303D.h>
// LSM303D compassD;

LSM303D::vector<int16_t> 
  running_min = { 32767,  32767,  32767}, 
  running_max = {-32768, -32768, -32768};

void setup()
{
  Serial.begin( 57600);
//  Serial.println( "setup()");

  // Address the Compass, to let it know that we're querying it.
  SoftI2CMaster::ackNotNackType ackBit1 =    // 0x1e
    i2cSM.beginTransmission( LSM303D::D_SA0_LOW_ADDRESS);

  // As per: "e4357\project\Compass\LSM303D-datasheet.pdf"'s Table 14, pg. 23:
  // Read the value from the Compass's WHO_AM_I Register:  
  // First clock out the WHO_AM_I SubAddress that we want to read from.
  //
  SoftI2CMaster::ackNotNackType ackBit2 = // 0x0f
    i2cSM.i2c_writeSubAddress( LSM303D::WHO_AM_I, SoftI2CMaster::i2c_no_auto_inc_sub);

  // Clock out a Repeated-Start to read the Compass's WHO_AM_I Register's value.
  // Calling beginTransmission() a 2nd time before calling endTransmission()
  // will clock out a Repeated-Start instead of a Start.
  // Also, tell beginTransmission() that we're reading this time, instead of writing,
  // which is its default action.
  //
  SoftI2CMaster::ackNotNackType ackBit3 = 
    i2cSM.beginTransmission( 
      LSM303D::D_SA0_LOW_ADDRESS,
      SoftI2CMaster::i2c_rw_bit_is_read);
      
   uint8_t whoAmI = i2cSM.i2c_read( SoftI2CMaster::i2c_nak);

   i2cSM.endTransmission();

    delay( 5000);
    printEnclosedHexData( "whoAmI", whoAmI);
/*
    printEnclosedBinData( "ackBit1", ackBit1);
    printEnclosedBinData( "ackBit2", ackBit2);
    printEnclosedBinData( "ackBit3", ackBit3);
*/

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
