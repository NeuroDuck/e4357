/*
 * SoftI2CMaster.cpp -- Multi-instance software I2C Master library
 * 
 * 2010-12 Tod E. Kurt, http://todbot.com/blog/
 *
 * This code takes some tricks from:
 *  http://codinglab.blogspot.com/2008/10/i2c-on-avr-using-bit-banging.html
 *
 * 2014, by Testato: update library and examples for follow Wire’s API of Arduino IDE 1.x
 *
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

#define i2cbitdelay 1

uint8_t SoftI2CMaster::bytesAvailable = 0;
uint8_t SoftI2CMaster::bufferIndex    = 0;
uint8_t SoftI2CMaster::totalBytes     = 0;

// Constructor
//
SoftI2CMaster::SoftI2CMaster()
{
    // do nothing, use setPins() later.
}

SoftI2CMaster::SoftI2CMaster( uint8_t sclPin, uint8_t sdaPin)
{
	SoftI2CMaster( 
		sclPin, sdaPin, 
		SoftI2CMaster::i2c_internal_pullups,
		SoftI2CMaster::i2c_scl_pulled_up
	);
}

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
	sclIsPulledUpOrNotType sclIsPulledUpOrNot
)
{
    uint8_t port;
    
    _usePullups     = pullups;
	_sclIsPulledUpOrNot = sclIsPulledUpOrNot;

    _sclPin = sclPin;
    _sdaPin = sdaPin;

	printEnclosedInt8( "sclPin", sclPin);
	printEnclosedInt8( "sdaPin", sdaPin);
	Serial.println( "");

    _sclBitMask = digitalPinToBitMask( sclPin);
	_sclNotBitMask = ~_sclBitMask;

    _sdaBitMask = digitalPinToBitMask( sdaPin);
	_sdaNotBitMask = ~_sdaBitMask;

	printEnclosedHexData( "_sclBitMask",    _sclBitMask);
	printEnclosedHexData( "_sclNotBitMask", _sclNotBitMask);
	printEnclosedHexData( "_sdaBitMask",    _sdaBitMask);
	printEnclosedHexData( "_sdaNotBitMask", _sdaNotBitMask);
	Serial.println( "");

    _sclPort        = digitalPinToPort( sclPin);
    _sclPortInReg   = portInputRegister( _sclPort);			// I.e., &PINC, x == Toggle Value.
    _sclPortOutReg  = portOutputRegister( _sclPort);		// I.e., &PORTC, x == Output Value.
    _sclDirReg      = portModeRegister( _sclPort);			// I.e., &DDRC, 1 == Output.

	printEnclosedInt8( "_sclPort", _sclPort);
	printEnclosedHexData( "_sclPortInReg", (long)_sclPortInReg);
	printEnclosedHexData( "_sclPortOutReg", (long)_sclPortOutReg);
	printEnclosedHexData( "_sclDirReg", (long)_sclDirReg);

    _sdaPort        = digitalPinToPort( sdaPin);
    _sdaPortInReg   = portInputRegister( _sdaPort);
    _sdaPortOutReg  = portOutputRegister( _sdaPort);
    _sdaDirReg      = portModeRegister( _sdaPort);

	printEnclosedInt8( "_sdaPort", _sdaPort);
	printEnclosedHexData( "_sdaPortInReg", (long)_sdaPortInReg);
	printEnclosedHexData( "_sdaPortOutReg", (long)_sdaPortOutReg);
	printEnclosedHexData( "_sdaDirReg", (long)_sdaDirReg);	

	_bothHiDirMask  = ~(_sdaBitMask | _sclBitMask);
	_bothHiPortMask =  (_sdaBitMask | _sclBitMask);

	_bothLoPortMask = ~(_sdaBitMask | _sclBitMask);
    _bothLoDirMask  =  (_sdaBitMask | _sclBitMask);

    i2c_init();
}

// Init bitbanging port, must be called before using the functions below.
// private:
void SoftI2CMaster::i2c_init(void)
{
	// SDA has to be Input+Pull-Up so the Sensor can drive it.
	i2c_sda_hi();

	// We need to do this once at the beginning if we're not using Internal Pull-Ups.
	if (_sclIsPulledUpOrNot == SoftI2CMaster::i2c_scl_not_pulled_up)
		i2c_scl_is_output();

	// SCL only needs to be if we want to allow the Sensor to stretch SCL by holding 
	// it low.
	i2c_scl_hi();

	_delay_us( 6);		// Let both lines sit HIGH for ~10 uS. to stabilize.
}

uint8_t SoftI2CMaster::beginTransmission( uint8_t address)
{
    return beginTransmission( address, i2c_rw_bit_is_write);
}

inline uint8_t shiftInReadNotWriteBit( uint8_t address, SoftI2CMaster::readBitNotWriteBitType readNotWrite)
{
	uint8_t shiftedAddress = address << 1;
	uint8_t readNotWriteAddress = bitWrite( shiftedAddress, 0, readNotWrite);

	return readNotWriteAddress;
}

// Send:
// 	ST, SAD+W (when called w/i2c_rw_bit_is_write)
// Confirm receipt of:
//	SAK
//
// Usage:
//	ackBitReturned = beginTransmission( address, i2c_rw_bit_is_write);
//  if (ackBitReturned != i2c_ack) { print "NAK"; return; }
//	
// 
uint8_t SoftI2CMaster::beginTransmission( 
	uint8_t address, readBitNotWriteBitType readNotWrite)
{
    i2c_start();
	
	return SoftI2CMaster::i2c_nak;	// Just for Logic Analyzer.

	uint8_t readNotWriteAddress = shiftInReadNotWriteBit( address, readNotWrite);
	
	Serial.print( "Writing to: 0x");
	Serial.println( readNotWriteAddress, HEX);
	
	transmittingInProgress = true;
	
    return i2c_write( readNotWriteAddress);
}

// Send a START Condition
// private:
void SoftI2CMaster::i2c_start(void)
{
	i2c_init();

    i2c_sda_lo();

	// Wait th(ST) uS., as per Table 7, pg. 14, in:
	// e4357\project\Compass\LSM303D-datasheet.pdf.
    _delay_us( 10);

    i2c_scl_lo();

    _delay_us( I2CSETTLINGTIME);			// Give things time to settle.
}

inline uint8_t setSubAdrAutoIncBit( 
	uint8_t address, SoftI2CMaster::autoIncSubAdrBitType autoIncSubAdr)
{
	uint8_t addressWithAutoIncSubAdrBit = bitWrite( address, 7, autoIncSubAdr);

	return addressWithAutoIncSubAdrBit;
}

uint8_t SoftI2CMaster::i2c_writeSubAddress( 
	uint8_t subAddress, autoIncSubAdrBitType autoIncSubAdr)
{
	uint8_t addressWithAutoIncSubAdrBit = 
		setSubAdrAutoIncBit( subAddress, autoIncSubAdr);
		
	Serial.print( "Writing to subAddress: ");
	Serial.println( addressWithAutoIncSubAdrBit, HEX);

	i2c_write( addressWithAutoIncSubAdrBit);
}

// write a byte to the I2C slave device
// private:
uint8_t SoftI2CMaster::i2c_write( uint8_t c)
{
    for (uint8_t i = 0 ; i < 8 ; i++)
	{
        i2c_writebit( c & 0x80);
        c <<= 1;
    }

    return i2c_readbit();
}

// private:
void SoftI2CMaster::i2c_writebit( uint8_t c)
{
    if (c > 0) 
        i2c_sda_hi();
	else
        i2c_sda_lo();

    i2c_scl_hi();				// Why spike it up high-then-back-to-low?
    _delay_us( i2cbitdelay);

    i2c_scl_lo();
    _delay_us( i2cbitdelay);

    if (c > 0)
        i2c_sda_lo();
   
    _delay_us( i2cbitdelay);
}

// private:
uint8_t SoftI2CMaster::i2c_readbit(void)
{
    i2c_sda_hi();
    i2c_scl_hi();
    _delay_us( i2cbitdelay);

    volatile uint8_t* pinReg = portInputRegister( _sdaPort);
    uint8_t c = *pinReg;  // I2C_PIN;

    i2c_scl_lo();
    _delay_us( i2cbitdelay);

    return (c & _sdaBitMask) ? i2c_nak : i2c_ack;
}

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
/*
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
*/
uint8_t SoftI2CMaster::readLast()
{
    return i2c_read( i2c_nak);
}

