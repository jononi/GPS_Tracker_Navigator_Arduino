//
#define version "0.9.1"

// peripherals (TFT, SD, GPS) pins
#define tft_cs 5
#define tft_dc 8
#define tft_rst 7 
#define sd_cs 9
#define tft_lite 6//backlite enable pin,if space available, use it to dim backlite
// button1: A0  button2: A1  button3: A2

// other definitions
#define orientation 1//correct orientation for the TFT display
// delivers RMC and GGA sentences only
#define PMTK_SET_NMEA_OUTPUT_RMCGGA "$PMTK314,0,1,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0*28"
// update frequency = 1Hz
#define PMTK_SET_NMEA_UPDATE_1HZ  "$PMTK220,1000*1F"

// settings
#define listenTimeOut 250u //# ms spent on UART listening for GPS data
#define hdop_thr 150u //thershold for hdop accuracy measure to display warning sign 
#define TPaddTimer 1000u //# ms between trackpoints data logging in T mode
#define WPaddTimer 10000u //# ms between waypoints logging in E mode
#define flushCounter 5 //number of points to be logged before writing data to sd card
#define sw_thr 1 //threshold for the number of times the distance to next WP is allowed 
//                 to increase before switching to the next WP in file
#define dist_thr 3//threshold in meters for the increase of distance to the
//                 current WP that will enable loading the next WP

//-----------------------------------------------------------------------------------
struct StateReg {
  //mode 0:tracking, 1:new trail, 2:follow trail (navigation), 3: no SD card 
  byte mode;
  // screen: 0: main, 1: stats, 2: more data, 3: select Mode
  byte screen;
  // boolean or byte it's all the same in RAM
  byte newfix; //flag for new fix available
  byte trip_on; //flag for trip going on: enable logging or navigation
  byte addTP; // flag to enable saving one trackpoint
  byte addWP; // flag to enable saving one waypoint
  byte button; //0: no button pressed, otherwise 1,2 or 3
  byte nxtWP_en;//enable switching to next WP
};

//-----------------------------------------------------------------------------------
struct gpsData {
  // date and time
  int year; 
  byte month;
  byte day;
  byte hrs;
  byte mins;
  byte secs;
  byte hdrds;
  
  // fix age
  uint32_t loc_age;
  
  // geolocation
  float lat;
  float lon;
  
  // computed trip data
  uint32_t tripdistance;
  byte tr_secs;
  byte tr_mins;
  byte tr_hrs;
  
  //counter for  trackpoints/waypoints
  uint16_t nbPTS;
  
  // navigation
  uint16_t distTo;// in meters
  float courseTo;// in radians, relative to current course
  float wplat;
  float wplon;
};

