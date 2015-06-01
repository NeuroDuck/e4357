//
// Keil's 32kByte-limit means that I can't compile in all at the same
// time the Libraries/Functions for:
//    m3pi
//    RPCFunction/ReadRange
//    RPCVariable
//
// So I comment out the ones I'm not debugging into, and leave
// uncommented-out just the one that I am debugging.
//
// For reading Line-sensing Sensors individually:
// Send: 
//   0x86 raw sensors 0 10 Reads all five IR sensors and sends the raw values as a
//                         sequence of two-byte ints, in the range 0-2000.
//
#include "mbed.h"
#include "m3pi.h"
#include "LSM303.h"
#include "Ping.h"
#include "MaxSonar.h"

m3pi   mpi;
Serial pc(   USBTX, USBRX); // tx, rx
Serial xBee( p28,   p27);
LSM303 compass;
Ping pong( p8);
MaxSonar* maxSonar;

int signI( int i)
{
  if (i < 0)
    return -1;

  if (i > 0)
    return 1;

  return 0;      
}

int maxI( int i, int j)
{
    if (i > j)
        return i;

  return j;
}

int minI( int i, int j)
{
    if (i > j)
        return j;

  return i;
}

const int numRadiusBands  = 9;
const int numThetaSectors = 16;

const int numSpeedRanges = numThetaSectors / 4;
int wheelRatios[numSpeedRanges];

const int wheelRatioStep = 50 / numSpeedRanges;

const int xBeeWaitMs = 20;      //  19200 baud to JoyStick's Display.
const int mpiWaitMs   = 2;      // 115200 baud to the 3pi base.

// wheelRatios[0] is a placeholder for Rotation.
//
void populateWheelRanges()
{
    int i;
    for (i = 1 ; i < numSpeedRanges ; i++)
    {
        wheelRatios[i] = wheelRatioStep * i;
    }
    wheelRatios[i] = 50;    // Ensure that we can drive straight :-).
}

void displayTurnVars1(
      int r, int theta,
        int minusOneMovingBackwardPositiveOneForward,
        int turningLeftNotRight)
{
    xBee.printf( 
        "r%d T%d BF%d LR%d", 
        r, theta,
        minusOneMovingBackwardPositiveOneForward,
        turningLeftNotRight);   
    
        wait_ms( xBeeWaitMs);               // Give the JoyStick time to display this message.
}

void displayTurnVars2(
      int r, int theta,
        int minusOneMovingBackwardPositiveOneForward,
        int turningLeftNotRight, int rotationSpeed)
{
    xBee.printf( 
        "r%d T%d BF%d LR%d R%d",
        r, theta,
        minusOneMovingBackwardPositiveOneForward,
        turningLeftNotRight,
        rotationSpeed);
    
        wait_ms( xBeeWaitMs);               // Give the JoyStick time to display this message.
}

//                    forward
//                       4
// slight left         5   3   slight right
// medium left       6       2    medium right
// hard leftFwd    7           1     hard rightFwd
// rotate left   8               0      rotate right
// hard leftBkwd   9          15     hard rightBkwd
//                  10      14
//                    11  13
//                      12
//                   backward
//
//
const int halfNumThetaSectors = numThetaSectors / 2;

const int leftMostForwardTheta   = halfNumThetaSectors - 1;
const int leftMostBackwardTheta  = halfNumThetaSectors + 1;
const int rightMostBackwardTheta = numThetaSectors - 1;
// const int rightMostForwardTheta  = 1;

const int forwardTheta  = halfNumThetaSectors / 2;
const int backwardTheta = forwardTheta + halfNumThetaSectors;
const int rotateLeftTheta = halfNumThetaSectors;
const int rotateRightTheta = 0;

int desiredHeading = -1;

inline int getCurrentHeading()
{
    return (int( compass.getTiltHeading() + 90) % 360) / 2;
}

