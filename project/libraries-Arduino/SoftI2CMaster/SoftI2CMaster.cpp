/*
 * SoftI2CMaster.cpp -- Multi-instance software I2C Master library
 * 
 * 2010-12 Tod E. Kurt, http://todbot.com/blog/
 *
 * This code takes some tricks from:
 *  http://codinglab.blogspot.com/2008/10/i2c-on-avr-using-bit-banging.html
 *
 * 2014, by Testato: update library and examples for follow Wire’s API of Arduino IDE 1.x
 */

#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <SoftI2CMaster.h>
#include <UsefulFunctions.h>

#include <util/delay.h>
#include <string.h>

uint8_t SoftI2CMaster::bytesAvailable = 0;
uint8_t SoftI2CMaster::bufferIndex    = 0;
uint8_t SoftI2CMaster::totalBytes     = 0;

// Constructor
//
SoftI2CMaster::SoftI2CMaster( 
	uint8_t sclPin, uint8_t sdaPin, 
	internalNotExternalPullupsType pullups,
	sclIsPulledUpOrNotType sclIsPulledUpOrNot
) :	transmittingInProgress( false)
{
    setPins( sclPin, sdaPin, pullups, sclIsPulledUpOrNot);
}

// Turn Arduino pin numbers into PORTx, DDRx, and PINx
//
// _ourPortInReg  = >0x26<
// _ourDirReg     = >0x27<
// _ourPortOutReg = >0x28<
//
void SoftI2CMaster::setPins( 
	uint8_t sclPin, uint8_t sdaPin, 
	internalNotExternalPullupsType pullups, 
	sclIsPulledUpOrNotType sclIsPulledUpOrNot)
{
    uint8_t port;
    
    _usePullups     = pullups;
	_sclIsPulledUpOrNot = sclIsPulledUpOrNot;

    _sclPin = sclPin;
    _sdaPin = sdaPin;
/*
	printEnclosedInt8( "sclPin", sclPin);
	printEnclosedInt8( "sdaPin", sdaPin);
	Serial.println( "");
*/
    _sclBitMask = digitalPinToBitMask( sclPin);
	_sclNotBitMask = ~_sclBitMask;

    _sdaBitMask = digitalPinToBitMask( sdaPin);
	_sdaNotBitMask = ~_sdaBitMask;
/*
	printEnclosedHexData( "_sclBitMask",    _sclBitMask);
	printEnclosedHexData( "_sclNotBitMask", _sclNotBitMask);
	printEnclosedHexData( "_sdaBitMask",    _sdaBitMask);
	printEnclosedHexData( "_sdaNotBitMask", _sdaNotBitMask);
	Serial.println( "");
*/
    _sclPort        = digitalPinToPort( sclPin);
//    _sclPortInReg   = portInputRegister( _sclPort); Not needed. // I.e., &PINC, x == Toggle Value.
    _sclPortOutReg  = portOutputRegister( _sclPort);		// I.e., &PORTC, x == Output Value.
    _sclDirReg      = portModeRegister( _sclPort);			// I.e., &DDRC, 1 == Output.
/*
	printEnclosedInt8( "_sclPort", _sclPort);
	printEnclosedHexData( "_sclPortInReg", (long)_sclPortInReg);
	printEnclosedHexData( "_sclPortOutReg", (long)_sclPortOutReg);
	printEnclosedHexData( "_sclDirReg", (long)_sclDirReg);
*/
    _sdaPort        = digitalPinToPort( sdaPin);
    _sdaPortInReg   = portInputRegister( _sdaPort);
    _sdaPortOutReg  = portOutputRegister( _sdaPort);
    _sdaDirReg      = portModeRegister( _sdaPort);
/*
	printEnclosedInt8( "_sdaPort", _sdaPort);
	printEnclosedHexData( "_sdaPortInReg", (long)_sdaPortInReg);
	printEnclosedHexData( "_sdaPortOutReg", (long)_sdaPortOutReg);
	printEnclosedHexData( "_sdaDirReg", (long)_sdaDirReg);	
*/
	i2c_init();
}

