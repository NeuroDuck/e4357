// This sketch shows the basic operation of the Thumb Joystick (COM-09032) and breakout board (BOB-09110).
// The joystick outputs two analog voltages (VERT and HORIZ), and one digital signal (SEL) for the pushbutton.

#define HORIZ_ORANGE_WIRE  A7    // Analog.
#define VERT_GREEN_WIRE    A6    // Analog.
#define DISPLAY_GREEN_WIRE 11    // Digital.
#define SETDIRBUTTON       12
#define LED                13

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

const int   numThetaSectors       = 16;
const float degreesPerThetaSector = 360.0 / numThetaSectors;
const int   noiseCeiling          = 250;
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

#define DEBUG0

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

#ifdef DEBUG
  Serial.print( "\n(r, theta) = (");
  Serial.print( rVal0);
  Serial.print( " : ");
  Serial.print( rVal);
#endif

  // Calculate thetaVal.
  // Shift the values from +/- 180 to 0-359.
  //
  thetaVal0 =
    int( atan2( yVal, xVal) * 180.0 / PI + degreesPerCircleF) % degreesPerCircleI;

#ifdef DEBUG
  Serial.print( ", ");
  Serial.print( thetaVal0);
  Serial.println( ")");
#endif

  // Determine which thetaSector we are in.
  // To do this, we'll convert the 0->359 range to 0->(numThetaSectors - 1).
  //
  // Rotate thetaVal half a sector CW, so sectors will straddle their center-theta's.
  //
  thetaVal0 = int( thetaVal0 + degreesPerThetaSector / 2.0) % degreesPerCircleI;

#ifdef DEBUG
  Serial.print( "thetaVal0b = ");
  Serial.print( thetaVal0);
#endif

  thetaVal = int( thetaVal0 / degreesPerThetaSector);

#ifdef DEBUG
  Serial.print( " => thetaVal = ");
  Serial.println( thetaVal);
#endif

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

#ifdef DEBUG
    Serial.print( "squashed          newThetaVal = ");
    Serial.println( newThetaVal);
#endif

    // Re-stretch out the value to spread it back across the original # of sectors.
    newThetaVal = newThetaVal * 2;

#ifdef DEBUG
    Serial.print( "newThetaVal2 = ");
    Serial.println( newThetaVal);