void turn( int r, int theta)
{
    if (r == 0)
    {
        xBee.printf( "All stop.");
        wait_ms( xBeeWaitMs);       // Give the JoyStick time to print this message.

        mpi.stop();                 // the Stop command.
        wait_ms( mpiWaitMs);        // Give the Robot time to stop. 
        
        desiredHeading = -1;        // We're no longer following a particular heading.

        return;
    }
    
    // -1 == backward, and 1 == forward, as this simplifies creating the 
    // m3pi::motor() speed argument.
    //
    // Similarly, I changed:
    //   private: m3pi::motor( int motor, float speed);
    // to be public, so I can pass a calculated value for the motor#,
    // to simplify the code for turning left vs. right.
    //
    int minusOneMovingBackwardPositiveOneForward = 
        (theta >= leftMostBackwardTheta && 
         theta <= rightMostBackwardTheta) ? -1 : 1;
    
    // Are we rotating rather than moving?
    if (theta == rotateLeftTheta || theta == rotateRightTheta)
        minusOneMovingBackwardPositiveOneForward = 0;

    int turningLeftNotRight = 
        (theta > forwardTheta && theta < backwardTheta) ? 1 : 0;
    
    // Forward = wheel speeds are 50:50, so all 4 slower wheels'
    // ratios are: 50, 37, 25, 12, and the other wheel = (100 - that).
        
    // Mirror us up from Quadrant III & IV into II & I, for easier 
    // indexing into wheelRatios[].
    //
    if (minusOneMovingBackwardPositiveOneForward < 0)
        theta = leftMostForwardTheta - (theta - leftMostBackwardTheta);
    
    // Mirror us right from Quadrant II into Quadrant I, for easier 
    // indexing into wheelRatios[].
    //
    if (turningLeftNotRight)
        theta = halfNumThetaSectors - theta;
    
    displayTurnVars1(
        r, theta,
        minusOneMovingBackwardPositiveOneForward,
        turningLeftNotRight);

    // Are we rotating?
    //  
    if (minusOneMovingBackwardPositiveOneForward == 0)
    {
        float rotationSpeed = 1.0 * r / numRadiusBands / 8; // (rotate slower).
        
        displayTurnVars2(
            r, theta,
            minusOneMovingBackwardPositiveOneForward,
            turningLeftNotRight, int( abs( rotationSpeed) * 10));
        
        mpi.motor( 1 - turningLeftNotRight,  rotationSpeed);
        wait_ms( mpiWaitMs);

        mpi.motor(     turningLeftNotRight, -rotationSpeed);
        wait_ms( mpiWaitMs);
        
        return;
    }
    else if (theta != forwardTheta)                 // == We're turning.
    {
         theta = 3;         // Limit us to the most gradual turn of the 3 for now.
    }
    else                    // We're going straight, so follow our compass.
    {
        if (desiredHeading < 0)
        {
            desiredHeading = getCurrentHeading();
        }
        int desiredHeadingChange = desiredHeading - getCurrentHeading();
        
//        if (desiredHeadingChange != 0)
 //           shallowTurnWithCompass( desiredHeadingChange);
    }
    
    float fasterWheelRatio =        wheelRatios[theta]  * r / 500.0;
    float slowerWheelRatio = (100 - wheelRatios[theta]) * r / 500.0;
    
    xBee.printf( 
        "fWR=%3.1f sWR=%3.1f",
      fasterWheelRatio, slowerWheelRatio);
    
    wait_ms( xBeeWaitMs);               // Give the JoyStick time to display this message.
    
    // Need to handle Rotation (= theta == 0), using turningLeftNotRight.
    //
    // Need to damp down the speed (r), from the commanded speed, down 
    // to 0 for rotation.
    //
  mpi.motor( 
        turningLeftNotRight, 
        fasterWheelRatio * minusOneMovingBackwardPositiveOneForward);   
    wait_ms( mpiWaitMs);

  mpi.motor( 
        1 - turningLeftNotRight, 
        slowerWheelRatio * minusOneMovingBackwardPositiveOneForward);
    wait_ms( mpiWaitMs);
    
    return;
}

#define BUFLEN 80
int prevTheta = 0;

// Typical input is:
//   "!1d1 1d2 1d2";
// to parse:
//   "!6 11 17"
//
// Returns:
//   1 == all's well.
//   0 thru -numFields == that field didn't parse.
// -10 -actualLength   == actualLen
//
// Return (-10 - strlen( cmd)) if the input length is not 
// as expected.
//
// Usage:
//  char* fieldsSpec = "!1d1 1d2 1d2";
//  int* results[] = { 
//    dummy, &rVal, &dummy, &thetaVal, &dummy, &checksum 
//  };
//  int invalidFieldNum = checkFields( cmd, fieldsSpec, results);
// 
//  Parsed values are returned in variables pointed to by the 
//  array elements in results[].
//
int checkFields( char* cmd, char* fieldsSpec, int* results[])
{
    int numFields = strlen( fieldsSpec) / 2;
    
    int fieldNdx;
    int expectedInputLength = 0;
    for (fieldNdx = 0 ; fieldNdx < numFields ; fieldNdx++)
    {
        expectedInputLength += fieldsSpec[fieldNdx * 2 + 1] - '0';
    }
    
    int actualInputLength = strlen( cmd);
    if (actualInputLength != expectedInputLength)
        return -10 - actualInputLength;
    
    int cmdNdx = 0;
    for (fieldNdx = 0 ; fieldNdx < numFields ; fieldNdx++)
    {
        int numRepeats = fieldsSpec[fieldNdx * 2 + 1] - '0';
        bool failed = false;
        *(results[fieldNdx]) = 0;  // Clear out any passed in junk.
        
        for (int repeatNdx = 0 ; repeatNdx < numRepeats ; repeatNdx++)
        {
            switch (fieldsSpec[fieldNdx * 2])
            {
                case 'd':
                    if (cmd[cmdNdx] < '0' || cmd[cmdNdx] > '9')
                        failed = true;
                    else
                    {
                        *(results[fieldNdx]) *= 10;
                        *(results[fieldNdx]) += cmd[cmdNdx] - '0';
                    }
                    break;

                default:
                    if (cmd[cmdNdx] != fieldsSpec[fieldNdx * 2])
                        failed = true;
            }
            cmdNdx++;
            
            if (failed)
                return -fieldNdx;
        }
    }
    
    return 1;       // All's well.
}

