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
double thetaVal0 = 0, thetaVal = 0, prevThetaVal = -1.0;

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
const int rotationModeFlag    = 100;
const int numSectors          = 8;
const int degreesPerSector    = 360 / numSectors;
const int noiseCeiling        = 120;
const int fullScaleFloor      = 900;
const int numberOfSpeedRanges = 8;
const int speedRangeWidth = 
  (fullScaleFloor - noiseCeiling) / numberOfSpeedRanges;
  
int speedRangeCeilings[numberOfSpeedRanges + 1];

void setSpeedRangeValues()
{ 
  for (int i = 0 ; i <= numberOfSpeedRanges ; i++)
  {
    speedRangeCeilings[i] = noiseCeiling + speedRangeWidth * i;
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

// The Plus and Minus values above define an Ellipse,
// in which to define R-Theta values.
//
boolean get_R_Theta()
{
  boolean changed = false;
  
  horizontal = analogRead( HORIZ_ORANGE_WIRE);  // will be 0-1023.
  vertical   = analogRead( VERT_GREEN_WIRE);    // will be 0-1023.
  
  const int noiseBand = 3;
  
  if (abs( horizontal - prevHorizontal) > noiseBand)
    changed = true;
  if (abs( vertical - prevVertical) > noiseBand)
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
    
  rVal0 = sqrt( xVal * xVal + yVal * yVal);
  thetaVal0 = int( atan2( yVal, xVal) * 180.0 / PI + 360.0) % 360;
  
  // Let's smooth out the values according to our criteria above.
  // Add numSectors to shist us upwards so we don't have to cross zero.
  //
  thetaVal = int( thetaVal0 / degreesPerSector) + numSectors;
  
  // Find the first Ceiling that the value is less than.
  //
  int i;
  for (i = 0 ; i <= numberOfSpeedRanges ; i++)
  {
    if (rVal0 < speedRangeCeilings[i])
      break;
  }
  rVal = i;    // 0 == in the noise bulls-eye.

  // We're in the outer-most ring, meaning we're in Rotation mode,
  // so just indicate if we're turning CW (= -) or CCW (= +);
  //
  if (i > numberOfSpeedRanges)
    rVal = rotationModeFlag * signI( thetaVal - prevThetaVal);
  
  changed = false;            // Re-start our considerations.

  if (rVal != prevRval || fabs( thetaVal - prevThetaVal) >= 1.0)
  {
    if (abs( prevRval) < rotationModeFlag ||
        abs( rVal) >= rotationModeFlag)
    {
      changed = true;    // Try to prevent bogus point when snapping      
    }
    else                 // back to center.
    {
//      Serial.print( "No #2: ");
//      debugPrint();
        delay( 250);     // Simplify preventing unwanted Snap-back 
    }                    // points.
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

#define SETBUTTON 4

void setup()
{
  Serial.begin( 9600);
  
  // Our "Set" Button.
  // It has a 10k Ohm pull-up resistor,
  // so LOW == Button is being pushed.
  //
  pinMode( SETBUTTON, INPUT);      

  setSpeedRangeValues();  
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
  Serial.println( thetaVal, 0);
}

void loop() 
{
  if (!get_R_Theta())
    return;
    
  buttonState = digitalRead( SETBUTTON);
  
  debugPrint();

  delay( 250);
}  

