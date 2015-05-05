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
// Power Codes:
//
// Long  pulse range = 75->86 (* 10uS.).     Range = 12, Avg. = 80  Fuzz = 10%.
// Short pulse range = 25(36)->44 (* 10uS.). Range = 20 Avg. = 34  Fuzz = 30%.
// Off         range = 14->28.   (this is the gap between Long's and Short's.)
//
// Use pulseThreshold below to determine if we have a Long or Short pulse.
//
const byte longTime  = 80u;
const byte shortTime = 34u;
const byte offTime   = 35u;
const byte pulseDurationThreshold = (44 + 75) / 2;
const byte sizeofInt = sizeof( int);

// For brevity, as much as possible, I'll express each button's 
// pattern as pairs of:
// <so many> long pulses, <so many> short pulses.
//
int toyPowerButtonPattern[] = 
{
  // We want to start at index 1, to match the output of the
  // rawirdecode program. so let's put this spot to good use,
  // to store our command name
  (int)"POWER",
//
// For 1-15:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,5,
//
// For 16: Gap of 1176 ms.
-1176, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-31:
// l s l s 
// 23242627
   1,2,1,5,
//
// For 32: Gap of 834 ms.
-834, -1,  // mS.
//
// For 33-42:
// l s l s 
// 33343538
   1,1,3,5,
//
// For 43-51:
// l s 
// 4346
   3,6,
//
// For 52-60:
// l s l s l s 
// 525354555860
   1,1,1,3,2,1,
//
// For 61-65:
// l s l 
// 616263
   1,1,3,-1,
//
// For 66: Gap of 1291 ms.
-1291, -1,  // mS.
//
// For 67-75:
// s l 
// 6775      // -2 = Past this gap we're doing Shorts first.
-2,8,1,
//
// For 76-77:
// s 
// 76
   2,-1,
//
// For 78: Gap of 3222 ms.
-3222, -1,  // mS.
//
// For 79-87:
// s l 
// 7987      // -2 = Past this gap we're doing Shorts first.
-2,8,1,
//
// For 88-89:
// s 
// 88
   2,-1,
//
// For 90: Gap of 3216 ms.
-3216, -1,  // mS.
//
// For 91-108:
// l s l  s  
// 9195100103
   4,5,3, 6, 
//
// For 109-115:
// l  s  
// 109110
   1, 6, 
//
// For 116-123:
// l  s  l  
// 116117118
   1, 1, 6, -1,
//
// For 124: Gap of 1152 ms.
-1152, -1,  // mS.
//
// For 125-131:
// l  s  
// 125126
   1, 6, 
//
// For 132-138:
// l  s  l  
// 132133134
   1, 1, 5, -1,
}; // End of toyPowerButtonPattern[].

int toyMuteButtonPattern[] =
{ 
  // We want to start at index 1, to match the output of the
  // rawirdecode program. so let's put this spot to good use,
  // to store our command name
  (int)"MUTE",
//
// My toy IR Remote's "Mute" command's pattern is:
//
// For 1-12:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,2,
//
// For 13-15:
// l s 
// 1315
   2,1,
//
// For 16: Gap of 1174 ms.
-1174, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-31:
// l s l s l s 
// 232426272931
   1,2,1,2,2,1,
//
// For 32: Gap of 831 ms.
-831, -1,  // mS.
//
// For 33-42:
// l s l s 
// 33343538
   1,1,3,5,
//
// For 43-51:
// l s l s 
// 43465051
   3,4,1,1,
//
// For 52-60:
// l s l s l s 
// 525354555960
   1,1,1,4,1,1,
//
// For 61-65:
// l s l 
// 616263
   1,1,3,-1,
//
// For 66: Gap of 1293 ms.
-1293, -1,  // mS.
//
// For 67-75:
// s l 
// 6775
-2,8,1,
//
// For 76-77:
// s l 
// 7677
   1,1,
//
// For 78: Gap of 3178 ms.
-3178, -1,  // mS.
//
// For 79-87:
// s l 
// 7987
-2,8,1,
//
// For 88-89:
// s l 
// 8889
   1,1,
//
// For 90: Gap of 3175 ms.
-3175, -1,  // mS.
//
// For 91-107:
// l s l  s  
// 9195100103
   4,5,3, 5, 
//
// For 108-119:
// l  s  
// 108112
   4, 8, 
//
// For 120-123:
// l  
// 120
   4, -1,
//
// For 124: Gap of 1152 ms.
-1152, -1,  // mS.
//
// For 125-131:
// l  s  
// 125126
   1, 6, 
//
// For 132-138:
// l  s  l  
// 132133134
   1, 1, 5, -1,
}; // End of toyMuteButtonPattern[].
  