// Input is now fixed-length 8-chars,
// with the last 2 digits being 
// the sum of the first two numbers,
// starting with a "!" :-). 
// At present radiusRange is always a single digit.
// I.e., ...
// 12345678
// !6 11 17
//
bool processDrivingCmd( char* cmd, char* errorMsg)
{
    int rVal = 0, thetaVal = 0, checksum = 0, dummy = 0;
    char fieldsSpec[] = "!1d1 1d2 1d2";
    int* results[] = { 
        &dummy, &rVal, &dummy, &thetaVal, &dummy, &checksum 
    };
    const char* fieldNames[] = { 
        "Start", "Radius", "Space1", "Theta", "Space2", "Chksum"
    };
    
    int invalidFieldNum = checkFields( cmd, fieldsSpec, results);
    
    if (invalidFieldNum <= -10)     // The length was incorrect.
    {
        sprintf( errorMsg, "%d-Length", 10 + invalidFieldNum);
        return false;
    }
    
    if (rVal + thetaVal != checksum)
    {
        sprintf( errorMsg, "%02d!%02dchk", rVal + thetaVal, checksum);
        return false;
    }

    if (invalidFieldNum != 1)
    {
        pc.printf( "%s\r\n", fieldNames[-invalidFieldNum]);
        return false;
    }
    
    if (rVal < 0 || rVal > numRadiusBands)
    {
        sprintf( errorMsg, "%d-Radius", rVal);
        return false;
    }
    
    if (thetaVal < 0 || thetaVal > numThetaSectors - 1)
    {
        sprintf( errorMsg, "%d-Theta", thetaVal);   
        return false;
    }
    
//  pc.printf( "rVal = '%d', thetaVal = '%d'\r", rVal, thetaVal);

    // Call our fancy new turning function.
    //  
    turn( rVal, thetaVal);
    
    return true;
}

char batteryDisplayMsg[10] = "B=0000mV";
bool deadBatteryNotNotified = true;
bool robotBatteryIsDead = false;
int robotBatteryLevel = 0;
    
void displayBatteryValue()
{
    mpi.locate( 0,0);
    
    mpi.print( batteryDisplayMsg, strlen( batteryDisplayMsg));
    wait_ms( mpiWaitMs);                // Give the Robot time to display this message.
}

void updateBatteryValue()
{
    robotBatteryLevel = mpi.battery() * 1000;

    int rbl = robotBatteryLevel;

    for (int i = 5 ; i >= 2 ; i--)
    {
        batteryDisplayMsg[i] = '0' + rbl % 10;
        rbl /= 10;
    }   
}
// Ticker tickr;

