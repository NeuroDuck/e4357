// 
// See the modulated 38kHz Remote Control waveforms here:
// https://learn.adafruit.com/ir-sensor/ir-remote-signals
//
// And the overall IR-Reading-Transmitting Tutorial is here:
// https://learn.adafruit.com/ir-sensor?view=all#using-an-ir-sensor
//
// You can use from about 35 KHz to 41 KHz but the sensitivity will 
// drop off so that it wont detect as well from afar.
//
// Likewise, you can use 850 to 1100 nm LEDs but they wont work as 
// well as 900 to 1000nm so make sure to get matching LEDs! Check 
// the datasheet for your IR LED to verify the wavelength.
// Try to get a 940nm - remember that 940nm is not visible light 
// (its Infra Red)!

// Power Codes:
// Use pulseThreshold below to determine if we have a Long or Short pulse.
//
// We're re-calculating these values just below in IR_Recorder now.
//
const byte laskoLongPulseDuration  = 62;       // 1240uS. = 0.62 * 2mS.
const byte laskoShortPulseDuration = 18;       //  360uS. = 0.18 * 2mS.
const byte longPulseDuration    = 78;
const byte shortPulseDuration   = 28;
byte shortLongPulseThreshold    = 54;
int offPulsesMaxDuration        = 29;
int gapPulsesMaxDuration        = 0;
const byte resolutionInUsec     = 20;

// Lasko Remotes don't send a Preamble, but the Bionaire and TV ones do:
//                                 OFF  ON
//                                Emit  Don't Emit
// We really want 450 here instead vvv of 255, so we'll fake it below.
const byte bionairePreamble[]   = {255, 215};
const byte tvPreamble[]         = {225, 225};
const byte* preambles[]         = {bionairePreamble, tvPreamble};
const byte bionairePreamble0placeholder = 255;
const uint16_t bionairePreamble0 = 450;	// Use this duration when we encounter 255 in array.

enum {bionairePattern = 0, tvPattern = 1, laskoPattern = 2};

// What our timing resolution should be, larger is better
// as its more 'precise' - but too large and you wont get
// accurate timing.
//
const long resolutionInUsecLong = resolutionInUsec;

// The typical Preamble I've seen so far = ~15mS., consisting of an initial
// 9mS. LOW followed by a 4.3mS. HIGH.
// The actual data-bearing pulse trains I'm looking at right now are all 
// 60mS. or less.
//
// So we can say that the longest HIGH or LOW we should ever be waiting for
// is 20mS.

// Cast everything to long to avoid constant int value overflow (at 65536).
//
// Let's make it just a bit shorter than the gaps (6490uS) that the Lasko 
// Remote produces.
//
const long maxPulseDuration = 6800L / resolutionInUsecLong;

byte laskoPowerButtonPattern[] = 
{
  laskoPattern, 12,                  // <= numPulsesInPattern
  0b11011000, 0b00010000
};

byte laskoSpeedButtonPattern[] = 
{
  laskoPattern, 12,                  // <= numPulsesInPattern
  0b11011000, 0b00100000
};

byte laskoTimerButtonPattern[] = 
{
  laskoPattern, 12,                  // <= numPulsesInPattern
  0b11011000, 0b10000000
};

byte bionairePowerButtonPattern[] = 
{
  bionairePattern, 17,         // <= numClumps
  0x11,0x91,0x11,0x11,0x11,0x21,0x33,0x31,0x20,
};

byte bionaireVarSpeedButtonPattern[] = 
{
  bionairePattern, 17,         // <= numClumps
  0x11,0x91,0x11,0x11,0x14,0x32,0x11,0x21,0x20,
};

byte bionaireTimerButtonPattern[] = 
{
  bionairePattern, 17,         // <= numClumps
  0x11,0x91,0x11,0x11,0x53,0x11,0x11,0x21,0x20,
};

byte bionaireOscillateButtonPattern[] = 
{
  bionairePattern, 17,         // <= numClumps
  0x11,0x91,0x11,0x11,0x41,0x21,0x11,0x13,0x30,
};

byte dvdPowerButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x1A,0x11,0x11,0x11,0x11,
};

byte tvChDownButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0xA4,0x11,0x12,0x11,0x10,
};

