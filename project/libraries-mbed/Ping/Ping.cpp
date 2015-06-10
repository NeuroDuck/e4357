#include "Ping.h"

#include "mbed.h"

Ping::Ping( PinName PING_PIN)
    : _event( PING_PIN)
    , _cmd( PING_PIN)
    , _timer()
{
    // Hopefully this helps the Ping's response pulse get above 3V.
    //
//    _cmd.mode( PullDown);               

    _event.rise( this,&Ping::_Starts);
    _event.fall( this,&Ping::_Stops);

    _SPEED_OF_SOUND_CM = 34167;
} 
        
void Ping::_Starts( void)
{
    _Valid = false;  // start the timer, and invalidate the current time.
    _Busy = true;
    _timer.start();
    _Time = _timer.read_us();      
}

void Ping::_Stops( void)
{
      _Valid = true;  // When it stops, update the time
      _Busy = false;
      _Time = _timer.read_us()- _Time;
}

// see the ping documentation 
// http://www.parallax.com/Portals/0/Downloads/docs/prod/acc/28015-PING-v1.6.pdf
//
long Ping::readDistanceInCM()
{
    // Set our pin to output to write out our pulse, as per:
    //
    // http://www.perboliallc.com/pads-distance-sensor-1.html
    // http://nebula.wsimg.com/8612c57a5c3f911ae07cf8038fd170ab?AccessKeyId=187C1F12DFE10BF9D991&disposition=0&alloworigin=1
    //
    _cmd.output();
    
    // Make sure we're starting out LOW.
    //    
    _cmd.write( 0);
    const int leadinDuration = 10;
    wait_us( leadinDuration);

    // Bring our pin HIGH for 5 uS., as per the above.
    //
    _cmd.write( 1);
    const int triggerPulseDuration = 5;
    wait_us( triggerPulseDuration);            // Originally was 3 uS.
    _cmd.write( 0);
    
    // Prepare to receive/record the Pin's pulse response.
    //
    const int waitToSwitchToInputDuration = 5;
    wait_us( waitToSwitchToInputDuration);    // Make sure the write( 0) is done(?).
    _cmd.input();
    
    // Apparently the max. value that _Time can take on is 21634 uS.
    // Relatedly, the PING waits somewhere between 0-750 uS. after 
    // receiving the pulse above, before it starts its response pulse.
    //
    // So to simplify things, let's hang out here for, then just return 
    // the measured distance ourselves, so now there's no need for the 
    // user to make a 2nd call to Read_cm().
    //
    const int pingThinkingDuration =   750;
    const int maxDistanceDuration  = 21634;
    const int plusAlittle          =  1000;
    
    const int maxWaitTimeIn_uS = 
        leadinDuration + 
        triggerPulseDuration + 
        waitToSwitchToInputDuration +
        pingThinkingDuration +
        maxDistanceDuration +
        plusAlittle;                  
                                      
    wait_us( maxWaitTimeIn_uS);
        
    return Read_cm();
}

// From the PADS doc above:
// "After an approximately 750 usec delay from your input pulse end the 
//  P.A.D.S will pull the pin high. The time from this pin high to when 
//  it goes low is the travel time for the acoustic pulse to the target."

// -1 means not valid.
//
// In Wikipedia, it says SoS is 34167 cm/Sec.
//
// _Time is in units of S. / 1000000, so:
//
// cm = _Time * 34167 / 1000000;
//
long Ping::Read_cm()
{
    if(_Valid && ~_Busy)                               // Interesting, dividing by 1000000
        return _Time * _SPEED_OF_SOUND_CM / 2000000;   // gives the there-and-back distance.
    else 
        return -1;
}

// In Wikipedia, it says SoS is 34167 cm/Sec.
//
// _Time is in units of S. / 1000000, so:
//
// cm = _Time * 34167 / 1000000;
//
void Ping::Set_Speed_of_Sound( int SoS_ms)
{
    _SPEED_OF_SOUND_CM = SoS_ms;
}
