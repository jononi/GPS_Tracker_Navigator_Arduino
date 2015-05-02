void setAction(StateReg &state, gpsData &gd){
  static uint32_t addPTimer;// timer for automatic TP/WP saving

  if (state.newfix){

    //refresh geolocation data
    refreshData(gd);//1322 bytes!

    // update navigation data if N mode and trip on going
    if (state.mode==2 && state.trip_on ){
      refreshNavigation(gd);//1788 bytes (robust version),1372 bytes (simple version)
    }

    // enabling WP/TP saving when time is up
    if (state.trip_on){
      // trip is on, automatically adding points is possible
      if (state.mode == 0 && ( millis()-addPTimer > TPaddTimer) ) {
        // tracking mode active and 2 sec elapsed since last saving
        state.addTP = 1;//enable TP saving
        addPTimer = millis();//reset timer
      }

      if (state.mode == 1 && ( millis()-addPTimer > WPaddTimer) ) {
        // exploration mode active and 10 sec elapsed since last saving
        state.addWP = 1;//enable WP saving
        addPTimer = millis();//reset timer
      }
    }
    //or state.addTP, state.addWP can be set by other means than the timers above

    // save a TP or WP if enabled and T or E modes
    if (state.mode < 2) addPstream(gd, state);

    // data is ready to be displayed, refresh main screen if active
    if (!state.screen) {
      refreshMainScreen(gd); 
    }

    state.newfix=0;//reset new fix flag when done with the fix
  }
  //dipslay info screen if enabled
  if (state.screen==1) {
    refreshInfoScreen(gd);
  }
}

//------------------------------------------------------------------------------------------------------------

void refreshData(gpsData &gd){
  //-------------------------Date, Time and Trip time--------------------------
  static byte old_secs=255;// init at safe value
  gps.crack_datetime(&gd.year, &gd.month, &gd.day, &gd.hrs, &gd.mins, &gd.secs, &gd.hdrds, &gd.loc_age); 
  
  /* for Tunis time
  if (gd.hrs==23){
    gd.day++;
    gd.hrs=0;
  }
  else {
    gd.hrs+=1;
  } 
 */ 
  
/* for EDT   */
  if (gd.hrs<4) {
    gd.day--; // kinda correcting the day (wrong if it's the first of the month) 
    gd.hrs+=8; // time correcting for EDT timezone
  }
  else gd.hrs-=4;//time correcting for EDT timezone

  // update trip time
  if ( (old_secs < 255) && state.trip_on ) {
    // it's not the first time we go here
    if (old_secs > gd.secs) {
      //i.e. old_sec=59, gd_secs=2
      gd.tr_secs += 60 + gd.secs - old_secs; 
    }
    else {
      gd.tr_secs += gd.secs - old_secs; 
    }
  }  
  old_secs = gd.secs;//on the first time, it starts here
  //carry out...
  if (gd.tr_secs > 59) { 
    gd.tr_mins++;//...for seconds
    gd.tr_secs -= 60;//for the case tr_secs>60
  }
  if (gd.tr_mins > 59) {
    gd.tr_hrs++;//...for minutes
    gd.tr_mins = 0;
  }
  //------------------------Location and Trip distance-------------------------------
  static float old_lat;//on first time, this is going to be 0
  static float old_lon;

  gps.f_get_position(&gd.lat, &gd.lon, &gd.loc_age);

  // replace invalid values by -1 (avoid garbage when displaying in info screen before any fix)
  // size = 92 bytes
  /*
  gd.loc_age != TinyGPS::GPS_INVALID_AGE ? :gd.loc_age=-1;//see http://en.wikipedia.org/wiki/%3F:
   gd.lat != TinyGPS::GPS_INVALID_F_ANGLE ? :gd.lat=-1;
   gd.lon != TinyGPS::GPS_INVALID_F_ANGLE ? :gd.lon=-1;
   */

  if (old_lat > 0 && state.trip_on){
    // these 2 lines saves space over the 3rd line!
    uint16_t dist = gps.distance_angle_between(old_lat, old_lon, gd.lat, gd.lon,1);
    gd.tripdistance+= dist;
    //gd.tripdistance+=gps.distance_angle_between(old_lat, old_lon, gd.lat, gd.lon,1);  
  }
  old_lat = gd.lat;
  old_lon = gd.lon;
}

//-------------------------------------Navigation---------------------------------------------------------

