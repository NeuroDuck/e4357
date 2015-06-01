/**
 * @file MaxSonar.cpp
 * @section LICENSE
 * Copyright (c) 2010 Mustafa Ozgur Kanli.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"),to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section Description
 * Implementation of MaxSonar class.
 */

#include "mbed.h"
#include "MaxSonar.h"

MaxSonar::MaxSonar(
    enum MSType type, enum MSMode mode, 
    PinName triggerOutPin, PinName analogInPin)
{
    // Set some defaults common to all devices.
    //
    this->units = MS_CM;
    this->voltage = 3.3;
    this->ain = NULL;
    this->rx_req = NULL;

    // Save settings.
    this->type = type;
    this->mode = mode;

    // Device specific settings.
    switch (this->type) 
    {
    case MS_LV:
        this->analog_scale = 512;
        break;
    
    // @todo Add functionality.    
    case MS_WR:
    case MS_WRC:
    case MS_XL:
    case MS_XL_AE:
    default:
        error("MaxSonar: Currently unsupported.\n");
        break;
    }

    // Mode specific settings.
    switch (this->mode) 
    {
    case MS_ANALOG:
        if (triggerOutPin != NC)
        {
            rx_req = new DigitalOut( triggerOutPin);
            rx_req->write( 0);       // Force it LOW to stop it ranging.
        }
        ain = new AnalogIn( analogInPin);
        this->analog_resolution = this->voltage / this->analog_scale;
        break;

    // @todo Add functionality.            
    case MS_SERIAL:
    case MS_PWM:
    default:
        error("MaxSonar: Currently unsupported.\n");
        break;
    }
}

MaxSonar::~MaxSonar( void) 
{
    delete ain;
}

void MaxSonar::setUnits( enum MSUnits units) 
{
    this->units = units;
}

void MaxSonar::setVoltage( float voltage) 
{
    this->voltage = voltage;
    this->analog_resolution = this->voltage / this->analog_scale;
}

float MaxSonar::readDistance( void) 
{
    if (rx_req == NULL)
        return -1.0;
        
    // Make sure we're starting out LOW.
    //    
    rx_req->write( 0);
    const int leadinDuration = 10;
    wait_us( leadinDuration);

    // Bring rx line high for 20us to perform a read. The read will
    // be available 49ms after request, as per:
    //   http://maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf
    //
    rx_req->write( 1);
    wait_us( 25);
    rx_req->write( 0);
    
    // Tick, tock.  The Analog interface is slooooww....
    //
    wait_ms( 49);

    return MaxSonar::read();
}

// Min. measurement is 7", which matches the PING.
// When PING = 144", MaxSonar = 205.41"
// (I'm fudging this to make the calc. below come 
//  out they way I want.)
//
const float equalPoint               =   7.0;
const float minMeasuredPING          =  36.0;
const float minCorrespondingMaxSonar =  49.0;
const float maxMeasuredPING          = 144.0;
const float maxCorrespondingMaxSonar = 205.41;
const float scaleFactor =
   (maxMeasuredPING - minMeasuredPING) / 
   (maxCorrespondingMaxSonar - minCorrespondingMaxSonar);

inline float inchesCorrection( float measuredInches)
{
    
    return equalPoint +
        (measuredInches - equalPoint) * scaleFactor;    
}

float MaxSonar::read( void)
{
    float range = 0.0;

    // Make the reading.
    switch (this->mode) 
    {
    case MS_ANALOG:
        //! Range value is computed in inches by default.
        range = (this->ain->read() * this->voltage) / this->analog_resolution;
        break;

    // @todo Add functionality.            
    case MS_SERIAL:
    case MS_PWM:
    default:
        error( "MaxSonar: Currently unsupported.\n");
        break;
    }
    
    // Perform conversion.
    switch (this->units) 
    {
    case MS_CM:
        range *= 2.54;
        break;
    
    case MS_INCH:       // Apparently, the default measured units are inches.
        range = inchesCorrection( range);
        break;
        
    default:
        error( "MaxSonar: Currently unsupported.\n");
        break;
    }

    return range;
}
