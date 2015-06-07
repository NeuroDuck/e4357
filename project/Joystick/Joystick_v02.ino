// This sketch shows the basic operation of the Thumb Joystick (COM-09032) and breakout board (BOB-09110).
// The joystick outputs two analog voltages (VERT and HORIZ), and one digital signal (SEL) for the pushbutton.

#define BATTERY_BLUE_WIRE  A5    // Analog.
#define HORIZ_ORANGE_WIRE  A7    // Analog.
#define VERT_GREEN_WIRE    A6    // Analog.

#define SET_DIR_BUTTON     11    // Digital.

#define DISPLAY_GREEN_WIRE 12    // Digital.
#define LED                13

#define DEBUGx

int signI( int i)
{
  if (i < 0)
    return -1;

  if (i > 0)
    return 1;

  return 2;        // So 0 ends up being 2 * rotationModeFlag;
}

const int firstSwitchPinNum = 2;
const int lastSwitchPinNum  = SET_DIR_BUTTON;
const int numSwitchPins     = lastSwitchPinNum - firstSwitchPinNum + 1;

const int adcSwitches[] = {A0};  // = 14;
const int numADCswitches = sizeof( adcSwitches) / sizeof( int);
const int lastADCswitchPinNum = adcSwitches[numADCswitches - 1];

// JoyStick Calibration Procedure:
//
// CALIBRATING == 'c' => Cartesian.  Do this first, copy results into vars.
// CALIBRATING == 'p' => Polar.      Do this second.
// CALIBRATING == 'x' => Calibrated normal operation.
//
// To run the Calibration routine...
//
// Append "x" to #define DEBUG, it it's not there already, to allow "c"-Calibration
// to free-run.
//
// Change CALIBRATING's value to 'c'.  This specifies collection of the initial 
// Cartesian Calibration values.
// Move the JoyStick around in all directions, and enter the last displayed values 
// under the #else below.  Don't change the values above the #else.
//
// Change CALIBRATING's value to 'p'.  This specifies collection of Polar Calibration
// values.
//
#define CALIBRATING 'x'

#if CALIBRATING == 'c'
int xMin    = 9999;
int xMax    =    0;
int xCenter =    0;

int yMin    = 9999;
int yMax    =    0;
int yCenter =    0;
#else
int xMin    =    0;
int xMax    = 1023;
int xCenter =  515;

int yMin    =   53;
int yMax    = 1023;
int yCenter =  515;
// int maxRperTheta[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int maxRperTheta[] = {  // Different directions have different max. values.
  1021,1103,1054,1097,1021,1185,1238,1192,1022,1206,1184,1229,1139,1169,1075,1119
};
#endif

const int   numThetaSectors           =  16;
const float degreesPerThetaSector     = 360.0 / numThetaSectors;
const float degreesPerHalfThetaSector = degreesPerThetaSector / 2.0;
const float degreesPerTwoThetaSectors = degreesPerThetaSector * 2.0;
const int   noiseCeiling              = 250;
const int   fullScaleFloor            = 900;
const int   numRadiusRanges           =   8;
const int   radiusRangeWidth = (fullScaleFloor - noiseCeiling) / numRadiusRanges;

int radiusRangeCeilings[numRadiusRanges + 1]; // +1 for noise bulls-eye.

void setRadiusRangeValues()
{
  for (int i = 0 ; i <= numRadiusRanges ; i++)
  {
    radiusRangeCeilings[i] = noiseCeiling + radiusRangeWidth * i;
  }
}

int prevHorizontal = 0, prevVertical = 0;
int rVal0 = 0, rVal = 0, prevRval = 0;
int thetaVal0 = 0, thetaVal = 0, prevThetaVal = -1;  // Ditto.

#ifdef DEBUG
int maxTimesThrough = 10;
#endif