// Init bitbanging port, must be called before using the functions below.
// See: "SCL-SDA-init()-LA_screenshot.gif".
//
// private:
inline void SoftI2CMaster::i2c_init(void)
{
	// SDA has to be Input+Pull-Up so the Sensor can drive it.
	i2c_sda_hi();

	// We need to do this once at the beginning if we're not using Internal Pull-Ups.
	if (_sclIsPulledUpOrNot == SoftI2CMaster::i2c_scl_not_pulled_up)
		i2c_scl_is_output();

	// SCL only needs to be if we want to allow the Sensor to stretch SCL by holding 
	// it low.
	i2c_scl_hi();		// Comes up 3.1uS. after SDA.

	_delay_us( 6);		// Let both lines sit HIGH for ~10 uS. to stabilize.
}

// -------------------------
// For STOP:
// -------------------------
// 1. delay, Drop SDA (it may already be LOW).
// 2. delay, Raise SCL.
// 3. delay, Raise SDA (tsu(SP) > 4 uS.).
//
// -------------------------
// tw(SP:SR) > 4.7 uS., delay before doing another START.
// The min. time it takes to get out of endTransmission()
// and back into beginTransmission() is 11.5uS., 
// so we're fine here.
// -------------------------
//
inline void SoftI2CMaster::i2c_stop(void)		
{
	// 1.								// Ugh, it takes 29uS. to get here,
//	_delay_us( i2cIntraBitDelayUs);		// so no need for any additional delay.
	i2c_sda_lo();
	
	// 2.
	_delay_us( i2cIntraBitDelayUs);		// (=4), this takes 8uS., so it's ok for now.
	i2c_scl_hi();
	
	// 3.
	_delay_us( i2cIntraBitDelayUs);		// (=4), this takes 7.5uS., so it's ok for now.
	i2c_sda_hi();
}

void SoftI2CMaster::endTransmission(void)
{
    i2c_stop();
    //return ret;  // FIXME
	
	transmittingInProgress = false;
}

// -------------------------
// For Repeated START:
// -------------------------
// 1. delay, Raise SDA.
// 2. delay, Raise SCL.
// These are in i2c_start():
// 3. delay, Drop SDA (tsu(SR) > 4.7 uS.).
// 4. delay, Drop SCL.
//
// private:
inline void SoftI2CMaster::i2c_repstart(void)
{
	// 1.							// Already done.  Prepare to
//	_delay_us( i2cIntraBitDelayUs);	// receive the Sensor's ACK.
//	i2c_sda_hi();
	
	// 2.
	_delay_us( i2cIntraBitDelayUs);
	i2c_scl_hi();
	
	// 3.							// Now we're set up to do another usual START,
	i2c_start();					// so call it here.
}

// Send a START Condition
// private:
inline void SoftI2CMaster::i2c_start(void)
{
    i2c_sda_lo();

	// Wait th(ST) > 4 uS., as per Table 7, pg. 14, in:
	// e4357\project\Compass\LSM303D-datasheet.pdf.  We're aiming for ~8 uS.
    _delay_us( i2cIntraBitDelayUs);

    i2c_scl_lo();
}

inline uint8_t SoftI2CMaster::shiftInReadNotWriteBit( 
	uint8_t address, readBitNotWriteBitType readNotWriteBit)
{
	uint8_t shiftedAddress = address << 1;

	// = bitWrite( shiftedAddress, 0, readNotWrite);
	uint8_t readNotWriteAddress = shiftedAddress | readNotWriteBit;

	return readNotWriteAddress;
}

// Send:
// 	ST, SAD+W (when called w/i2c_rw_bit_is_write)
// Confirm receipt of:
//	SAK
//
// Usage:
//	ackBitReturned = beginTransmission( address, SoftI2CMaster::i2c_rw_bit_is_write);
//  if (ackBitReturned != i2c_ack) { print "NAK"; return; }
//
inline SoftI2CMaster::ackNotNackType SoftI2CMaster::beginTransmission( 
	uint8_t address, readBitNotWriteBitType readNotWrite)
{
	if (transmittingInProgress)
		i2c_repstart();
	else
	{
		transmittingInProgress = true;
		i2c_start();
	}
	
	uint8_t readNotWriteAddress = 
		shiftInReadNotWriteBit( address, readNotWrite);

    return i2c_write( readNotWriteAddress);
}

SoftI2CMaster::ackNotNackType SoftI2CMaster::i2c_writeSubAddress( 
	uint8_t subAddress, autoIncSubAdrBitType autoIncSubAdrBit)
{
	uint8_t addressWithAutoIncSubAdrBit = 
		setSubAdrAutoIncBit( subAddress, autoIncSubAdrBit);

	return i2c_write( addressWithAutoIncSubAdrBit);
}