byte tvChUpButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x73,0x42,0x12,0x11,0x10,
};

byte tvDigit0ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x12,0x72,0x11,0x11,
};

byte tvDigit1ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x13,0x72,0x11,0x11,0x11,
};

byte tvDigit2ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x19,0x11,0x11,0x11,
};

byte tvDigit3ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x14,0x61,0x21,0x11,0x11,
};

byte tvDigit4ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x27,0x21,0x11,0x11,
};

byte tvDigit5ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x13,0x16,0x31,0x11,0x11,
};

byte tvDigit6ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x11,0x91,0x11,0x11,
};

byte tvDigit7ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x15,0x51,0x12,0x11,0x11,
};

byte tvDigit8ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x36,0x12,0x11,0x11,
};

byte tvDigit9ButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x13,0x25,0x22,0x11,0x11,
};

byte tvHUDDownButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x31,0x51,0x31,0x11,
};

byte tvHUDEnterButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x14,0x11,0x41,0x41,0x11,
};

byte tvHUDLeftButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x12,0x12,0x16,0x31,0x11,
};

byte tvHUDRightButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x13,0x21,0x42,0x31,0x11,
};

byte tvHUDUpButtonPattern[] = 
{
  tvPattern, 14,               // <= numClumps
  0x6B,0x11,0x15,0x14,0x11,0x31,0x11,
};

byte tvInputSelectButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x69,0x11,0x11,0x11,0x10,
};

byte tvMuteButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x61,0x11,0x91,0x11,0x10,
};

byte tvPowerButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x77,0x21,0x11,0x11,0x10,
};

byte tvVolDownButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x61,0x27,0x21,0x11,0x10,
};

byte tvVolUpButtonPattern[] = 
{
  tvPattern, 15,               // <= numClumps
  0x11,0x16,0x11,0x61,0x19,0x11,0x11,0x10,
};

char* patternNames[] =
{
  "LPOWER",
  "LSPEED",
  "LTIMER",
  "BPOWER",
  "BVARSPEED",
  "BTIMER",
  "BOSCILLATE",
  "DVDPOWER",
  "TVCHDOWN",
  "TVCHUP",
  "TVDIGIT0",
  "TVDIGIT1",
  "TVDIGIT2",
  "TVDIGIT3",
  "TVDIGIT4",
  "TVDIGIT5",
  "TVDIGIT6",
  "TVDIGIT7",
  "TVDIGIT8",
  "TVDIGIT9",
  "TVHUDDOWN",
  "TVHUDENTER",
  "TVHUDLEFT",
  "TVHUDRIGHT",
  "TVHUDUP",
  "TVINPSEL",
  "TVMUTE",
  "TVPOWER",
  "TVVOLDOWN",
  "TVVOLUP"
};

byte* irPatterns[] =
{
  // 0
  laskoPowerButtonPattern, laskoSpeedButtonPattern, laskoTimerButtonPattern,
  // 3
  bionairePowerButtonPattern, bionaireVarSpeedButtonPattern,
  // 5
  bionaireTimerButtonPattern, bionaireOscillateButtonPattern,
  // 7
  dvdPowerButtonPattern, 
  // 8
  tvChDownButtonPattern, tvChUpButtonPattern,
  // 10
  tvDigit0ButtonPattern, tvDigit1ButtonPattern, tvDigit2ButtonPattern,
  // 13
  tvDigit3ButtonPattern, tvDigit4ButtonPattern, tvDigit5ButtonPattern,
  // 16
  tvDigit6ButtonPattern, tvDigit7ButtonPattern, tvDigit8ButtonPattern,
  // 19
  tvDigit9ButtonPattern,
  // 20
  tvHUDDownButtonPattern, tvHUDEnterButtonPattern, tvHUDLeftButtonPattern,
  // 23
  tvHUDRightButtonPattern, tvHUDUpButtonPattern, 
  // 25
  tvInputSelectButtonPattern, tvMuteButtonPattern, tvPowerButtonPattern,
  // 28
  tvVolDownButtonPattern, tvVolUpButtonPattern
};
const byte sizeOfBytePtr = sizeof( byte*);
const byte numIRpatterns = sizeof( irPatterns) / sizeOfBytePtr;
