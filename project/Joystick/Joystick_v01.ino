// Thumb Joystick example
// Mike Grusin, SparkFun Electronics 3/11
// This code is free, baby. Use it however you like.

// This sketch shows the basic operation of the Thumb Joystick (COM-09032) and breakout board (BOB-09110).
// The joystick outputs two analog voltages (VERT and HORIZ), and one digital signal (SEL) for the pushbutton.

// Connections to joystick (change if you use different pins):

#define HORIZ_ORANGE_WIRE A7  // analog
#define VERT_GREEN_WIRE A6    // analog

// Also connect the joystick VCC to Arduino 5V, and joystick GND to Arduino GND.

// This sketch outputs serial data at 9600 baud (open Serial Monitor to view).

const int xMin    =    0;    // or 1, or 2.
const int xMax    = 1021;    // or 1022.
const int xCenter =  513;
const int xAvg    =  510;

const int yMin    =    0;    // or 1 or 2.
const int yMax    = 1023;    // or 1022.
const int yCenter =  514;    // or 503.
const int yAvg    =  511;

const int noiseBand = 2;

int prevHorizontal = 0, prevVertical = 0;
int horizontal, vertical;
double xVal  = 0, yVal = 0;
int rVal = 0, prevRval = -1;
double thetaVal = 0, prevThetaVal = -1.0;

// The Plus and Minus values above define an Ellipse,
// in which to define R-Theta values.
//
boolean get_R_Theta()
{
  boolean changed = false;
  
  horizontal = analogRead( HORIZ_ORANGE_WIRE);  // will be 0-1023.
  vertical   = analogRead( VERT_GREEN_WIRE);    // will be 0-1023.
  
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
    
  rVal = sqrt( xVal * xVal + yVal * yVal);
  thetaVal = atan2( yVal, xVal) * 180.0 / PI;
  
  // Let's smooth out the values to (r % 5), (theta % 10.0);
  rVal -= rVal % 5;  
  thetaVal = int( thetaVal / 10.0) * 10.0;
  
  return changed;
  
  if (rVal != prevRval || fabs( thetaVal - prevThetaVal) >= 1.0)
  {
    prevRval = rVal;
    prevThetaVal = thetaVal;
    changed = true;
  }

  return changed;
}

void setup()
{
  Serial.begin( 9600);
}

void loop() 
{
  if (!get_R_Theta())
    return;
/*
  Serial.print( "horizontal: ");
  Serial.print( horizontal, DEC);
  Serial.print( " vertical: ");
  Serial.print( vertical, DEC);
*/
  Serial.print( "xVal: ");
  Serial.print( xVal, 0);
  Serial.print( " :yVal: ");
  Serial.println( yVal, 0);
/*
  Serial.print( "  rVal: ");
  Serial.print( rVal, DEC);
  Serial.print( "   thetaVal: ");
  Serial.println( thetaVal, 2);
*/
}  