// private:
inline uint8_t SoftI2CMaster::setSubAdrAutoIncBit( 
	uint8_t address, autoIncSubAdrBitType autoIncSubAdrBit)
{
	// = bitWrite( address, 7, autoIncSubAdrBit);	// Hopefully address is only 7-bits. 

	if (autoIncSubAdrBit)
		address |= autoIncSubAdrBit;

	return address;
}

// This is working well without any delay()'s added.
//
// private:
inline SoftI2CMaster::ackNotNackType SoftI2CMaster::i2c_readbit(void)
{
	// Let SDA float (w/our Pull-Up enabled), so we can receive the Sensor's data.
	i2c_sda_hi();	

	// Tell the Sensor we are ready to read the next bit of its data now.
    i2c_scl_hi();

	// Oh boy, what did we receive?
    uint8_t c = *_sdaPortInReg;		// Did our Sensor ACK us? (0 == ACK, 1 == NACK).

	// Tell the Sensor that it's ok to clock out the next bit of its data now.
    i2c_scl_lo();					

    return (c & _sdaBitMask) ? i2c_nak : i2c_ack;
}

// -------------------------
// To clock out a Master ACK bit after reading 8-bits off of SDA:
// -------------------------
// 1. delay, Drop  SDA.
// 2. delay, Raise SCL.
// 3. delay, Drop  SCL.
// 4. delay, Raise SDA.
//
// private:
inline uint8_t SoftI2CMaster::i2c_read( ackNotNackType ack)
{
    uint8_t res = 0;
	
    for (uint8_t i = 0 ; i < 8 ; i++) 
	{
        res <<= 1;
        res |= i2c_readbit();  
    }

    if (ack == i2c_ack)					// Pull SDA LOW to ACK to the Sensor.
	{
        i2c_sda_lo();
		i2c_scl_hi();
		i2c_scl_lo();
		i2c_sda_hi();
	}
    else
        i2c_sda_hi();

    return res;
}

uint8_t SoftI2CMaster::readBytesFrom( 
	uint8_t address, uint8_t subAddress, numBytesToReadOrWriteType numBytesToRead, 
	uint8_t* registerValues)
{
  // Address the Sensor, to let it know that we're querying it.
  //
  ackNotNackType ackBit1 = beginTransmission( address);
  
  // Let it know which of its Registers we want to read from, and whether we're only 
  // after 1 byte or multiple bytes.
  //  
  autoIncSubAdrBitType autoIncBit = 
	(numBytesToRead > 1) ? i2c_auto_inc_sub : i2c_no_auto_inc_sub;

  ackNotNackType ackBit2 = i2c_writeSubAddress( subAddress, autoIncBit);

  // Clock out a Repeated-Start to read the desired Register's value.
  // Calling beginTransmission() a 2nd time before calling endTransmission()
  // will clock out a Repeated-Start instead of a Start.
  //
  // Also, tell beginTransmission() that we're reading this time, instead of writing,
  // which is its default action.
  //
  ackNotNackType ackBit3 = beginTransmission( address, i2c_rw_bit_is_read);
  
  // We've primed the Sensor to clock out its requested Register's value,
  // so let's read in our Sensor's Register's value(s).
  //
  // Here we read all but the last byte from the Sensor.
  //
  // At present we're only reading either 1 or 6 bytes.
  // After each read, change our pointer to read into the next destination byte.
  //
  // Theoretically, this switch() statement is better than a for() loop, as it 
  // doesn't have the processing every time through the loop to slow things down.
  //
  switch (numBytesToRead)
  {
  case 6:
	*(registerValues++) = i2c_read( i2c_ack);
  case 5:
	*(registerValues++) = i2c_read( i2c_ack);
  case 4:
	*(registerValues++) = i2c_read( i2c_ack);
  case 3:
	*(registerValues++) = i2c_read( i2c_ack);
  case 2:
	*(registerValues++) = i2c_read( i2c_ack);
  default:										// Only reading 1 byte.
	break;
  }

  // Now bat cleanup, and read the last (and possibly only) value from 
  // the Sensor's Register(s).
  //
  *registerValues = i2c_read( i2c_nak);

  // All done now, so clock out a STOP condition to return the I2C bus 
  // to an Idle state.
  //
  endTransmission();

  return (ackBit3 << 2 + ackBit2 << 1 + ackBit1);
}

