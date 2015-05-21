// Thumb Joystick example
// Mike Grusin, SparkFun Electronics 3/11
// This code is free, baby. Use it however you like.

// This sketch shows the basic operation of the Thumb Joystick (COM-09032) and breakout board (BOB-09110).
// The joystick outputs two analog voltages (VERT and HORIZ), and one digital signal (SEL) for the pushbutton.

// Connections to joystick (change if you use different pins):

#define HORIZ_ORANGE_WIRE A7  // analog
#define VERT_GREEN_WIRE A6    // analog

// Also connect the joystick VCC to Arduino 5V, and joystick GND to 
// Arduino GND.

// This sketch outputs serial data at 9600 baud (open Serial Monitor 
// to view).

const int xMin    =    0;    // or 1, or 2.
const int xMax    = 1021;    // or 1022.
const int xCenter =  513;
const int xAvg    =  510;

const int yMin    =    0;    // or 1 or 2.
const int yMax    = 1023;    // or 1022.
const int yCenter =  514;    // or 503.
const int yAvg    =  511;

int prevHorizontal = 0, prevVertical = 0;
int horizontal, vertical;
double xVal  = 0, yVal = 0;
int rVal0 = 0, rVal = 0, prevRval = -1;
int thetaVal0 = 0, thetaVal = 0, prevThetaVal = -1;

// To further increase the noise rejection of the JoyStick program,
// I'm thinking of cutting its Theta output into 8 sectors.  
// For the R value, I'll first try cutting it down to be one of 5 values: 
// 
//     < 75  = noise, i.e., ignore
//	next 1/3rd = 1/3rd of full-speed in the specified direction, 
//                   after rotation with reference to the Tilt-Compensated 
//                   Compass (= 3-axis Magnetometer)
//	next 1/3rd = 2/3ds of full speed, ditto.
//	last 1/3rd = full speed, ditto.	
//	> 900 = full-scale deflection, = Rotation mode, 
//              rather than Bearing mode.
//
// I'll rotate the Robot in Rotation mode then push the SETBUTTON, 
// to specify which way "North" is in the room I'm exploring.
//
const int   numThetaSectors       = 16;
const float degreesPerThetaSector = 360.0 / numThetaSectors;
const int   noiseCeiling          = 120;
const int   fullScaleFloor        = 900;
const int   numRadiusRanges  = 8;
const int   radiusRangeWidth = (fullScaleFloor - noiseCeiling) / numRadiusRanges;
  
int radiusRangeCeilings[numRadiusRanges + 1]; // +1 for noise bulls-eye.

void setRadiusRangeValues()
{ 
  for (int i = 0 ; i <= numRadiusRanges ; i++)
  {
    radiusRangeCeilings[i] = noiseCeiling + radiusRangeWidth * i;
  }
}
 
int signI( int i)
{
  if (i < 0)
    return -1;
    
  if (i > 0)
    return 1;
    
  return 2;        // So 0 ends up being 2 * rotationModeFlag;
}

