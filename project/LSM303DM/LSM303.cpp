#include "mbed.h"
#include <math.h>
#include "LSM303.h"
#include "SoftwareI2C.h"

#define SDAPIN p29
#define SCLPIN p30

SoftwareI2C i2c( SDAPIN, SCLPIN); 
// I2C i2c( SDAPIN, SCLPIN);

#define char uint8_t

void LSM303::setup()
{
#ifdef CALIBRATING //set in LSM303.h
    m_max.x = 1;
    m_max.y = 1;
    m_max.z = 1;
    m_min.x = 0;
    m_min.y = 0;
    m_min.z = 0;
    
    a_max.x = 1;
    a_max.y = 1;
    a_max.z = 1;
    a_min.x = 0;
    a_min.y = 0;
    a_min.z = 0;
    
#else
		m_min.x = -690;
		m_min.y = -702;
		m_min.z = -568;
		m_max.x = 480;
		m_max.y = 483;
		m_max.z = 589;
	
		a_min.x = -17376; 
		a_min.y = -22624; 
		a_min.z = -25024; 
		a_max.x = 16432; 
		a_max.y = 18464; 
		a_max.z = 29840;
/*    
    a_min.x = -542;
    a_min.y = -644;
    a_min.z = -632; 
    a_max.x = 496;
    a_max.y = 472;
    a_max.z = 566;
*/
#endif
    getScale(&scale);
    LSM303_write(0x27, CTRL_REG1_A);
    LSM303_write(0x00, CTRL_REG4_A);
    LSM303_write(MAG_SCALE_1_3 , CRB_REG_M); //magnetic scale = +/-1.3Gauss
    LSM303_write(0x00, MR_REG_M);  // 0x00 = continouous conversion mode
}

int LSM303::testAcc()
{
    if (i2c.write(LSM303_ACC, NULL, 0) ==0) return LSM303_ACC;
    return 255;
}

int LSM303::testMag()
{
    if (i2c.write(LSM303_MAG, NULL, 0) ==0)
		{
        if (LSM303_read(LSM303_WHO_AM_I_M)==0x3C) 
					{
            return LSM303_WHO_AM_I_M;
        } else {
            return LSM303_MAG;
        }
			}

    return 255;
}


void LSM303::getScale(Plane *scale)  {
    Plane vmax;
    Plane vmin;
    Plane avgs;
    //First the hard iron errors are removed from the maximums and minimum magnetometer vectors. 
    //These minimum and maximum vectors are the same as the ones being used to correct for hard iron errors.
    vmax.x= m_max.x - ((m_min.x + m_max.x)/2.0);
    vmax.y= m_max.y - ((m_min.y + m_max.y)/2.0);
    vmax.z =m_max.z - ((m_min.z + m_max.z)/2.0);

    vmin.x = m_min.x - ((m_min.x + m_max.x)/2.0);
    vmin.y = m_min.y - ((m_min.y + m_max.y)/2.0);
    vmin.z = m_min.z - ((m_min.z + m_max.z)/2.0);

    //The average distance from the centre is now calculated. We want to know how far from the centre, so the negative values are inverted.
    //avgs = vmax + (vmin*-1); //multiply by -1 to make negative values positive
    //avgs = avgs / 2.0;
    avgs.x = (vmax.x + vmin.x*-1)/2.0;
    avgs.y = (vmax.y + vmin.y*-1)/2.0;
    avgs.z = (vmax.z + vmin.z*-1)/2.0;
    //The components are now averaged out
    float avg_rad = avgs.x + avgs.y + avgs.z;
    avg_rad /= 3.0;
    //Finally calculate the scale factor by dividing average radius by average value for that axis.
    scale->x = (avg_rad/avgs.x);
    scale->y = (avg_rad/avgs.y);
    scale->z = (avg_rad/avgs.z);
}

float LSM303::getTiltHeading()
{
    getLSM303_accel();        
    getLSM303_mag();  // get the accel and magnetometer values, store them in a and m
    
    a.x -= ((int32_t)a_min.x + a_max.x) / 2;
    a.y -= ((int32_t)a_min.y + a_max.y) / 2;
    a.z -= ((int32_t)a_min.z + a_max.z) / 2;
    
    // subtract offset (average of min and max) from magnetometer readings
    m.x -= ((int32_t)m_min.x + m_max.x) / 2;
    m.y -= ((int32_t)m_min.y + m_max.y) / 2;
    m.z -= ((int32_t)m_min.z + m_max.z) / 2;
    
    m.x *= scale.x;
    m.y *= scale.y;
    m.z *= scale.z;
    
    vector_normalize(&a);
    vector_normalize(&m);
    //see appendix A in app note AN3192
    pitch = asin(-a.x);
    roll = asin(a.y/cos(pitch));
    float heading = 0;
    float xh = m.x * cos(pitch) + m.z * sin(pitch);
    float yh = m.x * sin(roll) * sin(pitch) + m.y * cos(roll) - m.z * sin(roll) * cos(pitch);
 
    heading = 180 * atan2(yh, xh)/PI;
    if (heading < 0) heading += 360;
    
    return heading;
}

void LSM303::vector_cross( const Plane *a,const Plane *b, Plane *out )
{
    out->x = a->y*b->z - a->z*b->y;
    out->y = a->z*b->x - a->x*b->z;
    out->z = a->x*b->y - a->y*b->x;
}

float LSM303::vector_dot( const Plane *a,const Plane *b )
{
    return a->x*b->x+a->y*b->y+a->z*b->z;
}

void LSM303::vector_normalize( Plane *a )
{
    float mag = sqrt(vector_dot(a,a));
    a->x /= mag;
    a->y /= mag;
    a->z /= mag;
}

void LSM303::getLSM303_accel()
{   
    char data[1] = { OUT_X_L_A | (1<<7)};
    char out[6] = {0,0,0,0,0,0};
    i2c.write( LSM303_ACC, data,1);
    i2c.read( LSM303_ACC, out, 6);

    a.x = short( (((short)out[1]) << 8) | out[0] );
    a.y = short( (((short)out[3]) << 8) | out[2] );
    a.z = short( (((short)out[5]) << 8) | out[4] );    
}

void LSM303::getLSM303_mag()
{
    char data[1] = { OUT_X_H_M };
    char out[6];

    i2c.write( LSM303_MAG, data, 1 );
    i2c.read( LSM303_MAG, out, 6 );
    // DLM, DLHC: register address for Z comes before Y
    m.x = short( out[0] << 8 | out[1] );
    m.y = short( out[4] << 8 | out[5] );
    m.z= short( out[2] << 8 | out[3] );
}

int LSM303::LSM303_read(int address)
{
    if (address >= 0x20) {
        _i2c_address = LSM303_ACC;
    } else {
        _i2c_address = LSM303_MAG;
    }

    char value[1];

    char data[1] = { address };
    i2c.write( _i2c_address, data, 1 );
    i2c.read( _i2c_address, value, 1 );
    return value[0];
}

int LSM303::LSM303_write(int data, int address)
{
    if (address >= 0x20) {
        _i2c_address = LSM303_ACC;
    } else {
        _i2c_address = LSM303_MAG;
    }

    char out[2] = { address, data };
    i2c.write( _i2c_address, out, 2 );
    return 0;
}