int toyChUpButtonPattern[] = 
{
(int)"CH-UP",
//
// For 1-12:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,2,
//
// For 13-15:
// l s 
// 1314
   1,2,
//
// For 16: Gap of 1205 ms.
-1205, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-30:
// l s l s l s 
// 232426272930
   1,2,1,2,1,1,
//
// For 31-51:
// l s l s 
// 31373844
   6,1,6,8,
//
// For 52-54:
// l 
// 52
   3,-1,
//
// For 55: Gap of 1181 ms.
-1181, -1,  // mS.
//
// For 56: Gap of 227 ms.
-227, -1,  // mS.
//
// For 57-61:
// s l s l 
// 57585961
-2,1,1,2,1,
//
// For 62-70:
// s l s l 
// 62656670
   3,1,4,1,
//
// For 71-82:
// s l s l s l 
// 717274757879
   1,2,1,3,1,4,
//
// For 83-91:
// s l s l s l 
// 838485868891
   1,1,1,2,3,1,
//
// For 92-98:
// s l s l 
// 92939496
   1,1,2,3,
//
// For 99: Gap of 1184 ms.
-1184, -1,  // mS.
//
// For 100-111:
// s  
// 100
-2,12, -1,
//
// For 112: Gap of 675 ms.
-675, -1,  // mS.
//
// For 113-124:
// s  
// 113
-2,12, -1,
}; // End of toyChUpButtonPattern[].

int toyChDnButtonPattern[] = 
{
(int)"CH-DN",
//
// For 1-11:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,1,
//
// For 12-15:
// l s 
// 1213
   1,3,
//
// For 16: Gap of 1204 ms.
-1204, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-36:
// l s l s l s l s 
// 2324262728293036
   1,2,1,1,1,1,6,1,
//
// For 37-40:
// l s 
// 3740
   3,1,
//
// For 41-50:
// l s l s 
// 41434849
   2,5,1,2,
//
// For 51-53:
// l 
// 51
   3,-1,
//
// For 54: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 55-63:
// l s l s l s 
// 555657586061
   1,1,1,2,1,3,
//
// For 64-70:
// l s l s 
// 64656970
   1,4,1,1,
//
// For 71-84:
// l s l s l s 
// 717374777884
   2,1,3,1,6,1,
//
// For 85-91:
// l s 
// 8587
   2,5,
//
// For 92-97:
// l s l 
// 929395
   1,2,3,-1,
//
// For 98: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 99-110:
// s 
// 99
-2,12,-1,
//
// For 111: Gap of 674 ms.
-674, -1,  // mS.
//
// For 112-123:
// s  
// 112
-2,12, -1,
}; // End of toyChDnButtonPattern[].

int toyVolDnButtonPattern[] = 
{
(int)"VOL-DN",
//
// For 1-11:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,1,
//
// For 12-15:
// l s l s 
// 12131415
   1,1,1,1,
//
// For 16: Gap of 1201 ms.
-1201, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-36:
// l s l s l s l s 
// 2324262728293036
   1,2,1,1,1,1,6,1,
//
// For 37-45:
// l s l s 
// 37383943
   1,1,4,3,
//
// For 46-50:
// l s 
// 4647
   1,4,
//
// For 51-53:
// l 
// 51
   3,-1,
//
// For 54: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 55-63:
// l s l s l s 
// 555657586061
   1,1,1,2,1,3,
//
// For 64-70:
// l s l s 
// 64656970
   1,4,1,1,
//
// For 71-82:
// l s l s l s 
// 717374777882
   2,1,3,1,4,1,
//
// For 83-94:
// l s l s 
// 83879091
   4,3,1,4,
//
// For 95-97:
// l 
// 95
   3,-1,
//
// For 98: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 99-110:
// s 
// 99
-2,12,-1,
//
// For 111: Gap of 674 ms.
-674, -1,  // mS.
//
// For 112-123:
// s  
// 112
-2,12, -1,
}; // End of toyVolDnButtonPattern[].

