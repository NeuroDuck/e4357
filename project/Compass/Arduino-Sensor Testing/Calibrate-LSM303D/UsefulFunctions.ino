//
// Functions that may be useful everywhere.
//
void printEnclosedData( char* desc, char* s)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( s);
    Serial.println( "<");
}

void printEnclosedBinData( char const* desc, byte b)
{
    Serial.print( desc);
    Serial.print( " = >0b");
    Serial.print( b, BIN);
    Serial.println( "<");
}

void printEnclosedHexData( char const* desc, long l)
{
    Serial.print( desc);
    Serial.print( " = >0x");
    Serial.print( l, HEX);
    Serial.println( "<");
}

void printEnclosedInt8( char const* desc, uint8_t i8)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( i8, DEC);
    Serial.println( "<");
}
void printEnclosedData( char const* desc, int i)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( i);
    Serial.println( "<");
}

void printEnclosedData( char* desc, uint16_t i16)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( i16);
    Serial.println( "<");
}

void printEnclosedLong( char const* desc, long l)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( l);
    Serial.println( "<");
}

void printEnclosedData( char* desc, char c)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( c);
    Serial.println( "<");
}
      
void printEnclosedData( char* desc, boolean b)
{
    Serial.print( desc);
    Serial.print( " = >");
    Serial.print( b);
    Serial.println( "<");
}

#define MAX_MSG_LEN 80

void queueFakeSettingUpdatedMsg( 
  const char* settingName, byte settingValue)
{
  char settingValueString[MAX_MSG_LEN] = "";
  itoa( (int)settingValue, settingValueString, 10);

  queueFakeSettingUpdatedMsg( settingName, settingValueString);
}

void queueFakeSettingUpdatedMsg( 
  const char* settingName, int settingValue)
{
  char settingValueString[MAX_MSG_LEN] = "";
  itoa( settingValue, settingValueString, 10);

  queueFakeSettingUpdatedMsg( settingName, settingValueString);
}

void queueFakeSettingUpdatedMsg( 
  const char* settingName, long settingValue)
{
  char settingValueString[MAX_MSG_LEN] = "";
  ltoa( settingValue, settingValueString, 10);

  queueFakeSettingUpdatedMsg( settingName, settingValueString);
}

void queueFakeSettingUpdatedMsg( 
  const char* settingName, uint16_t settingValue)
{
  char settingValueString[MAX_MSG_LEN] = "";
  itoa( (int)settingValue, settingValueString, 10);

  queueFakeSettingUpdatedMsg( settingName, settingValueString);
}
void queueFakeSettingUpdatedHexMsg( 
  const char* settingName, byte settingValue)
{
  char settingValueString[MAX_MSG_LEN] = "";
  itoa( settingValue, settingValueString, 16);

  queueFakeSettingUpdatedMsg( settingName, settingValueString);
}

void queueFakeSettingUpdatedMsg( 
  const char* settingName, char* settingValue)
{
  char msg[MAX_MSG_LEN];

  strcpy( msg, settingName);
  strcat( msg, " = >");
  strcat( msg, settingValue);
  strcat( msg, "<\n");
  
  Serial.print( msg);

/*
  byte msgLen = strlen( msg);
  
  if (msgLen >= MAX_MSG_LEN)
  {
    strcpy( isrMsgs, "MAX_MSG_LEN is overflowing!!!\n");
    return;
  }

  if (isrMsgsLen + msgLen < MAX_ISR_MSGS * MAX_MSG_LEN)
    strcat( isrMsgs, msg);
  else
    strcpy( isrMsgs, "Print Queue is overflowing!!!\n");

  isrMsgsLen = strlen( isrMsgs);
*/
}

/*
size_t strcspn ( const char * str1, const char * str2 );
Get span until character in string
Scans str1 for the first occurrence of any of the characters 
that are part of str2, returning the number of characters of 
str1 read before this first occurrence.

The search includes the terminating null-characters. 
Therefore, the function will return the length of str1 if 
none of the characters of str2 are found in str1.
*/
int trimString( char* s)
{
  int newLength = strcspn( s, "\r\n");
  
  s[newLength] = '\0';
  
  return( newLength);
}

/*
char * strpbrk ( char * str1, const char * str2 );
Locate characters in string
Returns a pointer to the first occurrence in str1 of any of 
the characters that are part of str2, or a null pointer if 
there are no matches.

The search does not include the terminating null-characters 
of either strings, but ends there.
*/
int trimString1( char* s)
{
  char* stringTermChar = strpbrk( s, "\r\n");
  
  stringTermChar = '\0';
  
  return( strlen( s));
}

// From:
//   https://learn.adafruit.com/memories-of-an-arduino/measuring-free-memory
//
int freeRam()
{
  extern int __heap_start, *__brkval;

  int v;

  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}

void printFreeRam()
{
  int freeRamResult = freeRam();
  queueFakeSettingUpdatedMsg( "freeRam", freeRamResult);
}

//              123456789a123456789b
// uint64_t x = 18446744073709551616LL; // (unsigned).
//
void printLL( uint64_t x0)
{                          //  1234567890
  const uint64_t divisor    = 1000000000LL;  // Remove the lowest 10 digits. 

  long x = x0 / divisor;
  Serial.print( x);
  Serial.print( ",");

//  x = x0 - x * divisor;
  x = x0 % divisor;

  char buf[20];
  ultoa( (uint32_t)x, buf, 10);

  for (byte i = 0 ; i < 9 - strlen( buf) ; i++)
  {
    Serial.print( "0");
  }
  Serial.println( buf);
}

