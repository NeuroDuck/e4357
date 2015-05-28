#include "mbed.h"
#include <LSM303.h>

// NOTE: In LSM303.cpp, we changed this line frpm:
//       I2C i2c( P0_4, P0_5);
// to:
// #define SDAPIN p29
// #define SCLPIN p30
// SoftwareI2C i2c( SDAPIN, SCLPIN); 

// Usage:
//	1. Hook up the LSM303 Compass to your desired I2C pins.
//	   I only used the SoftwareI2C library because both I2C pin sets are already 
//	   in use for other purposes on my m3pi mbed Robot (i.e., comm. w/3pi "1st floor",
//	   and comm. w/XBee).  You are free to change the "SoftwareI2C" line in LSM303.cpp
//	   back to "I2C".  
//	   I had to make some changes to LSM303.cpp&.h to make it compatible with the
//	   SoftwareI2C library, so you should probably use the stock library instead.
//	   Note: It's called LSM303DM (not LSM303), and can be found here:
//			 https://developer.mbed.org/users/fin4478/code/LSM303DM/
//	2. Get this program running in your mbed, then _slowly_ rotate the LSM303 a full
//	   360-degrees around all 3 axes, several times.
//	   As you do this, you'll see the printed output gradually indicate bigger min
//	   and max values.
//	3. Once the values are no longer changing, no matter how many rotations you do, 
//	   enter  the printed values in LSM303DM\LSM303.cpp:setup() and re-compile it.  
//	   Doing this will do the first level of calibration for your Compass.  
//	   The next level of calibration, which I do not cover here, is to correct for 
//	   "Soft Iron" and "Hard Iron" errors that occur in the final installed 
//	   environment of your Compass, due to nearby metal and magnetic fields.
//
LSM303 lsm;

Serial pc( USBTX, USBRX);

int main() 
{
	pc.baud( 57600);

	lsm.setup();

	while (1)
	{
        lsm.getLSM303_mag();
		
		// Mag_min handler
        if (lsm.m.x  <  lsm.m_min.x)
            lsm.m_min.x  =  lsm.m.x;

        if (lsm.m.y  <  lsm.m_min.y)
            lsm.m_min.y  =  lsm.m.y;

        if (lsm.m.z  <  lsm.m_min.z)
            lsm.m_min.z  =  lsm.m.z;

		// Mag_max handler
        if (lsm.m.x  >  lsm.m_max.x)
            lsm.m_max.x  =  lsm.m.x;

        if (lsm.m.y  >  lsm.m_max.y)
            lsm.m_max.y =   lsm.m.y;

        if (lsm.m.z  >  lsm.m_max.z)
            lsm.m_max.z  =  lsm.m.z;

        wait( 0.1);
				
		pc.printf(
			"m_min.x = %.0f; m_min.y = %.0f; m_min.z = %.0f; m_max.x = %.0f; m_max.y = %.0f; m_max.z = %.0f;\r\n",
			lsm.m_min.x, lsm.m_min.y,    lsm.m_min.z,    lsm.m_max.x,    lsm.m_max.y,    lsm.m_max.z);

		lsm.getLSM303_accel();

		// Acc_min handler
        if (lsm.a.x  <  lsm.a_min.x)
            lsm.a_min.x  =  lsm.a.x;

        if (lsm.a.y  <  lsm.a_min.y)
            lsm.a_min.y  =  lsm.a.y;

        if (lsm.a.z  <  lsm.a_min.z)
            lsm.a_min.z  =  lsm.a.z;

		// Acc_max handler
        if (lsm.a.x  >  lsm.a_max.x)
            lsm.a_max.x  =  lsm.a.x;

        if (lsm.a.y  >  lsm.a_max.y)
            lsm.a_max.y =   lsm.a.y;

        if (lsm.a.z  >  lsm.a_max.z)
            lsm.a_max.z  =  lsm.a.z;

        wait( 0.1);
				
		pc.printf(
			"a_min.x = %.0f; a_min.y = %.0f; a_min.z = %.0f; a_max.x = %.0f; a_max.y = %.0f; a_max.z = %.0f;\r\n",
			lsm.a_min.x, lsm.a_min.y,    lsm.a_min.z,    lsm.a_max.x,    lsm.a_max.y,    lsm.a_max.z);
	}
}