int toyVolUpButtonPattern[] = 
{
(int)"VOL-UP",
//
// For 1-13:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,3,
//
// For 14-15:
// l s 
// 1415
   1,1,
//
// For 16: Gap of 1204 ms.
-1204, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-35:
// l s l s l s 
// 232427282935
   1,3,1,1,6,1,
//
// For 36-44:
// l s l s l s 
// 363738394042
   1,1,1,1,2,3,
//
// For 45-52:
// l s l s l 
// 4546474850
   1,1,1,2,3,-1,
//
// For 53: Gap of 1184 ms.
-1184, -1,  // mS.
//
// For 54-62:
// l s l s l s 
// 545556575960
   1,1,1,2,1,3,
//
// For 63-72:
// l s l s l s 
// 636468697072
   1,4,1,1,2,1,
//
// For 73-93:
// l s l s 
// 73767786
   3,1,9,8,
//
// For 94-96:
// l 
// 94
   3,-1,
//
// For 97: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 98-109:
// s 
// 98
-2,12,-1,
//
// For 110: Gap of 675 ms.
-675, -1,  // mS.
//
// For 111-122:
// s  
// 111
-2,12, -1,
}; // End of toyVolUpButtonPattern[].

int toyAV_TVButtonPattern[] = 
{
(int)"AV-TV",
//
// For 1-11:
// l s l s l s 
// 1 4 7 8 1011
   3,3,1,2,1,1,
//
// For 12-15:
// l s 
// 1215
   3,1,
//
// For 16: Gap of 1204 ms.
-1204, -1,  // mS.
//
// For 17-22:
// l s 
// 1720
   3,3,
//
// For 23-36:
// l s l s l s l s 
// 2324262728293036
   1,2,1,1,1,1,6,1,
//
// For 37-49:
// l s 
// 3742
   5,8,
//
// For 50-53:
// l 
// 50
   4,-1,
//
// For 54: Gap of 1183 ms.
-1183, -1,  // mS.
//
// For 55: Gap of 226 ms.
-226, -1,  // mS.
//
// For 56-60:
// s l s l 
// 56575860
-2,1,1,2,1,
//
// For 61-72:
// s l s l s l 
// 616465697071
   3,1,4,1,1,2,
//
// For 73-85:
// s l s l 
// 73747778
   1,3,1,8,
//
// For 86-97:
// s l 
// 8694
   8,4,
//
// For 98: Gap of 1182 ms.
-1182, -1,  // mS.
//
// For 99-110:
// s 
// 99
-2,12,-1,
//
// For 111: Gap of 674 ms.
-674, -1,  // mS.
//
// For 112-123:
// s  
// 112
-2,12, -1,
}; // End of toyAV_TVButtonPattern[].

int* irPatterns[] =
{
  toyPowerButtonPattern,
  toyMuteButtonPattern,
  toyChUpButtonPattern,
  toyChDnButtonPattern,
  toyVolDnButtonPattern,
  toyVolUpButtonPattern,
  toyAV_TVButtonPattern
};

int irPatternSizes[] =
{
  sizeof( toyPowerButtonPattern) / sizeofInt,
  sizeof( toyMuteButtonPattern) / sizeofInt,
  sizeof( toyChUpButtonPattern) / sizeofInt,
  sizeof( toyChDnButtonPattern) / sizeofInt,
  sizeof( toyVolDnButtonPattern) / sizeofInt,
  sizeof( toyVolUpButtonPattern) / sizeofInt,
  sizeof( toyAV_TVButtonPattern) / sizeofInt
};
const byte numIRpatterns = sizeof( irPatternSizes) / sizeofInt;

