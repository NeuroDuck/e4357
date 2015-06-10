#include <Wire.h>
#include <LSM303D.h>
#include <LSM303DLM.h>

LSM303D compassD;
LSM303D::vector<int16_t> 
  runningD_min = { 32767,  32767,  32767}, 
  runningD_max = {-32768, -32768, -32768};
  
LSM303DLM compass;
LSM303DLM::vector running_min = {2047, 2047, 2047}, running_max = {-2048, -2048, -2048};

void setup()
{
  Serial.begin( 57600);

  Serial.println( "setup()");

  Wire.begin();

  compassD.init();
  compassD.enableDefault();

  compass.init();
  compass.enableDefault();
}

char report[80];

void loop()
{
  compassD.read();
  runningD_min.x = min( runningD_min.x, compassD.m.x);
  runningD_min.y = min( runningD_min.y, compassD.m.y);
  runningD_min.z = min( runningD_min.z, compassD.m.z);
  runningD_max.x = max( runningD_max.x, compassD.m.x);
  runningD_max.y = max( runningD_max.y, compassD.m.y);
  runningD_max.z = max( runningD_max.z, compassD.m.z);
  snprintf(
    report, sizeof( report), 
    "minD:{%+6d, %+6d, %+6d}  maxD:{%+6d, %+6d, %+6d}",
    (int)runningD_min.x, (int)runningD_min.y, (int)runningD_min.z,
    (int)runningD_max.x, (int)runningD_max.y, (int)runningD_max.z);
  Serial.println( report);
  delay( 100);
 
  compass.read();
  running_min.x = min( running_min.x, compass.m.x);
  running_min.y = min( running_min.y, compass.m.y);
  running_min.z = min( running_min.z, compass.m.z);
  running_max.x = max( running_max.x, compass.m.x);
  running_max.y = max( running_max.y, compass.m.y);
  running_max.z = max( running_max.z, compass.m.z);
  snprintf(
    report, sizeof( report), 
    "min:{%+6d, %+6d, %+6d}  max:{%+6d, %+6d, %+6d}\n",
    (int)running_min.x, (int)running_min.y, (int)running_min.z,
    (int)running_max.x, (int)running_max.y, (int)running_max.z);
  Serial.println( report);
  delay( 100);

}
// min:{ -8960,  -4023,  -5655}  max:{   +43,  +1573,   -486}
// min:{ -7749,  -4457,  -3400}  max:{ -3920,  -1183,  -1410}
//
// minD:{-11171,  -4518,  -4569}  maxD:{ -1811,   +556,   +986}
// min: {  -590,   -657,   -525}  max: {  +412,   +513,   +427}
//
// minD:{ -3390,  -3139,  -2784}  maxD:{ +3179,  +3122,  +3125}
// min:{   -617,   -577,   -611}  max:{   +455,   +503,   +353}
