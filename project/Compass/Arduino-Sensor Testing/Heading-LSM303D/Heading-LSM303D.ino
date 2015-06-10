#include <Wire.h>
#include <LSM303D.h>
#include <LSM303DLM.h>

LSM303D compassD;

LSM303DLM compass;

// Capture and average heading differences between the LSM303D and LSM303DLM.
//
// Adapted from:
//
//   https://github.com/pololu/lsm303-arduino/tree/master/LSM303
//
// which I renamed "LSM303D", to distinguish from the earlier version of this library,
// which I renamed "LSM303DLM".  The renamed libraries are in:
//
//	 e4357/project/libraries-Arduino/LSM303*

// To cope with running out of global variables memory on the Nano, only capture 
// averages for 90 degrees of the circle at a time.
//
#define NUMDEGREES 90

// Set CURRENTQUADRANTNUM to be between 0 and (360 / NUMDEGREES), usually one of 0 thru 3.
//
#define CURRENTQUADRANTNUM 3

// What angle do we start capturing at?
//
#define STARTINGDEGREES (NUMDEGREES * CURRENTQUADRANTNUM)

// Capture and average 25 samples per degree.
//
#define NUMSAMPLES 25

double diffAvgs[NUMDEGREES];
int    diffCounts[NUMDEGREES];
int numDiffsRemaining = NUMDEGREES;

void setup()
{
  Serial.begin( 57600);

  Wire.begin();

  compassD.init();
  compassD.enableDefault();

  compass.init();
  compass.enableDefault();
  
  // Initialize sample arrays.
  for (int degreeNdx = 0 ; degreeNdx < NUMDEGREES ; degreeNdx++)
  {
    diffAvgs[degreeNdx]   = 0.0;
    diffCounts[degreeNdx] =   0;
  }

  /*
  Calibration values; the default values of +/-32767 for each axis
  lead to an assumed magnetometer bias of 0. Use the Calibrate example
  program to determine appropriate values for your particular unit.
  */
  // Various Calibration Results:
  //
  // On JoyStick via Nano:
  // min: { -8960,  -4023,  -5655}    max: {   +43,  +1573,   -486}
  //
  // On Nano by itself:
  // min: { -8189,  -4233,  -5591}    max: {  -413,  +1115,   -760}
  //
  // Repeated w/Nano, on blanket:
  // min: { -9237,  -4508,  -5217}    max: { -1372,   +524,   -640}
  //
  // Repeated w/Nano, on kitchen table:
  // min: { -9239,  -4501,  -5283}    max: { -1584,   +774,   -531}
  //
  // Repeated in car:
  // min:{ -7749,  -4457,  -3400}  max:{ -3920,  -1183,  -1410}
  //
  // Repeated together:
  // minD:{-11171,  -4518,  -4569}  maxD:{ -1811,   +556,   +986}
  // min: {  -590,   -657,   -525}  max: {  +412,   +513,   +427}
  //
  // Repeated together, after rebuilding with shortest wires:
  // See photo: "Compass\LSM303DLM vs. LSM303D Heading Differences-Shortest Wiring.jpg".
  // minD:{ -3390,  -3139,  -2784}  maxD:{ +3179,  +3122,  +3125}
  // min:{   -617,   -577,   -611}  max:{   +455,   +503,   +353}
  //
  compassD.m_min = (LSM303D::vector<int16_t>){-3390, -3139, -2784};
  compassD.m_max = (LSM303D::vector<int16_t>){+3179, +3122, +3125};

  compass.m_min.x = -617; compass.m_min.y = -577; compass.m_min.z = -611;
  compass.m_max.x = +455; compass.m_max.y = +503; compass.m_max.z = +353;
}

// Display our results.
//
void printAvgDiffs()
{
  for (int degreeNdx = 0 ; degreeNdx < NUMDEGREES ; degreeNdx++)
  {
    Serial.print( degreeNdx + STARTINGDEGREES);
    Serial.print( ": ");
    Serial.println( diffAvgs[degreeNdx], 2);
  }
}

int headingNdx = 0;
double heading, headingD, diff, oldDenominator, newDenominator;
bool done = false;
bool captureAvgs = true;