// The Plus and Minus values above define an Ellipse, in which to define R-Theta values.
//
boolean get_R_Theta()
{
  boolean changed = false;
  
  horizontal = analogRead( HORIZ_ORANGE_WIRE);  // will be 0-1023.
  vertical   = analogRead( VERT_GREEN_WIRE);    // will be 0-1023.
  
  const int perSampleNoiseBand = 3;
  
  if (abs( horizontal - prevHorizontal) > perSampleNoiseBand)
    changed = true;
  if (abs( vertical - prevVertical) > perSampleNoiseBand)
    changed = true;
      
  if (!changed)
    return changed;
  
  prevHorizontal = horizontal;
  prevVertical   = vertical;
  
  if (horizontal < xCenter)
    xVal = map( horizontal, xMin, xCenter, -1000, 0);
  else if (horizontal > xCenter)
    xVal = map( horizontal, xCenter, xMax, 0, 1000);
  else
    xVal = 0;

  if (vertical < yCenter)
    yVal = map( vertical, yMin, yCenter, -1000, 0);
  else if (vertical > yCenter)
    yVal = map( vertical, yCenter, yMax, 0, 1000);
  else
    yVal = 0;
    
  // Convert (x, y) Cartesian Coordinates to (r, theta) Polar Coordinates.
  // Convert r into one of <numRadiusRanges> radiusRanges, 
  // and theta into one of <numThetaSectors> thetaSectors.
  //
  rVal0 = sqrt( xVal * xVal + yVal * yVal);
  
  // Find the first radiusRangeCeiling that rVal0 is less than.
  //
  int i;
  for (i = 0 ; i <= numRadiusRanges ; i++)
  {
    if (rVal0 < radiusRangeCeilings[i])
      break;
  }

  // 0 == we're in the noise bulls-eye.
  // i == numRadiusRanges, means we're in the full-deflection outer-donut,
  //      which formerly had special significance, indicating Rotation, and now
  //      is treated the same as other RadiusRanges.
  rVal = i;
  
  const float degreesPerCircleF = 360.0;
  const int   degreesPerCircleI = 360;
  
  Serial.print( "\n(r, theta) = (");
  Serial.print( rVal);
  
  // Calculate thetaVal.
  // Shift the values from +/- 180 to 0-359.
  //
  thetaVal0 = 
    int( atan2( yVal, xVal) * 180.0 / PI + degreesPerCircleF) % degreesPerCircleI;
  
  Serial.print( ", ");
  Serial.print( thetaVal0);
  Serial.println( ")");

  // Determine which thetaSector we are in.
  // To do this, we'll convert the 0->359 range to 0->(numThetaSectors - 1).
  //
  // Rotate thetaVal half a sector CW, so sectors will straddle their center-theta's.
  //
  thetaVal0 = int( thetaVal0 + degreesPerThetaSector / 2.0) % degreesPerCircleI;
  
  Serial.print( "thetaVal0b = ");
  Serial.print( thetaVal0);

  thetaVal = int( thetaVal0 / degreesPerThetaSector);
  
  Serial.print( " => thetaVal = ");
  Serial.println( thetaVal);
  
  // When rVal <= numRadiusRanges / 2, the JoyStick cannot resolve 16 thetaSectors, 
  // so smash the 3 {slight,medium,hard} types of turns into only {medium} turns.
  //  
  const int sectorsTooNarrowBelowRadiusRangeNdx = numRadiusRanges / 2 + 3;
  const int numQuadrants = 4;
  
  if (rVal < sectorsTooNarrowBelowRadiusRangeNdx)
  {    
    // Rotate thetaVal another half a sector CW, so sectors will straddle their 
    // center-theta's for double the degreesPerThetaSector.
    //
    thetaVal0 = int( thetaVal0 + degreesPerThetaSector / 2.0) % degreesPerCircleI;    

    // Re-compute thetaVal based on having half as many thetaSectors.
    //
    int newThetaVal = int( thetaVal0 / (degreesPerThetaSector * 2));

    Serial.print( "squashed          newThetaVal = ");
    Serial.println( newThetaVal);

    // Re-stretch out the value to spread it back across the original # of sectors.
    newThetaVal = newThetaVal * 2;

    Serial.print( "newThetaVal2 = ");
    Serial.println( newThetaVal);
    
//    if (newThetaVal != thetaVal)
    {
      Serial.print( "Changing thetaVal from ");
      Serial.print( thetaVal);
      Serial.print( " to ");
      Serial.println( newThetaVal);
      thetaVal = newThetaVal;
    }
  }
  
  // Lastly, we add in a full-circle, to prevent crossing zero in the next 
  // iteration, when subtracting from prevThetaVal.
  //
  thetaVal += numThetaSectors;

  changed = false;            // Re-start our considerations.

  if (rVal != prevRval || fabs( thetaVal - prevThetaVal) >= 1.0)
  {
      changed = true;    // Try to prevent bogus point when snapping      

      prevRval = rVal;
      prevThetaVal = thetaVal;
  }                      
  else
  {
//    Serial.print( "No #1: ");
//    debugPrint();
  }

  return changed;
}

#define SETDIRBUTTON 4

void setup()
{
  // Non-Fio Serial startup.
  //
//  Serial.begin( 9600);

  // Fio needs to start this way to support Wireless Programming.
  //
  Serial.begin( 57600);
//  Serial.println( 255);
  
  // Our "Set" Button.
  // It has a 10k Ohm pull-up resistor,
  // so LOW == Button is being pushed.
  //
  pinMode( SETDIRBUTTON, INPUT);      

  setRadiusRangeValues();  
  
//  debugPrint();
}

int buttonState;

void debugPrint()
{
  Serial.print( "horizontal: ");
  Serial.print( horizontal, DEC);
  Serial.print( " vertical: ");
  Serial.print( vertical, DEC);

/*
  Serial.print( "button = ");
  Serial.print( buttonState, DEC);
  Serial.print( "  ");
*/
  Serial.print( "   xVal: ");
  Serial.print( xVal, 0);
  Serial.print( " yVal: ");
  Serial.print( yVal, 0);

  Serial.print( "   rVal0: ");
  Serial.print( rVal0, DEC);
  Serial.print( " rVal: ");
  Serial.print( rVal, DEC);
  Serial.print( " thetaVal0: ");
  Serial.print( thetaVal0, 0);
  Serial.print( " thetaVal: ");
  Serial.println( thetaVal, 4);
}

void mbedPrint()
{
  Serial.print( rVal, DEC);
  Serial.print( " ");
  Serial.println( thetaVal - numThetaSectors, DEC);
}

void loop() 
{  
  if (!get_R_Theta())
    return;
    
  buttonState = digitalRead( SETDIRBUTTON);
  
  mbedPrint();

  delay( 250);
}  

