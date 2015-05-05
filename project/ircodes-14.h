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

/******************* our codes ************/
//
// For my "retired" toy IR Remote from SparkFun:
//
// This version is for the 119-pulse codes, starting with 1,6,1,1,6,1, 
// not the original hundreds and hundreds of pulses codes.
//
// Power Codes:
//
// Long  pulse range = 75->86 (* 10uS.).     Range = 12, Avg. = 80  Fuzz = 10%.
// Short pulse range = 25(36)->44 (* 10uS.). Range = 20 Avg. = 34  Fuzz = 30%.
// Off         range = 14->28.   (this is the gap between Long's and Short's.)
//
// Use pulseThreshold below to determine if we have a Long or Short pulse.
//
const byte longTime  = 80u;
const byte shortTime = 45u;
const byte offTime   = 35u;
const byte pulseDurationThreshold = (45u + 75u) / 2;
const byte sizeofInt = sizeof( int);

// For brevity, as much as possible, I'll express each button's 
// pattern as pairs of:
// <so many> Long pulses, <so many> Short pulses.
// -1's are placeholders, for when there is no Short pulse
// preceeding a Gap, etc.
// -2's mean, Short pulses come first after this Gap.
//
int toyPowerButtonPattern[] = 
{
  // We want to start at index 1, to match the output of the
  // rawirdecode program. so let's put this spot to good use,
  // to store our command name
  (int)"POWER",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-21:
// l s l s 
// 17181920
   1,1,1,2,
//
// For 22-30:
// l s l s l s 
// 222326272830
   1,3,1,1,2,1,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 35: Gap of 227 ms.
// -227, -1,  // mS.
//
// For 36-47:
// s l s l 
// 36374045
-2,1,3,5,3,
//
// For 48-54:
// s l 
// 4854
   6,1,
//
// For 55-61:
// s l s l 
// 55565760
   1,1,3,2,
//
// For 62-67:
// s l s l 
// 62636465
   1,1,1,3,
//
// For 68: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 69: Gap of 227 ms.
// -227, -1,  // mS.
//
// For 70-73:
// l s 
// 7073
   3,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-103:
// l s l s  
// 929698101
   4,2,3,3, 
//
// For 104-111:
// l  s  l  
// 104106109
   2, 3, 3, -1,
}; // End of toyPowerButtonPattern[].
/*
int toyChUpButtonPattern[] = 
{
(int)"CH-UP",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-30:
// l s 
// 1723
   6,8,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 35: Gap of 228 ms.
-228, -1,  // mS.
//
// For 36-47:
// s l s l 
// 36374045
-2,1,3,5,3,
//
// For 48-52:
// s l 
// 4852
   4,1,
//
// For 53-63:
// s l s l 
// 53565761
   3,1,4,3,
//
// For 64-67:
// s l 
// 6465
   1,3,
//
// For 68: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 69-73:
// l s 
// 6973
   4,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-103:
// l s l s l s  
// 9296979899101
   4,1,1,1,2,3, 
//
// For 104-111:
// l  s  l  s  l  
// 104105106107109
   1, 1, 1, 2, 3, -1,
}; // End of toyChUpButtonPattern[].

int toyChDnButtonPattern[] = 
{
(int)"CH-DN",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-20:
// l s 
// 1720
   3,1,
//
// For 21-30:
// l s l s 
// 21232829
   2,5,1,2,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 35-44:
// l s l s 
// 35363740
   1,1,3,5,
//
// For 45-55:
// l s 
// 4548
   3,8,
//
// For 56-64:
// l s l s 
// 56576064
   1,3,4,1,
//
// For 65-67:
// l 
// 65
   3,-1,
//
// For 68: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 69-73:
// l s 
// 6973
   4,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-105:
// l s l s  
// 929899101
   6,1,2,5, 
//
// For 106-111:
// l  s  l  
// 106107109
   1, 2, 3, -1,
}; // End of toyChDnButtonPattern[].

int toyMuteButtonPattern[] =
{ 
  // We want to start at index 1, to match the output of the
  // rawirdecode program. so let's put this spot to good use,
  // to store our command name
  (int)"MUTE",
//
// My toy IR Remote's "Mute" command's pattern is:
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-21:
// l s 
// 1718
   1,4,
//
// For 22-30:
// l s l s 
// 22232630
   1,3,4,1,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1185 ms.
-1185, -1,  // mS.
//
// For 35-44:
// l s l s 
// 35363740
   1,1,3,5,
//
// For 45-51:
// l s 
// 4548
   3,4,
//
// For 52-60:
// l s l s l s 
// 525354555657
   1,1,1,1,1,4,
//
// For 61-67:
// l s l s l 
// 6162636465
   1,1,1,1,3,-1,
//
// For 68: Gap of 1185 ms.
-1185, -1,  // mS.
//
// For 69-73:
// l s 
// 6973
   4,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-104:
// l s l s l  s  
// 92979899100101
   5,1,1,1,1, 4, 
//
// For 105-111:
// l  s  l  s  l  
// 105106107108109
   1, 1, 1, 1, 3, -1,
}; // End of toyMuteButtonPattern[].

int toyVolDnButtonPattern[] = 
{
(int)"VOL-DN",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-25:
// l s l s 
// 17181923
   1,1,4,3,
//
// For 26-30:
// l s 
// 2627
   1,4,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1185 ms.
-1185, -1,  // mS.
//
// For 35: Gap of 226 ms.
-226, -1,  // mS.
//
// For 36-47:
// s l s l 
// 36374045
-2,1,3,5,3,
//
// For 48-53:
// s l 
// 4852
   4,2,
//
// For 54-63:
// s l s l 
// 54565762
   2,1,5,2,
//
// For 64-67:
// s l 
// 6465
   1,3,
//
// For 68: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 69-73:
// l s 
// 6973
   4,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-103:
// l s l s  
// 929697101
   4,1,4,3, 
//
// For 104-111:
// l  s  l  
// 104105109
   1, 4, 3, -1,
}; // End of toyVolDnButtonPattern[].

int toyVolUpButtonPattern[] = 
{
(int)"VOL-UP",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-20:
// l s l s 
// 17181920
   1,1,1,1,
//
// For 21-30:
// l s l s l s 
// 212326272829
   2,3,1,1,1,2,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 35-44:
// l s l s 
// 35363740
   1,1,3,5,
//
// For 45-52:
// l s 
// 4548
   3,5,
//
// For 53-61:
// l s l s l s 
// 535456576061
   1,2,1,3,1,1,
//
// For 62-67:
// l s l 
// 626465
   2,1,3,-1,
//
// For 68: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 69: Gap of 226 ms.
-226, -1,  // mS.
//
// For 70-73:
// l s 
// 7073
   3,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-108:
// l s  
// 92101
   9,8, 
//
// For 109-111:
// l  
// 109
   3, -1,
}; // End of toyVolUpButtonPattern[].

int toyAV_TVButtonPattern[] = 
{
(int)"AV-TV",
//
// For 1-16:
// l s l s l s 
// 1 2 8 9 1016
   1,6,1,1,6,1,
//
// For 17-21:
// l s l s 
// 17182021
   1,2,1,1,
//
// For 22-30:
// l s l s l s 
// 222326282930
   1,3,2,1,1,1,
//
// For 31-33:
// l 
// 31
   3,-1,
//
// For 34: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 35-44:
// l s l s 
// 35363740
   1,1,3,5,
//
// For 45-52:
// l s 
// 4548
   3,5,
//
// For 53-62:
// l s l s l s 
// 535556576061
   2,1,1,3,1,2,
//
// For 63-67:
// l s l 
// 636465
   1,1,3,-1,
//
// For 68: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 69: Gap of 226 ms.
-226, -1,  // mS.
//
// For 70-73:
// l s 
// 7073
   3,1,
//
// For 74-85:
// l s l s 
// 74757879
   1,3,1,7,
//
// For 86-91:
// l s l s 
// 86878891
   1,1,3,1,
//
// For 92-104:
// l s l s  
// 929798100
   5,1,2,5, 
//
// For 105-111:
// l  s  l  
// 105106108
   1, 2, 4, -1,
}; // End of toyAV_TVButtonPattern[].
*/