void loop() 
{
  if (captureAvgs)
  {
    if (done)							// Just sit and spin, so the user can peruse the
      return;							// results.
    else if (numDiffsRemaining == 0)
    {
      printAvgDiffs();					// Display our results, then stop printing.
      done = true;
      return;
    }
  }

  /* When given no arguments, the heading() function returns the angular
   * difference in the horizontal plane between a default vector and
   * north, in degrees.
   * 
   * The default vector is chosen by the library to point along the
   * surface of the PCB, in the direction of the top of the text on the
   * silkscreen. This is the +X axis on the Pololu LSM303D carrier and
   * the -Y axis on the Pololu LSM303DLHC, LSM303DLM, and LSM303DLH
   * carriers.
   * 
   * To use a different vector as a reference, use the version of heading()
   * that takes a vector argument; for example, use
   * 
   *   compass.heading((LSM303::vector<int>){0, 0, 1});
   * 
   * to use the +Z axis as a reference.
   */

  compassD.read();
  headingD = compassD.heading();
  headingNdx = int( headingD + 0.5) - STARTINGDEGREES;

  if (!captureAvgs || diffCounts[headingNdx] != -NUMSAMPLES)
  {
    Serial.print( "D: ");
    Serial.print( headingD, 0);  
    
    compass.read();
    heading = compass.heading();
    Serial.print( "  DLM: ");
    Serial.print( heading, 0);
    delay( 10);
    
    // When on a pillow, the diff between the Compasses' Headings is quite small, 
	// i.e., 2-3, likely solely due to mounting orientation differences.
	//
    // When placed on the plastic stool, some places have a small (3-5) diff, 
    // that can be adjusted away by rotating one of the Compasses.
	//
    // When placed near one of the metal legs, the diff increases to ~60, and 
    // cannot be adjusted away.
	//
	// I found that the LSM303D is about 10x-20x(!) _more_ sensitive to long signal
	// wires than the LSM303DLM is.  
	//
	// Perhaps its sensitivity can be decreased, so it can become usable in places
	// where long wires are required?
	//
	// The tradeoff is, I found that the LSM303D has about 2x less random heading flipping 
	// back-and-forth than the LSM303DLM does.  To wit, the D flipped back and forth only
	// 1 degree, while the DLM flipped 2-3 degrees, both when sitting completely stationary.
    //
    diff = headingD - heading;
    if (diff < -345.0)            // Not sure yet what the best way to do this is.
      diff += 360;				  // I.e., I want change -355's to -5's, and preserve -5's.

    Serial.print( "  diff: ");
    Serial.println( diff, 0);
    
    if (captureAvgs && 
       (headingD < STARTINGDEGREES || headingD > STARTINGDEGREES + NUMDEGREES - 1))
    {
      Serial.print( "x: ");
      Serial.println( heading);
      return;
    }
  }
  
  if (!captureAvgs)
    return;
  
  if (diffCounts[headingNdx] == -NUMSAMPLES)          // Our work here is done.
  {
    Serial.print( headingNdx + STARTINGDEGREES);
    Serial.print( ": **: ");
    Serial.println( numDiffsRemaining);
    return;
  }
  else if (diffCounts[headingNdx] == NUMSAMPLES)
  {
    Serial.print( headingNdx + STARTINGDEGREES);
    Serial.print( ": *: ");
    Serial.println( --numDiffsRemaining);

    diffCounts[headingNdx] = -NUMSAMPLES;
    return;
  }
  else if (diffCounts[headingNdx] > NUMSAMPLES)
  {
    Serial.print( headingNdx + STARTINGDEGREES);	// Sanity check:
    Serial.print( ": *********************: ");		// Should never reach this point.
    Serial.println( numDiffsRemaining);
    return;
  }

  oldDenominator = (diffCounts[headingNdx])++;	// Maybe better to divide at the end,
  newDenominator = oldDenominator + 1;			// not sure.
  diffAvgs[headingNdx] = 						// Dilute existing avg. to add in new one.
    (diffAvgs[headingNdx] * oldDenominator + diff) / newDenominator;
}
