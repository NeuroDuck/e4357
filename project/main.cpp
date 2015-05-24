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
#include "mbed.h"
#include "m3pi/m3pi.h"

m3pi   mpi;
Serial pc(   USBTX, USBRX); // tx, rx
Serial xBee( p28,   p27);

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

// wheelRatios[0] is a placeholder for Rotation.
//
void populateWheelRanges()
{
	int i;
	for (i = 1 ; i < numSpeedRanges ; i++)
	{
		wheelRatios[i] = wheelRatioStep * i;
	}
	wheelRatios[i] = 50;	// Ensure that we can drive straight :-).
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

void turn( int r, int theta)
{
	if (r == 0)
	{
		mpi.stop();
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
	
	// Are we rotating?
	//	
	if (minusOneMovingBackwardPositiveOneForward == 0)
	{
		float rotationSpeed = (float)r / numRadiusBands; // / 4; // (run slower).
		
		mpi.motor( 1 - turningLeftNotRight,  rotationSpeed);
		wait_ms( 50);
		mpi.motor(     turningLeftNotRight, -rotationSpeed);
		
		return;
	}

	float fasterWheelRatio =        wheelRatios[theta]  * r / 500.0 / 4;
	float slowerWheelRatio = (100 - wheelRatios[theta]) * r / 500.0 / 4;
	
	// Need to handle Rotation (= theta == 0), using turningLeftNotRight.
	//
	// Need to damp down the speed (r), from the commanded speed, down 
	// to 0 for rotation.
	//
  mpi.motor( 
		1 - turningLeftNotRight, 
		fasterWheelRatio * minusOneMovingBackwardPositiveOneForward);
	
	wait_ms( 50);

  mpi.motor( 
		turningLeftNotRight, 
		slowerWheelRatio * minusOneMovingBackwardPositiveOneForward);
	
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
// 	char* fieldsSpec = "!1d1 1d2 1d2";
// 	int* results[] = { 
//	  dummy, &rVal, &dummy, &thetaVal, &dummy, &checksum 
//	};
//	int invalidFieldNum = checkFields( cmd, fieldsSpec, results);
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
	
	return 1;		// All's well.
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
	
	if (invalidFieldNum <= -10)		// The length was incorrect.
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
	
	pc.printf( "rVal = '%d', thetaVal = '%d'\r", rVal, thetaVal);

	// Call our fancy new turning function.
	//	
	turn( rVal, thetaVal);
	
	return true;
}

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
	mpi.leds( 0x0);		// Let's save our battery a bit for now.

	mpi.locate( 0, 1);
	char msg[] = "JOYSTICK";
	mpi.print( msg, strlen( msg));

	pc.baud(   57600);
	xBee.baud( 57600);
	
	populateWheelRanges();
	
	int bufLen = 0;
  char buf[BUFLEN] = "";
	char pbuf[BUFLEN], processResult[BUFLEN];
	int batteryDisplayCount = 0;

	while (1)
	{
			if (!xBee.readable())
				continue;

			char c = xBee.getc();
			pc.putc( c);
			
			if (bufLen >= BUFLEN || c == '\r' || c == '\n')
			{
				buf[bufLen] = '\0';		// Null-terminate our received cmd.
				
				pc.printf( "%s\r\n", buf);
				
				processResult[0] = '\0';
				bool processingSucceeded = processDrivingCmd( buf, processResult); 

				if (batteryDisplayCount % 100 > 20)
				{
					mpi.locate( 0, 0);
					
					if (!processingSucceeded)
						sprintf( msg, "%-8s", processResult);
					else
					{
						sprintf( pbuf, "%-4s L=%d", buf, strlen( buf));
						sprintf( msg, "%-8s", pbuf);
					}

					pc.printf( "%s\r\n", msg);
					mpi.print( msg, strlen( msg));
				}
				wait_ms( 10);
				
				bufLen = 0;
				pc.printf( "\r");
			}
			else						
				buf[bufLen++] = c;
			
			batteryDisplayCount++;
			
			if (batteryDisplayCount % 100 == 0)
			{
				mpi.locate( 0,0);
				sprintf( msg, "B=%4dmV", int( mpi.battery() * 1000));
				mpi.print( msg, strlen( msg));
			}
	}
	
	return 1;
}
