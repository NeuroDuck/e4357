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
LSM303D compassD;

LSM303D::vector<int16_t> 
  running_min = { 32767,  32767,  32767}, 
  running_max = {-32768, -32768, -32768};
  
uint8_t magAdr = 0x1e;

void setup()
{
  Serial.begin( 57600);
//  Serial.println( "setup()");

  compassD.init();
  compassD.enableDefault();
}

char report[80];

void loop()
{
  compassD.read();
  
  running_min.x = min( running_min.x, compassD.m.x);
  running_min.y = min( running_min.y, compassD.m.y);
  running_min.z = min( running_min.z, compassD.m.z);

  running_max.x = max( running_max.x, compassD.m.x);
  running_max.y = max( running_max.y, compassD.m.y);
  running_max.z = max( running_max.z, compassD.m.z);
  
  snprintf(
    report, sizeof(report), 
    "min:{%+6d, %+6d, %+6d}  max:{%+6d, %+6d, %+6d}",
    running_min.x, running_min.y, running_min.z,
    running_max.x, running_max.y, running_max.z);

  Serial.println( report);
}

void testReadWhoAmIRegister()
{
  uint8_t whoAmI   = 42;
  uint8_t regVal   = 0;
  uint8_t numBytes = 1;

  uint8_t ackBits0 = 
    i2cSM.readBytesFrom( 
      LSM303D::D_SA0_LOW_ADDRESS, LSM303D::WHO_AM_I, 
      SoftI2CMaster::i2c_read_or_write_1_byte, &regVal);
      
  delay( 1000);
  printEnclosedHexData( "whoAmI", whoAmI);
  printEnclosedBinData( "ackBit1", ackBits0 & 0x1); ackBits0 >>= 1;
  printEnclosedBinData( "ackBit2", ackBits0 & 0x1); ackBits0 >>= 1;
  printEnclosedBinData( "ackBit3", ackBits0 & 0x1); ackBits0 >>= 1;
}

void testReadRegistersIndividually()
{
  printEnclosedLSM303DHexRegister( "CTRL2", magAdr, LSM303D::CTRL2);
  printEnclosedLSM303DHexRegister( "CTRL1", magAdr, LSM303D::CTRL1);
  printEnclosedLSM303DHexRegister( "CTRL5", magAdr, LSM303D::CTRL5);
  printEnclosedLSM303DHexRegister( "CTRL6", magAdr, LSM303D::CTRL6);
  printEnclosedLSM303DHexRegister( "CTRL7", magAdr, LSM303D::CTRL7);
}

void testReadCTRL1andCTRL2RegistersIndividually()
{
  uint8_t whoAmI;
  uint8_t regVal = 0x06;
  
  uint8_t ackBits[10];
  
  uint8_t ackBits1 = i2cSM.writeBytesTo( 
    magAdr, LSM303D::CTRL2, 
    SoftI2CMaster::i2c_read_or_write_1_byte, 
    &regVal, ackBits);

  delay( 1000); 
  printEnclosedBinData( "ackBit11", ackBits1 & 0x1); ackBits1 >>= 1;
  printEnclosedBinData( "ackBit12", ackBits1 & 0x1); ackBits1 >>= 1;
  printEnclosedBinData( "ackBit13", ackBits1 & 0x1); ackBits1 >>= 1;

  uint8_t ackBits2 = i2cSM.readBytesFrom( 
    magAdr, LSM303D::CTRL1, SoftI2CMaster::i2c_read_or_write_1_byte, &whoAmI);
  
  delay( 1000);
  printEnclosedBinData( "ackBit21", ackBits2 & 0x1); ackBits2 >>= 1;
  printEnclosedBinData( "ackBit22", ackBits2 & 0x1); ackBits2 >>= 1;
  printEnclosedBinData( "ackBit23", ackBits2 & 0x1); ackBits2 >>= 1;
}

void testWriteBytesToAndReadBytesFrom()
{
  uint8_t magAdr = 0x1e;
  uint8_t magValues[6] = {0x25,0xb2,0x18,0x24,0x9c,0x30};
  uint8_t ackBits[10]  = {11,12,13,14,15,16,17,18,19,20};

  uint8_t numAckBits = i2cSM.writeBytesTo( 
    magAdr, LSM303D::CTRL1, SoftI2CMaster::i2c_read_or_write_6_bytes,
    magValues, ackBits);

  delay( 2000);
  printEnclosedInt8( "numAckBits", numAckBits);  
  printEnclosedHexData( "ackBits[0]", ackBits[0]);
  printEnclosedHexData( "ackBits[1]", ackBits[1]);
  printEnclosedHexData( "ackBits[2]", ackBits[2]);
  printEnclosedHexData( "ackBits[3]", ackBits[3]);
  printEnclosedHexData( "ackBits[4]", ackBits[4]);
  printEnclosedHexData( "ackBits[5]", ackBits[5]);
  printEnclosedHexData( "ackBits[6]", ackBits[6]);
  printEnclosedHexData( "ackBits[7]", ackBits[7]);

  uint8_t accValues[6] = {0x99,0x98,0x97,0x96,0x95,0x94};
  uint8_t ackBits3 = 
    i2cSM.readBytesFrom( 
      magAdr, LSM303D::CTRL1, SoftI2CMaster::i2c_read_or_write_6_bytes, accValues);

  delay( 2000);
  printEnclosedHexData( "accValues[0]", accValues[0]);
  printEnclosedHexData( "accValues[1]", accValues[1]);
  printEnclosedHexData( "accValues[2]", accValues[2]);
  printEnclosedHexData( "accValues[3]", accValues[3]);
  printEnclosedHexData( "accValues[4]", accValues[4]);
  printEnclosedHexData( "accValues[5]", accValues[5]);
  printEnclosedBinData( "ackBit31", ackBits3 & 0x1); ackBits3 >>= 1;
  printEnclosedBinData( "ackBit32", ackBits3 & 0x1); ackBits3 >>= 1;
  printEnclosedBinData( "ackBit33", ackBits3 & 0x1); ackBits3 >>= 1;  
}