// Write a byte to the I2C slave device.
// As this point, we've already called i2c_start(), meaning that SDA went LOW first,
// followed by SCL going LOW, which we've just returned from doing, just before 
// coming here.
//
// private:
SoftI2CMaster::ackNotNackType SoftI2CMaster::i2c_write( uint8_t c)
{
    for (uint8_t i = 0 ; i < 8 ; i++)
	{
        i2c_writebit( c & 0x80);
        c <<= 1;		
    }

    return i2c_readbit();
}

// The following is all for a 16MHz clock, i.e., on an Arduino Fio.
// 
// Delays will have to be added for a faster-clocked MCU, hopefully with #defines,
// such that they disappear when running on slow-clocked MCU's such as this one.
//
// We've identified i2cIntraBitDelayUs as the amount, prior to moving on to doing the 
// next step.  Each "d" below represents a i2cIntraBitDelayUs period.
//
// I.e., we're enterning here with both SDA & SCL LOW, so let's leave things as we found them.
// Along the way, delay i2cIntraBitDelayUs before doing each of these next steps:
// SDA:_7777
//     ddddd
// SCL:__^^_
//     1234
//
// To get from beginTransmission()->i2c_start() into:
//	  beginTransmission()->i2c_write()->i2c_writebit()
// takes 8.4uS., so there's no need for bit#7's delay #1. below.
// The following bits may need it though.  Bit #6 appears 4.0uS. before SCL->HIGH,
// which is lots of time in advance of the 250nS. when it's needed to appear.
//
// The transition between sda_write_hi() to sda_write_lo() is 3.3uS. in both dirs.
//
// private:
inline void SoftI2CMaster::i2c_writebit( uint8_t c)
{
	// 1.
//    _delay_us( i2cIntraBitDelayUs);
    if (c > 0)
        i2c_sda_hi();
	else
        i2c_sda_lo();

	// 2.
//    _delay_us( i2cIntraBitDelayUs);	// It's 5uS. later without this, 
    i2c_scl_hi();						// so no need for a delay here.

	// 3. Delay, then do nothing, as described above.
	//    (We're already plenty slow, so no need for any additional delays.) 
//    _delay_us( i2cIntraBitDelayUs * 1);	// i2cIntraBitDelayUs=4 * 1 gives 8.2uS., so good. 
	
	// 4.
    // _delay_us( i2cIntraBitDelayUs);  // Done in #3 just above.
    i2c_scl_lo();
	
	// Here is a problem.  The LSM303D datasheet says in its Table 7, pg. 14,
	// that th(SDA) (= holding time between SCL->LOW and the next SDA bit,
	// should be < 3.45uS.
	// And with the current implementation, it is 5.8uS.
	// For the moment, we'll see if the LSM303D can stomach that, and if not,
	// we'll try to tighten up the i2c_write()->i2c_writebit() loop.

	// Update: It turns out that the Compass seems to be happy with our current 
	//		   (slooow) implementation, so leave everything as is for now.
}

uint8_t SoftI2CMaster::writeBytesTo(	
	uint8_t address, uint8_t subAddress, numBytesToReadOrWriteType numBytesToWrite, 
	uint8_t* registerValues, uint8_t* ackBits)
{
  uint8_t ackBitNdx = 0;
  
  // Address the Sensor, to let it know that we're writing to it.
  //
  ackBits[ackBitNdx++] = beginTransmission( address);
  
  // Let it know which of its Registers we want to read from, and whether we're only 
  // after 1 byte or multiple bytes.
  //  
  autoIncSubAdrBitType autoIncBit = 
	(numBytesToWrite > 1) ? i2c_auto_inc_sub : i2c_no_auto_inc_sub;

  ackBits[ackBitNdx++] = i2c_writeSubAddress( subAddress, autoIncBit);

  // Clock out the data bytes to store into the Compass's specified Register(s).
  //
  
  switch (numBytesToWrite)
  {
  case 6:
	ackBits[ackBitNdx++] = i2c_write( *(registerValues++));
  case 5:
	ackBits[ackBitNdx++] = i2c_write( *(registerValues++));
  case 4:
	ackBits[ackBitNdx++] = i2c_write( *(registerValues++));
  case 3:
	ackBits[ackBitNdx++] = i2c_write( *(registerValues++));
  case 2:
	ackBits[ackBitNdx++] = i2c_write( *(registerValues++));
  default:													// Only writing 1 byte.
	break;
  }
  ackBits[ackBitNdx++] = i2c_write( *(registerValues++));

  // All done now, so clock out a STOP condition to return the I2C bus 
  // to an Idle state.
  //
  endTransmission();
  
  return ackBitNdx;
}

