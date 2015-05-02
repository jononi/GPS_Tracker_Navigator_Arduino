// gps project jaafar benabdallah 10/2013
// v 0.9.1: complete application with three modes: tracking, exploration and navigation.
// 05/2015: updated lcd library to comply with Arduino >1.5.7 and using new SPI library
// 05/2015: checks up and update info page with software version
// current size: 

/*free RAM report: 650 with new SPI lib. (used to be 750)
To Do:
1- use the new TinyGPS library /!\ remember to save and add my own changes to this lib
2- implement higher reading rate from GPS module and averaging of readings
 */
//-----------------------------------------------------------------------------------

//Libs
#include <SPI.h>
#include <SdFat.h>  //SD_FAT_VERSION 20130629
#include <Adafruit_ST7735.h> //customized class, and reduced font table: -786 bytes,works with arduino >1.5.7
#include <TinyGPS.h> //customized, some functions removed and some other added. From GPS_VERSION 12 
#include <MemoryFree.h> //needed for debug only 

#include "GPS_modular.h" //header file: data structures and definitions 

//---------------------------------------------------------------------------------

// Instances & global variables
Adafruit_ST7735 tft = Adafruit_ST7735(tft_cs,tft_dc,tft_rst);//adafruit's ST7735 lcd
TinyGPS gps;//old version, to be updated
StateReg state;// status register structure to know what's going on
gpsData gdata;// gps data structure
SdFat SD;//file system object
fstream sdFile;//file stream with R/W access

//set the TFT display as the output device for cout
ArduinoOutStream cout(tft);


//---------------------------------------------------------------------------------
void setup(){
  state.screen=3;// enable Mode Select screen
  setupCommon();
  setupMode();//mode select menu, exits with active mode
  state.screen = 4;//Indefinite screen to avoid changing modes while pressing buttons
  
  // local vars
  byte error_code=0;
  char name[] = "WAYPTS00.CSV";//filename template, valid for E and N modes
  
  // check for presence of SD card and initialize filesystem (FAT)
  //if (!SD.init(SPI_FULL_SPEED, sd_cs)) error_code = 1;
  if (!SD.begin(sd_cs,SPI_FULL_SPEED)) error_code = 1;
  else {
    // select file name and setup file access according to selected mode
    setupFile(&state, name);
    // error code if file not successfully open
    if (!sdFile.is_open()) error_code+=2;
    // set date time callback function
    SdFile::dateTimeCallback(dateTime);//126 bytes
  }

  // if errors, disable SD card operations
  if (error_code) state.mode = 3;//no sd card mode, basically a T mode without logging
  
  // display main screen frame (and enable main screen)  
  mainScreenFrame(state);
  // display active file name, otherwise print the error code
  tft.setCursor(3,103);
  if (error_code) cout << pstr("error: ") << int(error_code);
  else cout << name;
}

//---------------------------------------------------------------------------------

void loop(){
  eventListener(state);
  setAction(state,gdata);
}

//----------------------- call back function for file timestamps ------------------
void dateTime(uint16_t* date, uint16_t* time) {    
  // return date using FAT_DATE macro to format fields
  *date = FAT_DATE(gdata.year, gdata.month, gdata.day);

  // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(gdata.hrs, gdata.mins, gdata.secs);
}