//work in progress....
void refreshNavigation(gpsData &gd){

  uint32_t pos; // absolute stream position
  uint32_t wpidx;// WP index
  static uint32_t old_dist = 0;

  if (sdFile.eof()) {
    prepContainer(2,95,120,ST7735_RED);
    cout << pstr("End of WP file.");  
    return;
  }  

  if (state.nxtWP_en > sw_thr){
    pos = sdFile.tellg();//tells where to start parsing (useful for reverse reading)
    // keep loading WP until no WP is behind
    //while (abs(gps.distance_angle_between(gd.lat, gd.lon, gd.wplat, gd.wplon,0)-radians(gps.f_course())) > 1.57){
      readWPLine(&wpidx, &gd.wplat, &gd.wplon, &pos);  //get next WP from file
    //}
    gd.nbPTS = wpidx; 
    // debug
    //prepContainer(2,95,157,ST7735_RED);
    //cout << wpidx << ',' << setprecision(6) << wplat << ',' << wplon;
    // /debug
    state.nxtWP_en = 0; //reset the switch enable flag
  }

  // update the distance between current location and  current (new) WP
  gd.distTo = (unsigned int)gps.distance_angle_between(gd.lat, gd.lon, gd.wplat, gd.wplon,1);

  //dist_thr reduces the effect of jitte in locations
  if (gd.distTo > old_dist + dist_thr) {
    state.nxtWP_en++;
  }

  old_dist = gd.distTo;
  // update the courseTo
  gd.courseTo = gps.distance_angle_between(gd.lat, gd.lon, gd.wplat, gd.wplon,0);
  // debug: check that bearing To relative to N and bearing To computed correctly
  prepContainer(2,95,157,ST7735_RED);
  cout << "ctN:" << gd.courseTo << ",c:" << gps.f_course();
  // /debug
}

//---------------------------------------------------------------------------------
// write TP or WP to SD file in T and E modes
void addPstream(gpsData &gd, StateReg &state){
  static byte doflush=flushCounter;//counter to flush to sd card reglarly

  if (state.addTP){
    //tracking mode & addTP
    sdFile << "T," << setfill('0') << setw(2) << (int)gd.hrs << ':';
    sdFile << setfill('0') << setw(2) << (int)gd.mins << ':' << setw(2) << (int)gd.secs << ',';
    sdFile << setprecision(6) << gd.lat <<',';
    sdFile << gd.lon <<',';
    sdFile << setprecision(2) << gps.f_altitude() <<',';
    sdFile << gps.f_speed_kmph() << endl;
    gd.nbPTS++;
    doflush--;
    state.addTP = 0;//reset flag
  }
  else if (state.addWP){
    //exploration mode & add WP
    gd.nbPTS++;
    sdFile << gd.nbPTS << ',' << setprecision(6) << gd.lat <<',' << gd.lon << endl;
    doflush--;
    state.addWP = 0;//reset flag
  }

  //debug
  prepContainer(100,85,24,ST7735_YELLOW);
  cout << "df=" << int(doflush);//next flush time indicator
  //

  if (!doflush){
    sdFile.flush();//effectively write data to SD every 5 points saved, starting by the 6th point
    doflush=flushCounter;//reset counter
  }
}

//------------------------ Line read and parse from inout stream for navigation function -------------------

void readWPLine(uint32_t *wpidx, float *wplat, float *wplon, const uint32_t *newLinePos){
  char wpbuf[12];
  // go to indicated position
  sdFile.seekg(*newLinePos);//now we are at the start of a new line

  // /!\ using getline because it extracts the delimiter too. (refer to sdfat lib:fstream documentation)
  // extracting the delimiter allows the parsing functions to know where to stop
  // /!\ endl adds \r\n pair of chars
  // read first field: WP index
  sdFile.getline(wpbuf,5,',');//reads at most 5 chars, stop anyway if meets a comma		
  // parse WP index to int
  *wpidx = parse_int(wpbuf); // validated

  // read second field: WP lat
  sdFile.getline(wpbuf,12,',');
  // parse WP lat to float
  *wplat = parse_decimal(wpbuf);//validated

  // read third field: WP lon
  sdFile.getline(wpbuf,12,'\r');
  // parse WP lat to float
  *wplon = parse_decimal(wpbuf);
}

//----------------------------------------- Parsing functions ---------------------------------------- 
//check if the char represents a digit:
bool isdigit(char c) { 
  return c >= '0' && c <= '9'; 
}
// parse string into int
uint32_t parse_int(const char *str){
  uint32_t ret =0;
  while (isdigit(*str)) {
    ret = 10 * ret + *str++ - '0';
  }  
  return ret;
}

// parse string into signed decimal (float)
// /!\ b/c of setprecison(6) in WP saving, the mantissa will always have 6 digits (easier to parse)
// unless the file was edited with excel or alike then saved again on the card without specifying 
// that #digits right of the decimal point is 6.

float parse_decimal(const char *str){
  bool isneg = *str == '-';//check if the first char is '-' sign
  if (isneg) ++str;//skip the '-' sign if it's there
  uint32_t int_part = parse_int(str); //parse the base part to int
  while (*str != '.') str++; // advance to the radix sign
  str++; //jump over the radix sign
  uint32_t frac_part = parse_int(str); // parse the mantissa to int
  float ret = int_part + frac_part/1000000.0f;
  return isneg ? -ret : ret;
}









