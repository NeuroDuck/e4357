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

const int numRadiusBands  = 8;
const int numThetaSectors = 8;

int prevTheta = 0;	

void processCmd( char* cmd)
{
	char *rStr, *thetaStr;
	char delims[] = " \t";
	
	char* firstDelim = strpbrk( cmd, delims);
	
	if (firstDelim == NULL)
		return;																// Malformed, so bail.

	rStr     = strtok( cmd,  delims);
	thetaStr = strtok( NULL, delims);
	
//	pc.printf( "rStr = '%s', thetaStr = '%s'\r", rStr, thetaStr);
	
	// We seem to be receiving doubled characters for theta, so for now
	// just stomp the 2nd one and declare victory.
	//
	thetaStr[1] = '\0';
	int thetaVal = thetaStr[0] - '0';
	
	if (thetaVal < 0 || thetaVal > numThetaSectors - 1)
		return;																// Malformed, so bail.		
		
	int rVal    = atoi( rStr);
	int absRval = abs(  rVal);
	
	if ((absRval < 0   || absRval > numRadiusBands - 1) &&
			(absRval < 100 || absRval > 200))
		return;																// Malformed, so bail.
	
	int forwardSpeed = 0;										// Not moving yet.
	int rotateDir    = 0;										// No rotation yet.

	// absRval >= 100 means full-scale JoyStick deflection, which we
	// take to mean Rotation rather than movement.
	//
	if (absRval >= 100)
	{
//		pc.printf( "Processing full-scale theta = '%d'\r", thetaVal);
		
		// When we're told that theta changed from 2 to 7, it's ambiguous,
		// because we don't know if it changed in the cw or ccw direction.
		// To resolve the ambiguity, we'll calculate both differences.
		//
		// The positive one will be in the ccw direction.
		//
		// For the negative one, we'll add numThetaSectors.
		//
		int negDiff, posDiff;
		int localPrevTheta = prevTheta;
		
		int diff = thetaVal - prevTheta;
		
//		pc.printf( "diff = %d\r", diff);
		
		if (diff < 0)
		{
			negDiff =  diff + numThetaSectors;
			posDiff = -diff;
		}
		else if (diff > 0)
		{
			negDiff = numThetaSectors - diff;
			posDiff = diff;
		}
		else
			return;					// Nothing to see here, move along, move along.

//		pc.printf( "negDiff = %d, posDiff = %d\r", negDiff, posDiff);
		
		// Finally, rotateDir = the sign of the smaller of the absValue of 
		// cwVal and ccwVal.
		
		int signDiff = signI( diff);
		rotateDir = (negDiff < posDiff) ? -signDiff : signDiff;
		
		prevTheta = thetaVal;
	}
	else
		forwardSpeed = absRval;

	if (absRval >= 100)
	{
		pc.printf( "rotateDir = %d\r", rotateDir);
		
		const float rotationRate = 0.1;
		
		if (rotateDir > 0)
			mpi.left( rotationRate);
		else
			mpi.right( rotationRate);
	}
	else
	{
		pc.printf( "forwardSpeed = %d\r", forwardSpeed);
		
		if (thetaVal >= numThetaSectors / 2)
			forwardSpeed *= -1;
		
		if (forwardSpeed != 0)
			mpi.forward( forwardSpeed / 10.0);
		else
			mpi.stop();
	}
}

#define BUFLEN 80

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

		mpi.locate( 0,1);
		char msg[] = "JOYSTICK";
    mpi.print( msg, strlen( msg));
	
		char buf[BUFLEN] = "";
		int bufLen = 0;
	
		pc.baud( 57600);
		xBee.baud( 57600);
		
		int count = 0;
	
		while (1)
		{
        if (!xBee.readable())
					continue;

				char c = xBee.getc();
				pc.putc( c);
				
				if (bufLen >= BUFLEN || c == '\r' || c == '\n')
				{
					processCmd( buf);
					wait_ms( 250);
					
					bufLen = 0;
					pc.printf( "\r");
				}
				else						
					buf[bufLen++] = c;
				
				count++;
				
				if (count % 100 == 0)
				{
					mpi.locate( 0,0);
					sprintf( msg, "B=%4dmV", int( mpi.battery() * 1000));
					mpi.print( msg, strlen( msg));
				}
		}
		
		return 1;
}
