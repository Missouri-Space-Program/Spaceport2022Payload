
//Libraries and definitions needed to get SD card on the Feather working
#include <SD.h>
#include <SPI.h>
#include "RTClib.h"
#define cardSelect 4

RTC_DS3231 rtc;
//Pins in use by the acceleormeters
const int x1Input = A0;
const int y1Input = A1;
const int z1Input = A2;

const int x2Input = A3;
const int y2Input = A4;
const int z2Input = A5;
// Raw Ranges:

//These ranges were determined after callibration, so default on each axis should be around 1G
int x1RawMin = 490;
int x1RawMax = 528;

int y1RawMin = 491;
int y1RawMax = 533;

int z1RawMin = 494;
int z1RawMax = 532;


int x2RawMin = 492;
int x2RawMax = 530;

int y2RawMin = 491;
int y2RawMax = 530;

int z2RawMin = 494;
int z2RawMax = 531;

// Take multiple samples to reduce background noise
const int sampleSize = 10;

//Variables for our file that will be used to log data
char filename[15];
File logger;



void dateTime(uint16_t* date, uint16_t* time)
{
  DateTime now = rtc.now();
  *date = FAT_DATE(now.year(), now.month(), now.day());
  *time = FAT_TIME(now.hour(), now.minute(), now.second());
}

void setup() 
{
  //Open up serial communications on 9600 baud (This will be removed in final push)
  Serial.begin(57600);
  #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
  #endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //Create a reference pin that our accelerometers will use
  analogReference(AR_EXTERNAL);

  SdFile::dateTimeCallback(dateTime);

  //Check if the SD card can begin on the specified pin, if not then print an error message
  if(!SD.begin(cardSelect)) {
    Serial.println("Card initalization failed!");
  }
  //Because strings in C are literals, we have to use strcpy to set our file name up for use in the file system.
  strcpy(filename, "/DATA00.TXT");
  //This is a handy filesystem created by the folks at Adafruit that allows for a new file to be opened and written to each time the device is reset
  for (uint8_t i = 0; i < 100; i++) {
    filename[5] = '0' + i/10;
    filename[6] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (!SD.exists(filename)) {
      break;
    }
  }
  //Initalize our logging variable by opening it the first time and testing a write
  logger = SD.open(filename, FILE_WRITE);
  //If it wasn't opened, then alert the user
  if(!logger) {
    Serial.println("Couldn't open file!");
  }
  //Close the file after the test
  logger.close();
}

void loop() {
  DateTime now = rtc.now();
  //Use our getAxis function to get the raw data from the accelerometer
  long x1Raw = getAxis(x1Input);
  long y1Raw = getAxis(y1Input);
  long z1Raw = getAxis(z1Input);

  long x2Raw = getAxis(x2Input);
  long y2Raw = getAxis(y2Input);
  long z2Raw = getAxis(z2Input);

  // Convert raw values to 'milli-Gs" by using the map function
  long x1Scaled = map(x1Raw, x1RawMin, x1RawMax, -1000, 1000);
  long y1Scaled = map(y1Raw, y1RawMin, y1RawMax, -1000, 1000);
  long z1Scaled = map(z1Raw, z1RawMin, z1RawMax, -1000, 1000);

  long x2Scaled = map(x2Raw, x2RawMin, x2RawMax, -1000, 1000);
  long y2Scaled = map(y2Raw, y2RawMin, y2RawMax, -1000, 1000);
  long z2Scaled = map(z2Raw, z2RawMin, z2RawMax, -1000, 1000);

  // re-scale to fractional Gs
  float x1Accel = x1Scaled / 1000.0;
  float y1Accel = y1Scaled / 1000.0;
  float z1Accel = z1Scaled / 1000.0;

  float x2Accel = x2Scaled / 1000.0;
  float y2Accel = y2Scaled / 1000.0;
  float z2Accel = z2Scaled / 1000.0;
    
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print('/');
  Serial.print(now.year(), DEC);
  Serial.print(' ');
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);  
  Serial.print(" Temp:");
  Serial.print(rtc.getTemperature());
  Serial.print("C");
  //Print the output of the accelerometers (This is for debugging)
  Serial.print(" X1:");
  Serial.print(x1Accel);
  Serial.print("G Y1:");
  Serial.print(y1Accel);
  Serial.print("G Z1:");
  Serial.print(z1Accel);
  Serial.print("G");

  Serial.print(" X2:");
  Serial.print(x2Accel);
  Serial.print("G Y2:");
  Serial.print(y2Accel);
  Serial.print("G Z2:");
  Serial.print(z2Accel);
  Serial.println("G");
  //Reopen the file to be written to
  logger = SD.open(filename, FILE_WRITE);
  //Begin to print our data to the file

  logger.print(now.month(), DEC);
  logger.print('/');
  logger.print(now.day(), DEC);
  logger.print('/');
  logger.print(now.year(), DEC);
  logger.print(' ');
  logger.print(now.hour(), DEC);
  logger.print(':');
  logger.print(now.minute(), DEC);
  logger.print(':');
  logger.print(now.second(), DEC);

  logger.print(" ");
  logger.print(rtc.getTemperature());
  //logger.print("C");

 // logger.print(" X:");
  logger.print(' ');
  logger.print(x1Accel);
 // logger.print("G Y:");
  logger.print(' ');
  logger.print(y1Accel);
 // logger.print("G Z:");
  logger.print(' ');
  logger.print(z1Accel);

  logger.print(' ');
  logger.print(x2Accel);
  logger.print(' ');
  logger.print(y2Accel);
  logger.print(' ');
  logger.print(z2Accel);
  logger.println();
  //This adds a 500 millisecond delay so we have some gaps in the data receieved
 // delay(500);
  //Finally close the file when done writing to get the data to write to the SD card
  logger.close();
}

//This function will take in a given pin and return the raw reading of the axis given as an integer
int getAxis(int pin) {
  //Create a result to store each sample added up
  long result = 0;
  //Go through and read off the pin the set number of times (10) and add them up
  for(int i = 0; i < sampleSize; i++) {
    result += analogRead(pin);
  }
  //Finally return the average (divided by the sample size)
  return result / sampleSize;
}