#define MR_DATA_ACK     0x50
#define MR_DATA_NACK    0x58
#define MAX_BUFFER_SIZE 32

uint8_t data[MAX_BUFFER_SIZE];

uint8_t SoftI2CMaster::read( uint8_t address, uint8_t numberBytes)
{
  bytesAvailable = 0;
  bufferIndex    = 0;

  if (numberBytes == 0)
	numberBytes++;  

  int returnStatus = 0;
  /* returnStatus = */ i2c_start();
  
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

uint8_t SoftI2CMaster::endTransmission(void)
{
    i2c_stop();
    //return ret;  // FIXME
	
	transmittingInProgress = false;

    return 0;
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

//--------------------------------------------------------------------

// private:
void SoftI2CMaster::i2c_repstart(void)
{
    // Set both to high at the same time (releases drive on both lines).
	//
	i2c_sda_hi();
    i2c_scl_hi();

    i2c_scl_lo();                           // force SCL low
    _delay_us( i2cbitdelay);

    i2c_sda_release();                      // release SDA
    _delay_us( i2cbitdelay);

    i2c_scl_release();                      // release SCL
    _delay_us( i2cbitdelay);

    i2c_sda_lo();                           // force SDA low
    _delay_us( i2cbitdelay);
}

// Send a STOP Condition
//
void SoftI2CMaster::i2c_stop(void)
{
    i2c_scl_hi();
    _delay_us( i2cbitdelay);

    i2c_sda_hi();
    _delay_us( i2cbitdelay);
}

// read a byte from the I2C slave device
// private:
uint8_t SoftI2CMaster::i2c_read( ackNotNackType ack)
{
    uint8_t res = 0;

    for (uint8_t i = 0 ; i < 8 ; i++) 
	{
        res <<= 1;
        res |= i2c_readbit();  
    }

    if ( ack )
        i2c_writebit( 0);
    else
        i2c_writebit( 1);

    _delay_us( i2cbitdelay);

    return res;
}

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
