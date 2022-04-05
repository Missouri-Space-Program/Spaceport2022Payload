
//Libraries and definitions needed to get SD card on the Feather working
#include <SD.h>
#include <SPI.h>
#define cardSelect 4

//Pins in use by the acceleormeters
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
// Raw Ranges:

//These ranges were determined after callibration, so default on each axis should be around 1G
int xRawMin = 492;
int xRawMax = 530;

int yRawMin = 491;
int yRawMax = 530;

int zRawMin = 494;
int zRawMax = 532;

// Take multiple samples to reduce background noise
const int sampleSize = 10;

//Variables for our file that will be used to log data
char filename[15];
File logger;
void setup() 
{
  //Create a reference pin that our accelerometers will use
  analogReference(AR_EXTERNAL);
  //Open up serial communications on 9600 baud (This will be removed in final push)
  Serial.begin(9600);
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
  //Use our getAxis function to get the raw data from the accelerometer
  long xRaw = getAxis(xInput);
  long yRaw = getAxis(yInput);
  long zRaw = getAxis(zInput);
  // Convert raw values to 'milli-Gs" by using the map function
  long xScaled = map(xRaw, xRawMin, xRawMax, -1000, 1000);
  long yScaled = map(yRaw, yRawMin, yRawMax, -1000, 1000);
  long zScaled = map(zRaw, zRawMin, zRawMax, -1000, 1000);

  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  float zAccel = zScaled / 1000.0;
  //Print the output of the accelerometers (This is for debugging)
  Serial.print("X: ");
  Serial.print(xAccel);
  Serial.print("G, Y: ");
  Serial.print(yAccel);
  Serial.print("G, Z: ");
  Serial.print(zAccel);
  Serial.println("G");
  //Reopen the file to be written to
  logger = SD.open(filename, FILE_WRITE);
  //Begin to print our data to the file
  logger.print(" X: ");
  logger.print(xAccel);
  logger.print("G, Y: ");
  logger.print(yAccel);
  logger.print("G, Z: ");
  logger.print(zAccel);
  logger.println("G");
  //This adds a 500 millisecond delay so we have some gaps in the data receieved
  delay(500);
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