// Only called when CALIBRATING is == 'c' or 'p'.
//
void calibrate_R_Theta()
{
#ifdef DEBUG
  if (maxTimesThrough <= 0)
    return;
  maxTimesThrough--;
#endif

  int horizontal = analogRead( HORIZ_ORANGE_WIRE);  // Theoretically, will be 0-1023.
  int vertical   = analogRead( VERT_GREEN_WIRE);    // Theoretically, will be 0-1023.  

#if CALIBRATING == 'c'
  if (horizontal < xMin && horizontal != 0)     // It seems to love to 
    xMin = horizontal;                          // inaccurately jump to 0.
  if (horizontal > xMax)
    xMax = horizontal;
  if (vertical < yMin && vertical != 0)         // Ditto.
    yMin = vertical;
  if (vertical > yMax)
    yMax = vertical; 
   
  Serial.print( "x: ");
  Serial.print( xMin);
  Serial.print( ",");
  Serial.print( xMax);
  Serial.print( "  y: ");
  Serial.print( yMin);
  Serial.print( ",");
  Serial.print( yMax);
  Serial.print( "  ");
  Serial.print( "  center: ");
  Serial.print( horizontal);
  Serial.print( ",");
  Serial.println( vertical);
#else
  double xVal, yVal;

  mapHVtoXY( horizontal, vertical, &xVal, &yVal);
  mapXYtoRTheta( xVal, yVal, &rVal, &thetaVal);

  if (maxRperTheta[thetaVal] < rVal0)
    maxRperTheta[thetaVal] = rVal0;
  
  for (int i = 0 ; i < numThetaSectors ; i++)
  {
    Serial.print( maxRperTheta[i]);
    if (i < numThetaSectors - 1)
      Serial.print( ",");
  }
  Serial.println( "");
#endif
}

void mapHVtoXY( int h, int v, double* xVal, double* yVal)
{
#ifdef DEBUG
  Serial.print( "\n(h, v) = (");
  Serial.print( h);
  Serial.print( ", ");
  Serial.print( v);
  Serial.println( ")");
#endif
  
  // Map us to cover +/-1000 in both x and y.
  if (h < xCenter)
    *xVal = mapD( h, xMin, xCenter, -1000, 0);
  else if (h > xCenter)
    *xVal = mapD( h, xCenter, xMax, 0, 1000);
  else
    *xVal = 0;

  if (v < yCenter)
    *yVal = mapD( v, yMin, yCenter, -1000, 0);
  else if (v > yCenter)
    *yVal = mapD( v, yCenter, yMax, 0, 1000);
  else
    *yVal = 0;

#ifdef DEBUG
  Serial.print( "(x, y) = (");
  Serial.print( *xVal, 0);
  Serial.print( ", ");
  Serial.print( *yVal, 0);
  Serial.println( ")");
#endif
}

double mapD( long x, long imin, long imax, long omin, long omax)
{
#ifdef DEBUG
  Serial.print( "mapD( d:");
  Serial.print( x);
  Serial.print( " imin:");
  Serial.print( imin);
  Serial.print( " imax:");
  Serial.print( imax);
  Serial.print( " omin:");
  Serial.print( omin);
  Serial.print( " omax:");
  Serial.print( omax);
  Serial.print( ") = ");
#endif

  double numP1 = x - imin;
  double numP2 = omax - omin + 1;
  double denominator = imax - imin + 1;
  double result = omin + numP1 * numP2 / denominator;
  
#ifdef DEBUG
  Serial.print( omin);
  Serial.print( " + ");
  Serial.print( numP1, 0);
  Serial.print( " * ");
  Serial.print( numP2, 0);
  Serial.print( " / ");
  Serial.print( denominator, 0);
  Serial.print( " = ");
  Serial.println( result);
#endif

  return result;
}

const int   degreesPerCircleI = 360;
const float degreesPerCircleF = 360.0;

// Convert (x, y) Cartesian Coordinates to (r, theta) Polar Coordinates.
// Convert r into one of <numRadiusRanges> radiusRanges,
// and theta into one of <numThetaSectors> thetaSectors.
//
void mapXYtoRTheta( double xVal, double yVal, int* rVal, int* thetaVal)
{
  rVal0 = sqrt( xVal * xVal + yVal * yVal) + 0.5;

#ifdef DEBUG
  Serial.print( "rVal0 = ");
  Serial.println( rVal0);
#endif

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
  *rVal = i;

#ifdef DEBUG
  Serial.print( "(r, theta) = (");
  Serial.print( rVal0);
  Serial.print( " : ");
  Serial.print( *rVal);
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
  thetaVal0 = int( thetaVal0 + degreesPerHalfThetaSector) % degreesPerCircleI;

#ifdef DEBUG
  Serial.print( "thetaVal0b = ");
  Serial.print( thetaVal0);
#endif

  *thetaVal = int( thetaVal0 / degreesPerThetaSector);

#ifdef DEBUG
  Serial.print( " => thetaVal = ");
  Serial.println( *thetaVal);
#endif
}

