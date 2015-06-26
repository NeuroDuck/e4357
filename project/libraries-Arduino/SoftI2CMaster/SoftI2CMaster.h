/*
 * SoftI2CMaster.h -- Multi-instance software I2C Master library
 * 
 * 2010-2012 Tod E. Kurt, http://todbot.com/blog/
 * 2014, by Testato: update library and examples for follow Wire’s API of Arduino IDE 1.x
 */
#ifndef SoftI2CMaster_h
#define SoftI2CMaster_h

#include <inttypes.h>
#include <util/delay.h>

#define _SOFTI2CMASTER_VERSION 14  // software version of this library

class SoftI2CMaster
{
public:
  enum readBitNotWriteBitType         {i2c_rw_bit_is_write  = 0, i2c_rw_bit_is_read   = 1};
  enum ackNotNackType                 {i2c_ack              = 0, i2c_nak              = 1};
  enum autoIncSubAdrBitType           {i2c_no_auto_inc_sub  = 0, i2c_auto_inc_sub     = 1 << 7};
  enum internalNotExternalPullupsType {i2c_external_pullups = 0, i2c_internal_pullups = 1};
  enum sclIsPulledUpOrNotType		  {i2c_scl_not_pulled_up = 0,   i2c_scl_pulled_up = 1};
  enum numBytesToReadOrWriteType	  {i2c_read_or_write_1_byte = 1, i2c_read_or_write_6_bytes = 6};
  
  // set SDA high and to input (releases pin) (i.e. change to input,turnon pullup)
  //
  // As per:
  //
  //	http://www.arduino.cc/en/Reference/PortManipulation
  //
  // _sclPortReg, i.e. == &PORTC, x == Output Value.
  // _sclDirReg,  i.e. == &DDRC,  1 == Output.
  //
  uint8_t _bothHiDirMask;
  uint8_t _bothHiPortMask;

// From e4357\project\Compass\LSM303D-datasheet.pdf, Table 7, pg. 14,
// twSCLL & twSCLH - 4-5 uS.
//
#define i2cIntraBitDelayUs 4

  // Because of I2C's "Wired-OR" approach, a HIGH output is accomplished by alternating 
  // between these two states:
  // 	Pin is an Input, with its Pull-Up Resistor Enabled.
  // and:
  // 	Pin is an Output.
  // So the _hi() function will do:
  //	Input:						Clear the Pin's Direction Bit.
  //	Pull-Up Resistor Enabled:	Write a 1 to the Pin.
  // The _lo() function will do:
  //	Output:						Set the Pin's Direction Bit.
  //	Pull-Up Resistor Disabled:	By switching to be an Output.
  //
  // Which is the best order of steps to take to switch from Input with Pull-Up a
  // to Output LOW?
  // The ATmega328P datsheet discusses this in 14.2.3, pg. 77, 2nd paragraph.
  //  
  // Test to see if the Pull-Up remains Enabled, after switching to Output then back to Input.
  //
  // #define PINC _SFR_IO8( 0x06) #define DDRC _SFR_IO8( 0x07) #define PORTC _SFR_IO8( 0x08)
  //  
  void printSclDirRegValue();
  void printSclPortOutRegValue();

  uint8_t _sclNotBitMask;

  inline void i2c_scl_is_input()
  {
	*_sclDirReg &= _sclNotBitMask;							// <== Clear bit == Input.
  }
  inline void i2c_scl_is_output()
  {
	*_sclDirReg |= _sclBitMask; 							// <== Set   bit ==> Output.
  }
  inline void i2c_sda_is_input()
  {
	*_sdaDirReg &= _sdaNotBitMask;							// <== Clear bit == Input.
  }
  inline void i2c_sda_is_output()
  {
	*_sdaDirReg |= _sdaBitMask; 							// <== Set   bit ==> Output.
  }

  inline void i2c_scl_write_lo()
  {
  	*_sclPortOutReg &= _sclNotBitMask;						// <== Clear bit ==> value == 0.
  }
  inline void i2c_scl_write_hi()
  {
	*_sclPortOutReg |= _sclBitMask; 						// <== Set   bit == value == 1.
  }
/*
  inline void i2c_scl_toggle()		// Will this be needed to shorten the SCL pulses?
  {
	*_sclPortInReg |= _sclBitMask; 							// <== Set   bit == value == Toggle.
  }
*/
  uint8_t _sdaNotBitMask;

  inline void i2c_sda_write_lo()
  {
  	*_sdaPortOutReg &= _sdaNotBitMask;						// <== Clear bit ==> value == 0.
  }
  inline void i2c_sda_write_hi()
  {
	*_sdaPortOutReg |= _sdaBitMask; 						// <== Set   bit == value == 1.
  }
  inline void i2c_sda_toggle()
  {
	*_sdaPortInReg |= _sdaBitMask; 							// <== Set   bit == value == Toggle.
  }