int main() 
{
    // I'm happy to report that typing in /m3pi/battery in my Serial 
    // Terminal program did print out the battery value, and typing 
    // in /m3pi/left did make the Robot spin left.
    //
    // Meaning that the m3pi Library's approach to register-function-
    // for-RPC is working.
    //
    // Since the SerialRPCInterface Library's RPCFunction.cpp's 
    // registration approach isn't working, I'll modify it to match
    // the mp3i Library's approach, and then hopefully we'll have 
    // a RPCFunction()-registration Class that works.
    //  
    char mpiMsg[] = "JOYSTICK", c;

    mpi.locate( 0, 1);
    mpi.print( mpiMsg, strlen( mpiMsg));    
    mpi.leds( 0x0);     // Let's save our battery a bit for now.

    pc.baud(   57600);
    xBee.baud( 57600);
    
    populateWheelRanges();
    
    int joyStickBufLen = 0, pcBufLen = 0;
    char joyStickBuf[BUFLEN] = "";
    char processResult[BUFLEN];
    char pcBuf[BUFLEN] = "";

    updateBatteryValue();   // Call them once at the beginning then via Ticker.
    displayBatteryValue();
    
    compass.setup();
    float heading = 360;
    long frontRange;         // For Ping front-facing Sonar Sensor.
    int sideRange;
    
//  tickr.attach( &updateBatteryValue, 10.0);       // Update the value every 10 sec.

    PinName maxSonarTriggerOutPin = p15;
    PinName maxSonarAnalogInPin   = p16;
    maxSonar = new MaxSonar(
        MS_LV, MS_ANALOG, maxSonarTriggerOutPin, maxSonarAnalogInPin);

    maxSonar->setVoltage( 5.0);
    maxSonar->setUnits(   MS_INCH);

    robotBatteryLevel = mpi.battery() * 1000;
    pc.printf( "robotBatteryLevel = %d\r\n", robotBatteryLevel);

    while (1)
    {

        heading = compass.getTiltHeading();
        pc.printf( "heading = %.0f  ", heading);

//        robotBatteryLevel = mpi.battery() * 1000;
//        pc.printf( "frontRange(in.) = %d  robotBatteryLevel = %d\r\n", frontRange, robotBatteryLevel);

        frontRange = pong.readDistanceInCM();
        if (frontRange == -1)
            pc.printf( "frontRange == -1  ");
        else
        {
            frontRange /= 2.54;
            pc.printf( "frontRange(in.) = %d'%d\"  ", frontRange / 12, frontRange % 12);
        }
        
        sideRange = maxSonar->readDistance();
        printf( "sideRange(in.) = %d'%d\"\r\n", sideRange / 12, sideRange % 12);
        
        // Let's not go crazy with all the printing...
        wait_ms( 200);

        continue;        
        
        if (deadBatteryNotNotified && robotBatteryLevel < 4400)
        {                    // 1234567890123456.
            xBee.printf( "He's dead, Jim.");
            wait_ms( 1500);             // Give the user time to read this message.
            xBee.printf( "=Robot bat. dead");
            wait_ms( xBeeWaitMs);   // Give the JoyStick time to display this message.

            deadBatteryNotNotified = false;
            robotBatteryIsDead = true;
        }
        
        while (pc.readable())           // Give us a way to type in a message to send to
        {                                                   // the JoyStick.
            c = pc.getc();
            pcBuf[pcBufLen++] = c;  
            wait_ms( 1);                        // Is another char coming?
        }

        if (pcBufLen > 0)
        {
            // Stomp any received EOL char, so we don't send it to the JoyStick.
            if (pcBuf[pcBufLen - 1] == '\r' || pcBuf[pcBufLen - 1] == '\n')
                pcBuf[pcBufLen - 1] = '\0';
            else
                pcBuf[pcBufLen] = '\0';

            pcBufLen = 0;
            xBee.printf( "%s", pcBuf);
        }

        if (!xBee.readable())
            continue;

        c = xBee.getc();
        pc.putc( c);
        
        if (joyStickBufLen < BUFLEN && c != '\r' && c != '\n')
        {
            joyStickBuf[joyStickBufLen++] = c;
            continue;
        }                                   // The JoyStick sends us an EOL of {13,10}.

        if (joyStickBufLen == 0)            // We received an EOL of "\n\r", or a blank line,
            continue;                                       // so disgard it and start over.
        else
            joyStickBuf[joyStickBufLen] = '\0';     // Null-terminate our received cmd.
        
        pc.printf( "joyStickBuf = >%s<\r\n", joyStickBuf);
        
        processResult[0] = '\0';    // Give the Robot time to prepare to receive
        wait_ms( 10);                           // our response.
        bool processingSucceeded = processDrivingCmd( joyStickBuf, processResult); 

        if (!processingSucceeded)
            sprintf( mpiMsg, "%-8s", processResult);
        else
        {
            int originaljoyStickBufLen = strlen( joyStickBuf);

            // joyStickBuf = "!1 02 03".
            joyStickBuf[5] = '\0';          // We only want to see r & theta on Robot.
            sprintf( mpiMsg, "%-4s L=%d", &(joyStickBuf[1]), originaljoyStickBufLen);
        }

        pc.printf( "%s\r\n", mpiMsg);
        
        mpi.locate( 0, 0);              
        mpi.print( mpiMsg, strlen( mpiMsg));
        wait_ms( mpiWaitMs);                    // Give the Robot time to display this message.

        displayBatteryValue();

        joyStickBufLen = 0;
        pc.printf( "\r");
        
//      wait_ms( 2000);                             // Just for now for testing.
//      mpi.stop();
    }
    
    return 1;
}
