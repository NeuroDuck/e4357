#ifndef LSM303_h
#define LSM303_h
#include "mbed.h"

/* LSM303 Address definitions */
#define LSM303_MAG  0x3C  // assuming SA0 grounded
#define LSM303_ACC  0x30  // assuming SA0 grounded


/* LSM303 Register definitions */
#define CTRL_REG1_A 0x20
#define CTRL_REG2_A 0x21
#define CTRL_REG3_A 0x22
#define CTRL_REG4_A 0x23
#define CTRL_REG5_A 0x24
#define HP_FILTER_RESET_A 0x25
#define REFERENCE_A 0x26
#define STATUS_REG_A 0x27
#define OUT_X_L_A 0x28
#define OUT_X_H_A 0x29
#define OUT_Y_L_A 0x2A
#define OUT_Y_H_A 0x2B
#define OUT_Z_L_A 0x2C
#define OUT_Z_H_A 0x2D
#define INT1_CFG_A 0x30
#define INT1_SOURCE_A 0x31
#define INT1_THS_A 0x32
#define INT1_DURATION_A 0x33
#define CRA_REG_M 0x00
#define CRB_REG_M 0x01//refer to the Table 58 of the datasheet of LSM303DLM
#define MAG_SCALE_1_3 0x20//full-scale is +/-1.3Gauss
#define MAG_SCALE_1_9 0x40//+/-1.9Gauss
#define MAG_SCALE_2_5 0x60//+/-2.5Gauss
#define MAG_SCALE_4_0 0x80//+/-4.0Gauss
#define MAG_SCALE_4_7 0xa0//+/-4.7Gauss
#define MAG_SCALE_5_6 0xc0//+/-5.6Gauss
#define MAG_SCALE_8_1 0xe0//+/-8.1Gauss
#define MR_REG_M 0x02
#define OUT_X_H_M 0x03
#define OUT_X_L_M 0x04
#define OUT_Y_H_M 0x07
#define OUT_Y_L_M 0x08
#define OUT_Z_H_M 0x05
#define OUT_Z_L_M 0x06
#define SR_REG_M 0x09
#define IRA_REG_M 0x0A
#define IRB_REG_M 0x0B
#define IRC_REG_M 0x0C
#define LSM303_WHO_AM_I_M        0x0F // DLM only

#define PI                    3.14159265

// Uncomment when calibrating.
#define CALIBRATING 

/** LSM303DLM  mbed code based on AN3192 Application note and LSM303DLH example code by Jim Lindblom SparkFun Electronics 
  modified by Frankie.Chu to arduino in year 2012.

   date: 13/10/13
   license: Use this with your own risk:-)

   Calibration the compass and accelometer is a must to make your compass to work:
@code
   //calibration loop
   for(int i = 0; i <200; i++) {

        lsm.getLSM303_mag();

// Mmin handler
        if(lsm.m.x  <  lsm.m_min.x)
            lsm.m_min.x  =  lsm.m.x;

        if(lsm.m.y  <  lsm.m_min.y)
            lsm.m_min.y  =  lsm.m.y;

        if(lsm.m.z  <  lsm.m_min.z)
            lsm.m_min.z  =  lsm.m.z;

// Mmax handler
        if(lsm.m.x  >  lsm.m_max.x)
            lsm.m_max.x  =  lsm.m.x ;

        if(lsm.m.y  >  lsm.m_max.y)
            lsm.m_max.y =  lsm.m.y;

        if(lsm.m.z  >  lsm.m_max.z)
            lsm.m_max.z  =  lsm.m.z;
        wait(0.1);
    }
@endcode
*/
class LSM303
{
public:
    //! A plane with x,y and z axis
    typedef struct Plane {
        float x, y, z;
    } Plane;
    
    //! accelerometer readings
    Plane a; 
    Plane a_max;
    Plane a_min;
    //! magnetometer readings
    Plane m; 
    Plane m_max; // maximum magnetometer values, used for calibration
    Plane m_min; // minimum magnetometer values, used for calibration
    Plane scale; //soft magneting field scaling
    //! Initialises LSM303DLM chip
    void setup();
    //!Tests Accelometer. Returns 0xFF on error, 0x30 if succesful.
    int testAcc();
    //!Tests Compass. Returns 0xFF on error, 0x0F if succesful and LSM303DLM; 0x3C if LSM303DH.
    int testMag();   
    //! Returns compass heading in degrees   
    float getTiltHeading();
    //! Reads magnetometer values to m
    void getLSM303_mag();
    //! Reads accelerometer values to a
    void getLSM303_accel();
    
private:
    int _i2c_address;
    float pitch;
    float roll;
 
    int LSM303_read(int address);
    int LSM303_write(int data, int address);    
    // Plane functions
    static void vector_cross(const Plane *a, const Plane *b, Plane *out);
    static float vector_dot(const Plane *a,const Plane *b);
    static void vector_normalize(Plane *a);
    
    void getScale(Plane *scale);
};

#endif