  // Set SCL LOW and switch Pin to be Output.
  inline void i2c_scl_lo()                                 
  {
  	if (_sclIsPulledUpOrNot == SoftI2CMaster::i2c_scl_pulled_up)
	{
	  i2c_scl_is_output();					// Need to switch only if using Pull-Ups.
	}
	i2c_scl_write_lo();
  }

  // If using Internal Pull-Ups:
  // 	Switch us to be Input    (= write to DirReg), 
  //	then turn on the Pull-Up (= write to PortOutReg).
  // If using External Pull-Ups:
  //	Just switch us to Input  (= write to DirReg), so the Pull-Up can do its pulling-up.
  //	and leave the Internal Pull-Up Resistor disconnected.
  //
  inline void i2c_scl_hi()                                 
  {
	// SCL needs to be Input+Pull-Up if we want to allow the Sensor to stretch SCL.
	//
	if (_sclIsPulledUpOrNot == SoftI2CMaster::i2c_scl_pulled_up)
	{
	  i2c_scl_is_input();
	
	  // Using Internal rather than External Pull-Up Resistors for our I2C bus.
	  //
	  if (_usePullups == i2c_internal_pullups) 
	  {
		i2c_scl_write_hi();									// <== Set   bit == Pull-Up is on.
	  }
	}
	else	// We're always Output for this case, so no need to change the direction.
	{
	  i2c_scl_write_hi();
	}
  }

  // sets SDA low and drives output
  inline void i2c_sda_lo()                                
  {
	i2c_sda_is_output();
	i2c_sda_write_lo();
  }
					  
  inline void i2c_sda_hi()
  {
	i2c_sda_is_input();
	i2c_sda_write_hi();						// Turn on the Pull-Up.
  }

private:
  // per object data
  uint8_t _sclPin;
  uint8_t _sdaPin;
  uint8_t _sclBitMask;
  uint8_t _sdaBitMask;

  uint8_t _sclPort;
  uint8_t _sdaPort;
//  volatile uint8_t *_sclPortInReg;	// Not needed.
  volatile uint8_t *_sdaPortInReg;
  volatile uint8_t *_sclPortOutReg;
  volatile uint8_t *_sdaPortOutReg;
  volatile uint8_t *_sclDirReg;
  volatile uint8_t *_sdaDirReg;
  
  static uint8_t bytesAvailable;
  static uint8_t bufferIndex;
  static uint8_t totalBytes;

  internalNotExternalPullupsType _usePullups;
  sclIsPulledUpOrNotType		 _sclIsPulledUpOrNot;
  bool transmittingInProgress;
  
  // private methods
													// x = Checked in Logic Analyzer.
  inline void i2c_init(void);						// x.
  inline void i2c_stop(void);						// x.
  inline void i2c_repstart(void);					// x.
  inline void i2c_start(void);						// x.
  inline void i2c_writebit( uint8_t c);				// x.	
  inline uint8_t i2c_read( ackNotNackType ack);		// x.
  inline ackNotNackType i2c_readbit(void);			// x.

  inline uint8_t shiftInReadNotWriteBit( 
	uint8_t address, readBitNotWriteBitType readNotWriteBit);

  inline uint8_t setSubAdrAutoIncBit( 
	uint8_t address, autoIncSubAdrBitType autoIncSubAdrBit);

public:
  ackNotNackType i2c_writeSubAddress(
	uint8_t subAddress, autoIncSubAdrBitType autoIncSubAdrBit);
	
  // public methods
  SoftI2CMaster( 
	uint8_t sclPin, uint8_t sdaPin, 
	internalNotExternalPullupsType usePullups, 
	sclIsPulledUpOrNotType sclIsPulledUpOrNot
  );

  void setPins( 
	uint8_t sclPin, uint8_t sdaPin, 
	internalNotExternalPullupsType usePullups,
	sclIsPulledUpOrNotType sclIsPulledUpOrNot
  );

  ackNotNackType beginTransmission(
	uint8_t address, readBitNotWriteBitType readNotWrite = i2c_rw_bit_is_write);

  uint8_t readBytesFrom( 
	uint8_t address, uint8_t subAddress, numBytesToReadOrWriteType numBytesToRead, 
	uint8_t* registerValues);

  uint8_t writeBytesTo( 
	uint8_t address, uint8_t subAddress, numBytesToReadOrWriteType numBytesToWrite, 
	uint8_t* registerValues, uint8_t* ackBits);

  ackNotNackType i2c_write( uint8_t c);		// x.

  void endTransmission(void);

  uint8_t write( uint8_t) { }	// LSM303D library is temporarily using this.

/* May be needed in some form by the LSM303D library...
 *
  void write( uint8_t*, uint8_t);
  void write( int);
  void write( char*);

  uint8_t requestFrom( int address);
  uint8_t requestFrom( uint8_t address);
  uint8_t requestFrom( int address, int numberBytes);
*/
  uint8_t requestFrom( uint8_t address, uint8_t numberBytes) {}

  uint8_t read( ackNotNackType ack);
  uint8_t read() {}
  uint8_t read( uint8_t address, uint8_t numberBytes);
  uint8_t readLast();

  uint8_t available();
};

#endif
