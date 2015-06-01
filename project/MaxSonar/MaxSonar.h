/**
 * @file MaxSonar.h
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
 * This class provides an object to obtain range measurements from a
 * Maxbotix MaxSonar ultrasonic range finder.
 *
 * Supported devices:
 * 1) LV-MaxSonar: -EZ0 MB1000, -EZ1 MB1010, -EZ2 MB1020,
 *                 -EZ3 MB1030, -EZ4 MB1040
 *
 * Supported modes:
 * 1) Analog.
 */
#ifndef MAXSONAR_H_
#define MAXSONAR_H_

#include "mbed.h"

//! Define types of MaxSonar devices.
enum MSType {
    MS_LV = 0,      //!< Supported.
    MS_WR,          //!< Not supported yet.
    MS_WRC,         //!< Not supported yet.
    MS_XL,          //!< Not supported yet.
    MS_XL_AE        //!< Not supported yet.
};

//! Define units for returning range.
enum MSUnits {
    MS_CM = 0,      //!< centimeters.
    MS_INCH         //!< inches.
};

//! Define access mode for obtaining range measurement.
enum MSMode {
    MS_ANALOG = 0,  //!< Supported.
    MS_SERIAL,      //!< Not supported yet.
    MS_PWM          //!< Not supported yet.
};

/**
 * Class to read range measurements from MaxBotix MaxSonar
 * range-finder devices.
 *
 * Example
 * @code
 * #include "mbed.h"
 * #include "MaxSonar.h"
 * 
 * int main() {
 *     MaxSonar *range;
 *     float r;
 *
 *     // Create and configure object for 3.3V powered LV-series device, 
 *     // accessed with analog reads (in cm) on p16, triggered by p7.
 *     range = new MaxSonar(MS_LV, MS_ANALOG, p7, p16);
 *     range->setVoltage(3.3);
 *     range->setUnits(MS_CM);
 *
 *     while(1) {
 *         // Trigger read, wait 49ms until ranger finder has
 *         // finished, then read. 
 *         range->triggerRead();
 *         wait_ms(49);
 *         r = range->read();
 *       
 *         // Print and delay 0.5s.
 *         printf("Range: %.3f cm\n", r);
 *         wait(0.5);
 *     }
 * }
 * @endcode 
 */
class MaxSonar 
{
private:
    enum MSType type;           //!< Device type. 
    enum MSMode mode;           //!< Range reading mode.
    enum MSUnits units;         //!< Range units.
    float voltage;              //!< Supply/reference voltage (V).
    int analog_scale;           //!< resolution = voltage/scale.
    float analog_resolution;    //!< V/inch to compute range.
    AnalogIn *ain;              //!< For analog reads.
    DigitalOut *rx_req;         //!< For triggered reads (PWM or AIN).
    
    /**
     * Read range value.
     *
     * @return  Range value in the set units (default cm).
     */
    float read( void);

public:
    /**
     * Constructor.
     *
     * @param   type    The type of device.
     * @param   mode    The access mode.
     * @param   pin1    MS_ANALOG: Pin connected to RX of device.
     *                  MS_SERIAL: Pin connected to RX of device.
     *                  MS_PWM: Pin connected to RX of device.
     * @param   pin2    MS_ANALOG: Pin connected to AN of device.
     *                  MS_SERIAL: Pin connected to TX of device.
     *                  MS_PWM: Pin connected to PW of device.
     * @note    pin1 may be NC if only continuous reading is desired
     *          when in analog or pwm mode.
     * @note    Default units are in cm (MS_CM).
     */
    MaxSonar(enum MSType type, enum MSMode mode,
             PinName pin1, PinName pin2);

    /**
     * Destructor.
     */
    ~MaxSonar( void);

    /**
     * Set the units for reading range value. Default cm (MS_CM).
     *
     * @param   units   The specified units option.
     */
    void setUnits( enum MSUnits units);

    /**
     * Specify the supply voltage used by the device. 
     * 
     * @param   voltage The specified voltage (default 3.3)
     *
     * @note    This is important for correct conversion of the voltage 
     *          from pin AN of device into the range value.
     */
    void setVoltage( float voltage);

    /**
     * Trigger a reading of range. The reading will be ready 49 ms
     * after the trigger, accessable by the read() function. 
     * 
     * @param   none
     *
     * @note    Triggered reading is only possible if a pin connected to 
     *          RX of the device was specified when calling the constructor. 
     */
    float readDistance( void);
};

#endif