/* May be needed in some form by the LSM303D library...
 *
uint8_t SoftI2CMaster::requestFrom( int address)
{
    return requestFrom( (uint8_t)address);
}

uint8_t SoftI2CMaster::requestFrom( uint8_t address)
{
    i2c_start();
	
	uint8_t readNotWriteAddress = shiftInReadNotWriteBit( address, i2c_rw_bit_is_read);

    return i2c_write( readNotWriteAddress); 
}

uint8_t SoftI2CMaster::requestFrom( int address, int numberBytes)
{
  return requestFrom( (uint8_t)address, (uint8_t)numberBytes);
}

uint8_t SoftI2CMaster::requestFrom( uint8_t address, uint8_t numberBytes)
{
  int returnStatus = read( address, numberBytes);

  return (returnStatus) ? 0 : numberBytes;
}

// FIXME: this isn't right, surely
uint8_t SoftI2CMaster::read( ackNotNackType ack)
{
  return i2c_read( ack);
}

//
uint8_t SoftI2CMaster::read()
{
    return i2c_read( i2c_ack);
}

uint8_t SoftI2CMaster::readLast()
{
    return i2c_read( i2c_nak);
}
*/
#define MR_DATA_ACK     0x50
#define MR_DATA_NACK    0x58
#define MAX_BUFFER_SIZE 32

uint8_t data[MAX_BUFFER_SIZE];

/* May be needed in some form by the LSM303D library...
 *
uint8_t SoftI2CMaster::read( uint8_t address, uint8_t numberBytes)
{
  bytesAvailable = 0;
  bufferIndex    = 0;

  if (numberBytes == 0)
	numberBytes++;  

  int returnStatus = 0;
  // returnStatus = 
  i2c_start();
  
  if (returnStatus)
	return returnStatus;
  
  returnStatus = beginTransmission( address);
  
  if (returnStatus)
  {
    if (returnStatus == 1)
	{
	  return 5;
	}
    return( returnStatus);
  }
  
  int nack = numberBytes - 1;
  
  for (uint8_t i = 0 ; i < numberBytes ; i++)
  {
    if (i == nack)
    {
	  returnStatus = i2c_read( i2c_nak);

      if (returnStatus == 1)
	  {
		return 6;
	  }

      if (returnStatus != MR_DATA_NACK)
	  {
		return returnStatus;
	  }
    }
    else
    {
	  returnStatus = i2c_read( i2c_ack);
	  
      if (returnStatus == 1)
	  {
		return 6;
	  }
      if (returnStatus != MR_DATA_ACK)
	  {
	    return returnStatus;
	  }
    }
	
    data[i] = returnStatus;
	
    bytesAvailable = i + 1;
    totalBytes     = i + 1;
  }

  returnStatus = endTransmission();

  if (returnStatus)
  {
    if (returnStatus == 1)
	{
		return(7);
	}
    return returnStatus;
  }
  
  return returnStatus;
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
//
uint8_t SoftI2CMaster::write( uint8_t data)
{
    return i2c_write( data);
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
//
void SoftI2CMaster::write( uint8_t* data, uint8_t quantity)
{
    for (uint8_t i = 0; i < quantity; ++i)
	{
        write( data[i]);
    }
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
//
void SoftI2CMaster::write( char* data)
{
    write( (uint8_t*)data, strlen( data));
}

// must be called in:
// slave tx event callback
// or after beginTransmission(address)
//
void SoftI2CMaster::write( int data)
{
    write( (uint8_t)data);
}
*/
//--------------------------------------------------------------------

uint8_t SoftI2CMaster::available()
{
  return bytesAvailable;
}

void SoftI2CMaster::printSclDirRegValue()
{
	printEnclosedHexData( "_sclDirReg value", (long)(*_sclDirReg));  
}

void SoftI2CMaster::printSclPortOutRegValue()
{
	printEnclosedHexData( "_sclPortOutReg value", (long)(*_sclPortOutReg));
}