boolean get_R_Theta()
{
  boolean changed = false;
  double xVal = 0, yVal = 0;

  int horizontal = analogRead( HORIZ_ORANGE_WIRE);  // will be 0-1023.
  int vertical   = analogRead( VERT_GREEN_WIRE);    // will be 0-1023.

  const int perSampleNoiseBand = 3;
  if (abs( horizontal - prevHorizontal) <= perSampleNoiseBand &&
      abs( vertical - prevVertical) <= perSampleNoiseBand)
    return changed;

  prevHorizontal = horizontal;
  prevVertical   = vertical;

  mapHVtoXY( horizontal, vertical, &xVal, &yVal);
  mapXYtoRTheta( xVal, yVal, &rVal, &thetaVal);

  // When rVal <= numRadiusRanges / 2, the JoyStick cannot resolve 16 thetaSectors,
  // so smash the 3 {slight,medium,hard} types of turns into only {medium} turns.
  //
  // Alas, this means that when driving slowly, only medium turns are possible, 
  // but so be it, until I can devise a new strategy to handle this shortcoming
  // of the JoyStick.
  //                      // + 3 more, to smash even more RadiusRanges for now.
  const int sectorsTooNarrowBelowRadiusRangeNdx = numRadiusRanges / 2 + 3;

  if (rVal < sectorsTooNarrowBelowRadiusRangeNdx)
  {
    // Rotate thetaVal another half a sector CW, so sectors will straddle their
    // center-theta's for Sectors that are degreesPerTwoThetaSectors degrees wide.
    //
    thetaVal0 = int( thetaVal0 + degreesPerHalfThetaSector) % degreesPerCircleI;

    // Re-compute thetaVal based on having half as many thetaSectors.
    //
    int newThetaVal = int( thetaVal0 / degreesPerTwoThetaSectors);

#ifdef DEBUG
    Serial.print( "squeezed         newThetaVal = ");
    Serial.println( newThetaVal);
#endif

    // Re-stretch out the value to spread it back across the original # of sectors.
    newThetaVal *= 2;

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
  
  // We want to avoid sending adjacent rVal=0 commands, even if thetaVal has
  // changed.
  //
  changed = false;                     // Re-start our considerations.

  if (prevRval == 0 && rVal == 0)
  {
    // Just ignore this case
#ifdef DEBUG
    Serial.println( "Ignoring r = 0 -> 0");
#endif
  }
  else if (prevRval != 0 && rVal == 0)      // This is our Stop command.
  {
    changed = true;    
    prevRval = rVal;
    // Don't update prevThetaVal in this case.
#ifdef DEBUG
    Serial.println( "Approving: r = !0 -> 0");
#endif
  }
  else if (thetaVal != prevThetaVal)
  {
    changed = true;
    prevRval = rVal;
    prevThetaVal = thetaVal;
#ifdef DEBUG
    Serial.println( "Approving: theta != prevTheta");
#endif
  }

  return changed;
}

// GPIO Pin Config & Access Registers:
// 
// PCICR:
// [PCIE0] enables PCINT[7:0]    (PORTB, PINB, DDRB, PCMSK0).
// [PCIE1] enables PCINT[14:8]   (PORTC, PINC, DDRC, PCMSK1).
// [PCIE2] enables PCINT[23:16]  (PORTD, PIND, DDRD, PCMSK2).

// The Registers that control the Ports are explained here:
//   http://www.arduino.cc/en/Reference/PortManipulation
//
// In short, it says:
//  DDR{B,C,D}  = Pin Direction, also set by PinMode():
//    1 => Output.
//  PORT{B,C,D} = 8 Output Pins together (R/W):
//    when the Pin is an Input:
//      1 => Activate Pull-Up Resistor, also set by PinMode( ...., INPUT_PULLUP);
//    when the Pin is an Output:
//      1 => output == HIGH.
//  PIN{B,C,D}  = 8 Input  Pins together (R/O), however:
//    writing a 1 toggles the corresponding PORT value.

// Let's use these handy #define's in pins_arduino.h and Arduino.h:
//
//  Example:
//    bool pinState = bitRead( 
//      *portInputRegister( digitalPinToPort( pin)),
//      digitalPinToPCMSKbit( pin));
//  or:
//    bool pinState = 
//      *portInputRegister( digitalPinToPort( pin)) & 
//      digitalPinToBitMask( pin);
//
//  digitalPinToBitMask( pin)         // One of: _BV( 0) thru _BV( 7).
//  digitalPinToPort( pin)            // One of: PB 2, PC 3, PD 4.
//  portOutputRegister( portNum)      // One of: &PORTB, &PORTC, &PORTD.
//  portInputRegister( portNum)       // One of: &PINB,  &PINC,  &PIND.
//  portModeRegister( portNum)        // One of: &DDRB,  &DDRC,  &DDRD.
//
//  digitalPinToPCICR( pin)           // Returns &PCICR for 0-21, else 0.
//  digitalPinToPCICRbit( pin)        // One of: 0, 1, 2.
//  digitalPinToPCMSK( pin)           // One of: &PCMSK0, &PCMSK1, &PCMSK2.
//  digitalPinToPCMSKbit( pin)        // One of: 0-7.
//
//  clockCyclesPerMicrosecond()
//  clockCyclesToMicroseconds()
//  microsecondsToClockCycles()
//
//  bitRead(  value, bit);
//  bitSet(   value, bit);
//  bitClear( value, bit);
//  bitWrite( value, bit, bitValue);
//  bit( bitNum);

// Set up Pin Change Interrupts to monitor our DIP Switches.
//
bool setDipSwitchPinMode( int pinNum, bool bValue, char *cValuePtr)
{
  // Configure DDRxn and PORTxn above for this Pin.
  //
  pinMode( pinNum, INPUT_PULLUP);    
  
  // Ensure that this Pin's PORT's Change Interrupts are enabled.
  //
  *digitalPinToPCICR( pinNum) |= _BV( digitalPinToPCICRbit( pinNum));

  // Ensure that this Pin's Pin Change Mask Register bit is set.
  *digitalPinToPCMSK( pinNum) |= _BV( digitalPinToPCMSKbit( pinNum));

  return true;
}
                       // 11        A
                       // 10987654320
char dipSwitchStates[] = "00000000000";
                       // 01234567890

// Loop through the DIP Switches, calling processValueFunction() for each
// of them.
// processValueFunction() will be called with these values for each Switch:
//  pinNum == the Pin# on the Arduino Fio board.
//  bValue == the Pin's current value as a bool.
//  cValuePtr == a pointer to the char representing the value, that is sent
//            to the Robot.  This is to allow modification of the stored char.
//
// Switches connected to ADC pins appear in the list of Switch states after 
// the ones connected to Digital pins.
//
inline int getDipSwitchCharNdx( int pinNum)
{
  if (pinNum <= lastSwitchPinNum)
    return lastSwitchPinNum - pinNum;
  else
    return numSwitchPins + pinNum - lastADCswitchPinNum;
}

inline bool getDipSwitchStateFromPinNum( int pinNum)
{
  return dipSwitchStates[getDipSwitchCharNdx( pinNum)] - '0';
}

inline bool getDipSwitchStateFromCharNdx( int charNdx)
{
  return dipSwitchStates[charNdx] - '0';
}

bool callThisFunctionForThisDipSwitch(
  bool (*processValueFunction)(int pinNum, bool bValue, char *cValuePtr),
  int pinNum)
{
    int charNdx = getDipSwitchCharNdx( pinNum);
    bool bValue = getDipSwitchStateFromCharNdx( charNdx);
    char* cValuePtr = &(dipSwitchStates[charNdx]);

    bool result = (*processValueFunction)( pinNum, bValue, cValuePtr);

    return result;
}

bool callThisFunctionForEveryDipSwitch(
  bool (*processValueFunction)(int pinNum, bool bValue, char *cValuePtr))
{
  bool result = false;
  
  // First process the DIP Switches connected to Digital pins.
  for (int pinNum = firstSwitchPinNum ; pinNum <= lastSwitchPinNum ; pinNum++)
  {
    result |= callThisFunctionForThisDipSwitch( processValueFunction, pinNum);
  }
  
  // Then do the ones connected to ADC pins.
  for (int adcPinNdx = 0 ; adcPinNdx < numADCswitches ; adcPinNdx++)
  {
    result |= 
      callThisFunctionForThisDipSwitch( 
        processValueFunction, adcSwitches[adcPinNdx]);
  }

  return result;
}

#include <TimerOne.h>
#include <SoftwareSerial.h>
SoftwareSerial displaySerial( 
  255, DISPLAY_GREEN_WIRE, false, &dipSwitchesPCINTisr);

// Our PushButton's Input Pin has a Pullup Resistor, so the LED should be 
// HIGH by default.
//
bool ledState             = HIGH;
const unsigned long joyStickSamplesPerSec = 20UL;

void setup()
{
  // Fio needs to start this way to support Wireless Programming.
  //
  Serial.begin( 57600);
  displaySerial.begin( 19200);  // Let's get the chars displayed quickly.
  delay( 100);                  // Ensure that the Display has
                                // finished initializing.
  // Our "Set" Button has the internal pull-up resistor enabled,
  // so LOW == Button is being pushed.
  //
  pinMode( LED, OUTPUT);
  pinMode( DISPLAY_GREEN_WIRE, OUTPUT);
  
  // Set up all our DIP Switches (FIRST_DIP_SWITCH thru LAST_DIP_SWITCH)
  // and SET_DIR_BUTTON, to be inputs with Pull-Up Resistors enabled, so
  // we can use a simple switch to ground to change their states.
  //
  callThisFunctionForEveryDipSwitch( setDipSwitchPinMode);

  displaySerial.write( 12);                 // Clear.
  delay( 5);                                // Required delay.
  displaySerial.write( 22);                 // Display on, no cursor.
  delay( 5);                                // Just in case this is needed.
  
  digitalWrite( LED, ledState); // Set up the Output LED for initial state.

//  plataDuckBulletin();
//  playSongOfTheWind_Suzuki3();

  setRadiusRangeValues();

  displayOn2ndLine( "<JoyStick ready>");

  // Suck up the first junk sampling due to the ADC still initializing, so
  // the ADC will be ready when Timer1 calls SampleJoyStick() just below.
  //
  analogRead( HORIZ_ORANGE_WIRE);  // will be 0-1023.
  analogRead( VERT_GREEN_WIRE);    // will be 0-1023.
 
  // Call this at 20Hz, to do the JoyStick ADC readings.
  Timer1.initialize( 1000UL * 1000UL / joyStickSamplesPerSec);  
  Timer1.attachInterrupt( 
    updateVoltageMeasurementDisplayAndSampleJoyStickViaTimer1);
  
  // Set up ADC for delivering sample results via ISR( ADC_vect). 
  // For now, just use the Arduino-provided analogRead().
  //
  // A single conversion is started by writing a logical one 
  // to the ADC Start Conversion bit, ADSC.
  //
  // If the full 10-bit precision is required, ADCL must be read first, then ADCH, 
  // to ensure that the content of the Data Registers belongs to the same
  // conversion. 
  // Once ADCL is read, ADC access to Data Registers is blocked. When ADCH is read,
  // ADC access to the ADCH and ADCL Registers is re-enabled.
  //
  // Auto Triggering is enabled by setting the ADC Auto Trigger Enable bit, ADATE 
  // in ADCSRA.  This provides a method of starting conversions at fixed intervals.
  //
  // If Auto Triggering is enabled, single conversions can be started by writing 
  // ADSC in ADCSRA to one. ADSC can also be used to determine if a conversion is 
  // in progress. The ADSC bit will be read as one during a conversion, 
  // independently of how the conversion was started.
  // 
  // ADMUX – ADC Multiplexer Selection Register 24.9.1, pg. 248:
  // : REFS[1:0] = 0 => AREF, Internal Vref turned off
  // : ADLAR: ADC Left Adjust Result: 0 => ADC9-8 in ADCH, ADC7-0 in ADCL.
  // : MUX[3:0]: Analog Channel Selection Bits = 0-7 => Sample A0-A7. A8 = Temp.
  //
  // ADCSRA – ADC Control and Status Register A 24.9.2, pg 249:
  // : ADEN  => ADC Enable (Set separately from ADSC, to avoid 25-clock conv.
  //            due to ADC Initialization.
  // : ADSC  => ADC Start one Conversion (when in Single Conversion Mode)
  //         => ADC Start Free-Running Conv. (when in Free-Running Mode)
  //         => Reads as 1 while Conversion is in progress, 0 after.
  // : ADATE => Auto Triggering Enable.  Trigger Source specified in ADCSRB.
  // : ADIE  => ADC Interrupt Enable.
  // : ADPS  => [2:0]: ADC Prescaler Select Bits: 000 => Input Clock = Sys.C./2.
  // 
  // ADCH, ADCL => The ADC Data Register 24.9.3, pg. 250:
  //               When ADCL is read, the ADC Data Register is not updated until 
  //               ADCH is read. Consequently, ADCL must be read first, then ADCH.
  // 
  // ADCSRB – ADC Control and Status Register B 24.9.4, pg. 251:
  // : ACME => Analog Comparator Multiplexer Enable (Only applicable when using 
  //           See Table 23-1, pg. 234.              Analog Comparator).
  // : ADTS[2:0]: ADC Auto Trigger Source = 000 => Free Running Mode.
  //
  // DIDR0 – Digital Input Disable Register 0 24.9.5, pg. 251:
  // : ADC[5:0]D : Digital Input Disable => Set to 1 to not power unused Digital 
  //                                        Input Buffers on A0-5 Pins.
}

volatile bool timeToUpdateVoltageMeasurementAndSampleJoyStick = false;

void updateVoltageMeasurementDisplayAndSampleJoyStickViaTimer1()
{
  timeToUpdateVoltageMeasurementAndSampleJoyStick = true;
}

// Only update Voltage Display once per 60 sec.
const int voltageMeasurementDisplayDivisor = joyStickSamplesPerSec * 60;
int voltageMeasurementDisplayCounter = 0;
bool blockedAwaitingReply = false;

void updateVoltageMeasurementAndSampleJoyStick()
{
  // Do the ADC sampling for the JoyStick's X and Y Axes every time.
  //
  if (get_R_Theta())
  {
    if (!blockedAwaitingReply)
    {
      blockedAwaitingReply = true;
      displayOn2ndLine( "<Btn> = end wait");

      displayAndSendJoyStickCmd();     // Send the JoyStick command to the Robot.
      delay( 250);                     // Give the Robot time to respond.
    }
  }

  // Only update the Voltage Measurement once per 60 sec.
  //
  if (voltageMeasurementDisplayCounter++ % voltageMeasurementDisplayDivisor)
    return;

  char voltageMsg[6];
  int batteryADC = analogRead( BATTERY_BLUE_WIRE);

  // 1.54V on the Voltage Divider = 4.92V in "real life".
  const float voltageDividerScaleFactor = 4.92 / 1.54;

  // map( value, fromLow, fromHigh, toLow, toHigh);
  int batteryMV = map( batteryADC, 0, 1023, 0, 330);      // Fio = 3.3V.

  batteryMV = batteryMV * voltageDividerScaleFactor;

  // 11111
  // 56789
  // 4.92V
  //
  int digit = batteryMV / 100;
  batteryMV -= digit * 100;
  voltageMsg[0] = '0' + digit;

  voltageMsg[1] = '.';

  digit = batteryMV / 10;
  batteryMV -= digit * 10;
  voltageMsg[2] = '0' + digit;
  
  digit = batteryMV % 10;
  voltageMsg[3] = '0' + digit;

  voltageMsg[4] = 'V';
  voltageMsg[5] = '\0';
  
  displaySerial.write( 139);                // Move to (0,11).
  delay( 5);                                // Just in case this is needed.
  displaySerial.print( voltageMsg);
  delay( 5);                                // Just in case this is needed.

  timeToUpdateVoltageMeasurementAndSampleJoyStick = false;
}

/* From the AT328MEGA Datsheet and here:
 * http://www.arduino.cc/en/Hacking/PinMapping168
 *8 PB0 (PCINT0/CLKO/ICP1)
 *9 PB1 (PCINT1/OC1A)
 *10 PB2 (PCINT2/SS/OC1B)
 *11 PB3 (PCINT3/OC2A/MOSI)
 x12 PB4 (PCINT4/MISO)            // Display.
 x13 PB5 (SCK/PCINT5)             // LED.
 xXTAL1 PB6 (PCINT6/XTAL1/TOSC1)
 xXTAL2 PB7 (PCINT7/XTAL2/TOSC2)
 *A0 PC0 (ADC0/PCINT8)
 xA1 PC1 (ADC1/PCINT9)
 xA2 PC2 (ADC2/PCINT10)
 xA3 PC3 (ADC3/PCINT11)
 xA4 PC4 (ADC4/SDA/PCINT12)
 xA5 PC5 (ADC5/SCL/PCINT13)
 xRESET PC6 (RESET/PCINT14)
 xRX PD0 (RXD/PCINT16)
 xTX PD1 (TXD/PCINT17)
 *2 PD2 (INT0/PCINT18)
 *3 PD3 (PCINT19/OC2B/INT1)
 *4 PD4 (PCINT20/XCK/T0)
 *5 PD5 (PCINT21/OC0B/T1)
 *6 PD6 (PCINT22/OC0A/AIN0)
 *7 PD7 (PCINT23/AIN1)
 */
// So we want:
//  PCINT0-3    (PB0-3)  (8-11)    PCMSK0 = PCINT0 | PCINT1 | PCINT2 | PCINT3;
//  PCINT8      (PC0)    (A0)      PCMSK1 = PCINT8;
//  PCINT18-23  (PD2-7)  (2-7)     PCMSK2 = PCINT18-23;
//
// ADC0 = PortC[0] (aka PC0) -> PCINT8.

// My defining a:
//   ISR( PCINT<n>_vect)
// conflicts with the SoftwareSerial library's wishes, as apparently it wants
// to direct all of ISR( PCINT<1-3>_vect to its own interrupt handler.  Rather
// inconsiderate of it, if you ask me, as shown by this compile error:
//
//   C:\Program Files (x86)\Arduino\hardware\arduino\avr\libraries\
//   SoftwareSerial/SoftwareSerial.cpp:227: multiple definition of 
//     `__vector_4'
//
// where at that point in SoftwareSerial.cpp, it says:
//
//   #if defined(PCINT1_vect)
//     ISR(PCINT1_vect, ISR_ALIASOF(PCINT0_vect));
//   #endif
//
// I found the macro definitions used in SoftwareSerial.cpp here:
//   C:\Arduino\arduino-1.0-windows\arduino-1.0\hardware\arduino\variants\standard\pins_arduino.h
//   C:\Arduino\arduino-1.0-windows\arduino-1.0\hardware\arduino\cores\arduino\Arduino.h
//
// In SoftwareSerial::begin(), it says:
//   "Enable the PCINT for the entire port here, but never disable it
//    (others might also need it, so we disable the interrupt by using
//    the per-pin PCMSK register).
//   *digitalPinToPCICR(_receivePin) |= _BV(digitalPinToPCICRbit(_receivePin));
//
// I want it to call my ISR( PCINT<1-3>) too, along with its, so I'm adding 
// a function-pointer arg to SoftwareSerial(), which it will call if the
// PinChange that triggered the ISR() call is not one that it's waiting for.
//
// ISR( PCINT1_vect)      // Have to share this with SoftwareSerial Library.
//
volatile bool timeToUpdateDipSwitchStates = false;

inline void dipSwitchesPCINTisr()
{
  timeToUpdateDipSwitchStates = true;
}

void updateDipSwitchStates()
{
  bool aDipSwitchChanged = 
    callThisFunctionForEveryDipSwitch( readDipSwitchStateAndStoreIt);

  if (aDipSwitchChanged)
  {
    Serial.print( "Switches changed. (");
    Serial.print( dipSwitchStates);
    Serial.println( ")");
  
    displayOn2ndLine( dipSwitchStates);
    
    operateDebouncedButton( SET_DIR_BUTTON, LED);
  }

  timeToUpdateDipSwitchStates = false;
}

// Our SET_DIR_BUTTON has a PullUp, so it's LOW only when it's currently 
// being pushed.
//
// We initialize ledState to HIGH.
//
bool curButtonState       = ledState;        

// We want to block Button processing until right after it's been released.
//
bool prevButtonState      = curButtonState;  

// Each time after we push then release the Button, the LED should toggle.
//
// Check the values of the DIPSwitches, and call the functions 
// corresponding to the Switches that are HIGH.
//
// The higher-order bits (i.e., D6-D10) control independent invocation of 
// particular individual functions, so multiple functions in this category
// can be invoked each time the SET_DIR_BUTTON is pushed, specified by 
// which of these Switches are HIGH.
//
// The lower-order bits (D2-D5) of the Switches form a bit-field.  Only 
// one of the resulting 16 possible functions is called when the SET_DIR_BUTTON
// is pushed.
//
void operateDebouncedButton( int buttonPinNum, int ledPinNum)
{
  curButtonState = getDipSwitchStateFromPinNum( buttonPinNum);
  
  // We only want to handle the ButtonWasReleased situation.
  // The if below will catch the Button being LOW, then being HIGH. 
  // 
  if (curButtonState == HIGH && prevButtonState == LOW)
  {
    delay( 1);                      // Crude form of button debouncing.
    
    ledState = !ledState;
    digitalWrite( ledPinNum, ledState);
    
    // 17 = backlight on, 18 = backlight off.
    displaySerial.write( 18 - ledState);
    delay( 5);                                // Just in case this is needed.

    if (blockedAwaitingReply)          // Give ourselves an out for when 
    {                                  // we're tired of waiting.
      displayOn2ndLine( "<JoyStick ready>");
      clearJoyStickCmdDisplay();
      blockedAwaitingReply = false;        
      return;
    }
  }
  prevButtonState = curButtonState;
}

char receivedMsg[100];
int  receivedMsgNdx = 0;

void loop()      
{
#if CALIBRATING == 'c' || CALIBRATING == 'p'
  calibrate_R_Theta();
  return;
#endif
  
  if (timeToUpdateVoltageMeasurementAndSampleJoyStick)
    updateVoltageMeasurementAndSampleJoyStick();
    
  if (timeToUpdateDipSwitchStates)
    updateDipSwitchStates();

  bool firstChar = true;
  
  while (Serial.available())
  {
    char c = Serial.read();
    if (c != '\r' && c != '\n')
    {
      if (firstChar)
      {
        firstChar = false;
        receivedMsgNdx = 0;
      }
      receivedMsg[receivedMsgNdx++] = c;
    }
  }

  if (!firstChar)                           // We received some reply.
  {
    clearJoyStickCmdDisplay();

    receivedMsg[receivedMsgNdx] = '\0';
    displayOn2ndLine( receivedMsg);

    blockedAwaitingReply = false; // Unblock us from sending another command.
  }
}

bool readDipSwitchStateAndStoreIt( int pinNum, bool bValue, char *cValuePtr)
{
  // These pins all have Pull-Up Resistors, so let's display a "1" when the
  // pin's DIP Switch is closed.
  // 
  char newCValue = '0' + 1 - digitalRead( pinNum);
  
  bool changed = (*cValuePtr != newCValue);
  
  *cValuePtr = newCValue;
  
  return changed;           // We want to know if any Switches changed state.
}


// Make our print format more structured, to simplify deciding if it's been
// received intact by the Robot.
//
// Total# of bytes sent (up to but not including the "\n") == 8, i.e., ...
//   12345678
//   !6 11 17
//
void displayAndSendJoyStickCmd()
{
  char joystickMsg[10];
  
  joystickMsg[0] = '!';                  // #1.

  joystickMsg[1] = '0' + rVal;           // For now, this is always one digit.  #2.
  joystickMsg[2] = ' ';                  // #3.

  int downshiftedThetaVal = thetaVal - numThetaSectors;
  int tensDigit = downshiftedThetaVal / 10;
  int onesDigit = downshiftedThetaVal % 10;

  joystickMsg[3] = '0' + tensDigit;      // #4.
  joystickMsg[4] = '0' + onesDigit;      // #5.
  joystickMsg[5] = ' ';                  // #6.

  int checksum = rVal + downshiftedThetaVal;
  tensDigit = checksum / 10;
  onesDigit = checksum % 10;

  joystickMsg[6] = '0' + tensDigit;      // #7.
  joystickMsg[7] = '0' + onesDigit;      // #8.
  joystickMsg[8] = '\0';

  displaySerial.write( 128);                // Move to (0,0).
  delay( 5);                                // Just in case this is needed.

  displaySerial.print( joystickMsg);
  delay( 10);                               // Just in case this is needed.

  Serial.println( joystickMsg); // Send our JoyStick command to the Robot.
}

void clearJoyStickCmdDisplay()
{
  displaySerial.write( 128);                // Move to (0,0).
  delay( 5);                                // Just in case this is needed.
  displaySerial.print( "        ");
  delay( 10);                               // Just in case this is needed.
}

void displayCharOn2ndLine( char c, int pos)
{
  displaySerial.write( 148 + pos);          // Move to (1, pos).
  delay( 5);                                // Just in case this is needed.
  displaySerial.write( c);
  delay( 5);                                // Just in case this is needed.
}

// Add const to arg to suppress warning when passing in literal string.
//
void displayOn2ndLine( const char* msg) 
{
  const int displayCharsWidth = 16;
  //                     0123456789012345
  const char spaces[] = "                ";

  displaySerial.write( 148);                // Move to (1,0).
  delay( 5);                                // Just in case this is needed.
  displaySerial.print( msg);
  delay( 15);                               // Just in case this is needed.

  int startSpacesNdx = strlen( msg);

  if (startSpacesNdx < strlen( spaces))
    displaySerial.print( &(spaces[startSpacesNdx]));
    // Already delay()'ed enough just above.
}

void plataDuckBulletin()
{
  displaySerial.write( 18);                 // Turn backlight off.
  delay( 5);                                // Just in case this is needed.
  displaySerial.write( 12);                 // Clear.
  delay( 5);                                // Just in case this is needed.

  //  while( digitalRead( SETDIRBUTTON))    // Wait patiently for the 
  //  {};                                   // button to be pushed.

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
  delay( 5);                                // Just in case this is needed.
  displaySerial.write( 17);                 // Turn backlight on.
  delay( 5);                                // Required delay.
  displaySerial.write( "Song of the Wind");
  delay( 15);                               // Just in case this is needed.

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
