// This sketch shows the basic operation of the Thumb Joystick (COM-09032) and breakout board (BOB-09110).
// The joystick outputs two analog voltages (VERT and HORIZ), and one digital signal (SEL) for the pushbutton.

#define BATTERY_BLUE_WIRE  A5    // Analog.
#define HORIZ_ORANGE_WIRE  A7    // Analog.
#define VERT_GREEN_WIRE    A6    // Analog.

#define SET_DIR_BUTTON     11    // Digital.

#define DISPLAY_GREEN_WIRE 12    // Digital.
#define LED                13

const int firstSwitchPinNum = 2;
const int lastSwitchPinNum  = SET_DIR_BUTTON;
const int numSwitchPins     = lastSwitchPinNum - firstSwitchPinNum + 1;

const int adcSwitches[] = {A0};  // = 14;
const int numADCswitches = sizeof( adcSwitches) / sizeof( int);
const int lastADCswitchPinNum = adcSwitches[numADCswitches - 1];

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

#include <SoftwareSerial.h>
SoftwareSerial displaySerial( 
  255, DISPLAY_GREEN_WIRE, false, &joystickPCINTisr);

// Our PushButton's Input Pin has a Pullup Resistor, so the LED should be 
// HIGH by default.
//
bool ledState             = HIGH;  

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
  
  digitalWrite( LED, ledState); // Set up the Output LED for initial state.

//  plataDuckBulletin();
//  playSongOfTheWind_Suzuki3();

  setRadiusRangeValues();

  displayOn2ndLine( "<JoyStick ready>");
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
//    bool pinState = bitRead( 
//      *portInputRegister( digitalPinToPort( pin)),
//      digitalPinToPCMSKbit( pin));
//  or:
//    bool pinState = 
//      *portInputRegister( digitalPinToPort( pin)) & 
//      digitalPinToBitMask( pin);

/*
  volatile bool state1 = false;
  volatile bool state2 = false;
  volatile bool newDataForPrinting = false;
  int interruptingPin = A0;
*/
inline void joystickPCINTisr()
{  
/*
  state1 = !bitRead( 
    *portInputRegister( digitalPinToPort( interruptingPin)),
    digitalPinToPCMSKbit( interruptingPin));

//  Serial.print( "state1 = ");
//  Serial.println( state1);
    
  state2 = 
    !(*portInputRegister( digitalPinToPort( interruptingPin)) & 
    digitalPinToBitMask( interruptingPin));

//  Serial.print( "state2 = ");
//  Serial.println( state2);
*/
//  newDataForPrinting = true;
  
  updateDipSwitchStates();
}

char receivedMsg[100];
int  receivedMsgNdx = 0;

int dssCtr = 0;
int batteryMeasureCtr = 0;
bool blockedAwaitingReply = false;

void loop()      
{  
  const int loopsToSkipChecking = 150;
  
  // Let's not spend _all_ of our time doing this.  This is a good value 
  // to catch very brief button-presses.
  //
//  if (dssCtr++ % loopsToSkipChecking == 0)     
//    updateDipSwitchStates();           // Now called in joystickPCINTisr();

/*
  if (newDataForPrinting)
  {
    Serial.print( "state1 = ");
    Serial.print( state1);
    Serial.print( "  state2 = ");
    Serial.print( state2);
    Serial.println( "");
    
    newDataForPrinting = false;    
  }
  
  return;
*/
  if (batteryMeasureCtr++ % 10000 == 0)              // Ditto.
    displayVoltageMeasurement();

  if (dssCtr % loopsToSkipChecking == 0)     
    operateDebouncedButton( SET_DIR_BUTTON, LED);    // Ditto.

  if (get_R_Theta())
  {
    if (!blockedAwaitingReply)
    {
      blockedAwaitingReply = true;
      displayOn2ndLine( "<Btn> = end wait");

      displayAndSendJoyStickCmd(); // Send the JoyStick command to the Robot.
      delay( 250);                     // Give the Robot time to respond.
    }
  }

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

void updateDipSwitchStates()
{
  bool aDipSwitchChanged = 
    callThisFunctionForEveryDipSwitch( readDipSwitchStateAndStoreIt);

  if (aDipSwitchChanged)
  {
    Serial.print( "Switches changed. (");
    Serial.print( dipSwitchStates);
    Serial.println( ")");
  }
  
  displayOn2ndLine( dipSwitchStates); 
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
// one of the 16 possible functions is called when the SET_DIR_BUTTON is 
// pushed.
//
void operateDebouncedButton( int buttonPinNum, int ledPinNum)
{
  curButtonState = digitalRead( buttonPinNum);  // Usually == SET_DIR_BUTTON.
  
  // We only want to handle the ButtonWasReleased situation.
  //
  // The if below will catch the Button being LOW, then being HIGH. 
  // 
  if (curButtonState == HIGH && prevButtonState == LOW)
  {
    delay( 1);                      // Crude form of button debouncing.
    
    if (ledState == HIGH)
    {
      digitalWrite( ledPinNum, LOW);
      ledState = LOW;
      displaySerial.write( 18);                 // Turn backlight off.
    } 
    else 
    {
      digitalWrite( ledPinNum, HIGH);
      ledState = HIGH;
      displaySerial.write( 17);                 // Turn backlight on.
    }
    
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

  displaySerial.print( joystickMsg);
  Serial.println( joystickMsg); // Send our JoyStick command to the Robot.
}

void clearJoyStickCmdDisplay()
{
  displaySerial.write( 128);                // Move to (0,0).
  displaySerial.print( "        ");
}

void displayVoltageMeasurement()
{
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
  displaySerial.print( voltageMsg);
}

void displayCharOn2ndLine( char c, int pos)
{
  displaySerial.write( 148 + pos);            // Move to (1, pos).
  displaySerial.write( c);
}

// Add const to arg to suppress warning when passing in literal string.
//
void displayOn2ndLine( const char* msg) 
{
  const int displayCharsWidth = 16;
  //                     0123456789012345
  const char spaces[] = "                ";

  displaySerial.write( 148);                // Move to (1,0).
  displaySerial.print( msg);

  int startSpacesNdx = strlen( msg);

  if (startSpacesNdx < strlen( spaces))
    displaySerial.print( &(spaces[startSpacesNdx]));
    
  delay( 15);
}

void plataDuckBulletin()
{
  displaySerial.write( 18);                 // Turn backlight off.
  displaySerial.write( 12);                 // Clear.

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
