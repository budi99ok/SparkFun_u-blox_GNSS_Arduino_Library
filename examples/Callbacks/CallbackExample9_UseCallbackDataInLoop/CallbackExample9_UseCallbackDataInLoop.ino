/*
  Configuring the GNSS to automatically send position reports over I2C and display them using a callback
  By: Paul Clark
  SparkFun Electronics
  Date: April 15th, 2022
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to access the callback data from the main loop.
  The simple way is to use a global flag: set it in the callback, check it and clear it in the main loop.
  Or, you can be more sophisticated and use the callback flags themselves.

  Uncomment the #define useGlobalFlag below to use the simple method.

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  Hardware Connections:
  Plug a Qwiic cable into the GPS and a BlackBoard
  If you don't have a platform with a Qwiic connection use the SparkFun Qwiic Breadboard Jumper (https://www.sparkfun.com/products/14425)
  Open the serial monitor at 115200 baud to see the output
*/

//#define useGlobalFlag // Uncomment this line to use a global flag to indicate that the callback data is valid

#ifdef useGlobalFlag
bool newPVTdata = false;
#endif

#include <Wire.h> //Needed for I2C to GPS

#include <SparkFun_u-blox_GNSS_Arduino_Library.h> //http://librarymanager/All#SparkFun_u-blox_GNSS
SFE_UBLOX_GNSS myGNSS;

// Callback: callbackPVT will be called when new NAV PVT data arrives
// See u-blox_structs.h for the full definition of UBX_NAV_PVT_data_t
//         _____  You can use any name you like for the callback. Use the same name when you call setAutoPVTcallbackPtr
//        /                  _____  This _must_ be UBX_NAV_PVT_data_t
//        |                 /               _____ You can use any name you like for the struct
//        |                 |              /
//        |                 |              |
void callbackPVT(UBX_NAV_PVT_data_t *ubxDataStruct)
{

#ifdef useGlobalFlag // If we are using the global flag

  newPVTdata = true;
  
#endif

}

void setup()
{
  Serial.begin(115200);
  while (!Serial); //Wait for user to open terminal
  Serial.println("SparkFun u-blox Example");

  Wire.begin();

  //myGNSS.enableDebugging(); // Uncomment this line to enable helpful debug messages on Serial

  if (myGNSS.begin() == false) //Connect to the u-blox module using Wire port
  {
    Serial.println(F("u-blox GNSS not detected at default I2C address. Please check wiring. Freezing."));
    while (1);
  }

  myGNSS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGNSS.saveConfigSelective(VAL_CFG_SUBSEC_IOPORT); //Save (only) the communications port settings to flash and BBR

  myGNSS.setNavigationFrequency(2); //Produce two solutions per second

  myGNSS.setAutoPVTcallbackPtr(&callbackPVT); // Enable automatic NAV PVT messages with callback to callbackPVT
}

void loop()
{
  myGNSS.checkUblox(); // Check for the arrival of new data and process it.


#ifndef useGlobalFlag // If we are not using the global flag

  static bool callbackDataValid = false; // Flag to show that fresh callback data is available

  // Is the callback data valid?
  // If automaticFlags.flags.bits.callbackCopyValid is true, it indicates new PVT data has been received and has been copied
  // automaticFlags.flags.bits.callbackCopyValid will be cleared when the callback is called
  
  if ((callbackDataValid == false) && (myGNSS.packetUBXNAVPVT->automaticFlags.flags.bits.callbackCopyValid == true))
  {
    Serial.println(F("NAV PVT callback data is now valid"));
    callbackDataValid = true; // Set the flag
  }

#endif

  
  myGNSS.checkCallbacks(); // Check if any callbacks are waiting to be processed.

  // Check if new PVT data has been received


#ifdef useGlobalFlag

  if (newPVTdata)
  {


#else // If we are not using the global flag


  // automaticFlags.flags.bits.callbackCopyValid will have been cleared after the callback was called
  
  if ((callbackDataValid == true) && (myGNSS.packetUBXNAVPVT->automaticFlags.flags.bits.callbackCopyValid == false))
  {
    callbackDataValid = false; // Clear the flag

  
#endif


    Serial.println();

    Serial.print(F("Time: ")); // Print the time
    uint8_t hms = myGNSS.packetUBXNAVPVT->callbackData->hour; // Print the hours
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F(":"));
    hms = myGNSS.packetUBXNAVPVT->callbackData->min; // Print the minutes
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F(":"));
    hms = myGNSS.packetUBXNAVPVT->callbackData->sec; // Print the seconds
    if (hms < 10) Serial.print(F("0")); // Print a leading zero if required
    Serial.print(hms);
    Serial.print(F("."));
    unsigned long millisecs = myGNSS.packetUBXNAVPVT->callbackData->iTOW % 1000; // Print the milliseconds
    if (millisecs < 100) Serial.print(F("0")); // Print the trailing zeros correctly
    if (millisecs < 10) Serial.print(F("0"));
    Serial.print(millisecs);

    long latitude = myGNSS.packetUBXNAVPVT->callbackData->lat; // Print the latitude
    Serial.print(F(" Lat: "));
    Serial.print(latitude);

    long longitude = myGNSS.packetUBXNAVPVT->callbackData->lon; // Print the longitude
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = myGNSS.packetUBXNAVPVT->callbackData->hMSL; // Print the height above mean sea level
    Serial.print(F(" Height above MSL: "));
    Serial.print(altitude);
    Serial.println(F(" (mm)"));
  }

  Serial.print(".");
  delay(50);
}