int tvPowerButtonPattern[] = 
{
(int)"tvPower",
//
// For 1-18:
// l s l s 
// 1 5 1013
   4,5,3,6,
//
// For 19-25:
// l s 
// 1920
   1,6,
//
// For 26-33:
// l s l 
// 262728
   1,1,6,-1,
//
// For 34: Gap of 2270 ms.
-2270, -1,  // mS.
//
// For 35-43:
// l s 
// 3539
   4,5,
//
// For 44-52:
// l s 
// 4447
   3,6,
//
// For 53-61:
// l s l s 
// 53546061
   1,6,1,1,
//
// For 62-67:
// l 
// 62
   6,-1,
}; // End of tvPowerButtonPattern[].

int tvMuteButtonPattern[] = 
{
(int)"tvMute",
//
// For 1-17:
// l s l s 
// 1 5 1013
   4,5,3,5,
//
// For 18-29:
// l s 
// 1822
   4,8,
//
// For 30-33:
// l 
// 30
   4,-1,
//
// For 34: Gap of 2270 ms.
-2270, -1,  // mS.
//
// For 35-43:
// l s 
// 3539
   4,5,
//
// For 44-51:
// l s 
// 4447
   3,5,
//
// For 52-63:
// l s 
// 5256
   4,8,
//
// For 64-67:
// l 
// 64
   4,-1,
}; // End of tvMuteButtonPattern[].

int* irPatterns[] =
{
  toyPowerButtonPattern, 
  tvPowerButtonPattern,
  tvMuteButtonPattern
/*
  toyChUpButtonPattern,
  toyChDnButtonPattern,
  toyMuteButtonPattern,
  toyVolDnButtonPattern,
  toyVolUpButtonPattern,
  toyAV_TVButtonPattern */
};

int irPatternSizes[] =
{
  sizeof( toyPowerButtonPattern) / sizeofInt, 
  sizeof( tvPowerButtonPattern)  / sizeofInt,
  sizeof( tvMuteButtonPattern)   / sizeofInt,
  /*
  sizeof( toyChUpButtonPattern) / sizeofInt, 
  sizeof( toyChDnButtonPattern) / sizeofInt,
  sizeof( toyMuteButtonPattern) / sizeofInt,
  sizeof( toyVolDnButtonPattern) / sizeofInt,
  sizeof( toyVolUpButtonPattern) / sizeofInt,
  sizeof( toyAV_TVButtonPattern) / sizeofInt */
};
const byte numIRpatterns = sizeof( irPatternSizes) / sizeofInt;