#endif

    if (newThetaVal != thetaVal)
    {
#ifdef DEBUG
      Serial.print( "Changing thetaVal from ");
      Serial.print( thetaVal);
      Serial.print( " to ");
      Serial.println( newThetaVal);
#endif
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

#include <SoftwareSerial.h>
SoftwareSerial displaySerial =
  SoftwareSerial( 255, DISPLAY_GREEN_WIRE);

bool ledState        = HIGH;
bool curButtonState  = ledState;
bool prevButtonState = !curButtonState;

void setup()
{
  // Fio needs to start this way to support Wireless Programming.
  //
  Serial.begin( 57600);
  displaySerial.begin( 19200);
  delay( 100);                  // Ensure that the Display has
                                // finished initializing.
  // Our "Set" Button.
  // It has a 10k Ohm pull-up resistor,
  // so LOW == Button is being pushed.
  //
  pinMode( SETDIRBUTTON, INPUT_PULLUP);
  pinMode( LED, OUTPUT);
  pinMode( DISPLAY_GREEN_WIRE, OUTPUT);

  displaySerial.write( 12);                 // Clear.
  delay( 5);                                // Required delay.
  displaySerial.write( 22);                 // Display on, no cursor.
  
  digitalWrite( LED, ledState); // Set up the Output LED for initial state.

  //  plataDuckBulletin();
  //  playSongOfTheWind_Suzuki3();

  setRadiusRangeValues();
}

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

// Make our print format more structured, to simplify deciding if it's been
// received intact by the Robot.
//
// Total# of bytes sent (up to but not including the "\n") == 8, i.e., ...
//   12345678
//   !6 11 17
//
void mbedPrint( char* msg)
{
  msg[0] = '!';                  // #1.

  msg[1] = '0' + rVal;           // For now, this is always one digit.  #2.
  msg[2] = ' ';                  // #3.

  int downshiftedThetaVal = thetaVal - numThetaSectors;
  int tensDigit = downshiftedThetaVal / 10;
  int onesDigit = downshiftedThetaVal % 10;

  msg[3] = '0' + tensDigit;      // #4.
  msg[4] = '0' + onesDigit;      // #5.
  msg[5] = ' ';                  // #6.

  int checksum = rVal + downshiftedThetaVal;
  tensDigit = checksum / 10;
  onesDigit = checksum % 10;

  msg[6] = '0' + tensDigit;      // #7.
  msg[7] = '0' + onesDigit;      // #8.
  msg[8] = '\0';

  displayOn1stLine( msg);
//  Serial.println( msg);
}

void displayOn1stLine( char* msg)
{
  displaySerial.write( 128);                // Move to (0,0).
  displaySerial.print( msg);
}

void displayCharOn2ndLine( char c, int pos)
{
  displaySerial.write( 148 + pos);            // Move to (1, pos).
  displaySerial.write( c);
}

void debounceButton( int buttonPinNum, int ledPinNum)
{
  curButtonState = digitalRead( buttonPinNum);
  
  if (curButtonState == HIGH && prevButtonState == LOW)
  {
    delay( 1);                      // Crude form of button debouncing.

    if (ledState == HIGH)
    {
      digitalWrite( ledPinNum, LOW);
      ledState = LOW;
    } 
    else 
    {
      digitalWrite( ledPinNum, HIGH);
      ledState = HIGH;
    }
  }
  prevButtonState = curButtonState;
}

int i = 0;
char mbedMsg[10];
bool buttonState;

void loop()
{
  debounceButton( SETDIRBUTTON, LED);
  
  if (ledState)
    displaySerial.write( 17);                 // Turn backlight on.
  else
    displaySerial.write( 18);                 // Turn backlight off.

  if (get_R_Theta())
    mbedPrint( mbedMsg);
    
  delay( 250);                    // Give the Robot time to respond.

  bool firstChar = true;
  while (Serial.available())
  {
    char c = Serial.read();
    if (c != '\r' && c != '\n')
    {
      if (firstChar)
      {
        displaySerial.write( 12);                 // Clear.
        delay( 5);                                // Required delay.
        displayOn1stLine( mbedMsg);
        firstChar = false;
        i = 0;
      }
      displayCharOn2ndLine( c, i++);
    }
  }
}

void plataDuckBulletin()
{
  displaySerial.write( 18);                 // Turn backlight off.
  displaySerial.write( 12);                 // Clear.

  //  while( digitalRead( SETDIRBUTTON))
  //  {};

  displaySerial.write( 17);                 // Turn backlight on.
  delay( 5);                                // Required delay.
  displaySerial.print( "I am a PlataDuck..");
  delay( 1500);
  displaySerial.print( "Quack! ");
  delay( 1000);
  displaySerial.print( "Quack! ");
  delay( 2000);
  displaySerial.write( 12);                 // Clear.
  displaySerial.print( "I Quack...");
  displaySerial.write( 13);                 // Newline.
  delay( 1500);
  displaySerial.print( "therefore I am!");

  delay( 2000);
  displaySerial.write( 18);                 // Turn backlight off.
  displaySerial.write( 12);                 // Clear.
}

// d = 1/duration of notes.
// We're not supporting 1/64th-notes duration.
// D(1)  = Whole note (= 2 sec.), stored as a 0x1, up to
// D(32) = 1/32nd note(= 1/16 sec.), stored as 32.
//
#define D(d) 0x80+d

void playSongOfTheWind_Suzuki3()
{
  displaySerial.write( 12);                 // Clear.
  displaySerial.write( 17);                 // Turn backlight on.
  delay( 5);                                // Required delay.
  displaySerial.write( "Song of the Wind");

  // For me, C, F, & G are Flat.
  // A, A#, B, C, C#, D, D#, E, F, F#, G, G#, Rest
  // -- x      x  --     x      x  --  x  --
  // 0, 1 , 2, 3, 4 , 5, 6 , 7, 8, 9 ,10, 11, 12.
  byte data[] = {
    //8th  4th  A  B  C# D  E  E  E  E  F# D  5th A 4th F#  4th   E  Rest
    D(8), 24, 0, 2, 4, 5, 7, 7, 7, 7, 9, 5, 25, 0, 24, 9, D(4), 7, 12,
    //     F#  D 5th  A 4th F#  4th   E Rest 8th   E  D  D  D  D  C# C#
    D(8), 9, 5, 25, 0, 24, 9, D(4), 7, 12, D(8), 7, 5, 5, 5, 5, 4, 4,
    //C# C# B  B  B
    4, 4, 2, 2, 2,
    //A  C# 4th   E  8th   E  D  D  D  D C# C# C# C#  B  B  B  4th   A
    0, 4, D(4), 7, D(8), 7, 5, 5, 5, 5, 4, 4, 4, 4, 2, 2, 2, D(4), 0
  };
  int numNotes = sizeof( data) / sizeof( byte);

  int currentDurationNdx;
  int currentDurationReciprocal = 0;
  int numSixteenthsDuration = 0;

  for (int i = 0 ; i < numNotes ; i++)
  {
    if (data[i] / 10 == 2)
      displaySerial.write( 212 + data[i] - 20);         // Set nth Scale.
    else if (data[i] > 127)
    {
      currentDurationReciprocal = data[i] & 0x3f;
      currentDurationNdx = log2( currentDurationReciprocal);
      displaySerial.write( 209 + 5 - currentDurationNdx);  // Set 1/nth duration.
    }
    else    // 1/32note = 1/16 sec.
    {
      displaySerial.write( 220 + data[i]);                 // Play a note.
      numSixteenthsDuration += 32 / currentDurationReciprocal;
    }
  }

  Serial.println( numSixteenthsDuration);
  int delayProduct = numSixteenthsDuration * 1000L / 16 + 2000;
  Serial.println( delayProduct);
  delay( delayProduct);
  displaySerial.write( 18);                 // Turn backlight off.
  displaySerial.write( 12);                 // Clear.
}

int log2( int n)
{
  int result = 0;

  while (n > 1)
  {
    n /= 2;
    result++;
  }

  return result;
}
