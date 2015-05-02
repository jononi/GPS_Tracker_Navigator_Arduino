
//--------------------------------Main Screen Elements-------------------------------------------------
void mainScreenFrame(StateReg &state){
  state.screen=0;//set the associated flag
  tft.fillScreen(ST7735_BLACK); //clear screen
  tft.drawRect(15,5,120,12,ST7735_WHITE);//1:date and time container
  tft.drawRect(145,5,10,12,ST7735_WHITE);//10: active mode container
  //experimental
  tft.setCursor(5,18);
  cout << pstr("speed");
  //
  tft.drawRect(5,26,40,12,ST7735_WHITE);//2:speed container
  //experimental
  tft.setCursor(115,18);
  cout << pstr("crs/dst");
  //
  tft.drawRect(115,26,40,12,ST7735_WHITE);//3:course/ distanceTo container
  drawCircle(78,52,31,ST7735_WHITE);//4:course visualization container
  tft.drawRect(5,70,40,12,ST7735_WHITE);//5: trip distance container
  tft.drawRect(109,70,46,12,ST7735_WHITE);//6:trip time container
  //experimental
  tft.setCursor(14,40);
  cout << pstr("PTS");
  //
  tft.drawRect(14,49,22,12,ST7735_WHITE);//11:number of points (TP, WP) processed

  tft.setTextColor(ST7735_GREEN);
  // display active mode in 11
  tft.setCursor(147,7);
  if (!state.mode) cout << 'T';//tracking
  if (state.mode==1) cout << 'E';//exploration
  if (state.mode==2) cout << 'N';//navigation
  if (state.mode==3) cout << '!';//no SD card

  //container for start/pause button (2) in 7
  tft.drawRect(61,113,38,12,ST7735_WHITE);
  tft.setCursor(63,115);
  if (state.mode < 4) cout << pstr("Start");
  else cout << pstr("No SD");

  //container for Info screen button (3) in 8
  tft.drawRect(119,113,38,12,ST7735_WHITE);
  tft.setCursor(121,115);
  cout << pstr("Info");

  //container for add point/switch to next point contextual button (1 in 9

  tft.drawRect(3,113,38,12,ST7735_WHITE); 
  tft.setCursor(5,115);
  if (!state.mode) cout << pstr("addTP");
  if (state.mode==1) cout << pstr("addWP");
  if (state.mode==2) cout << pstr("nxtWP");
}

//--------------------------------Info Screen Elements-------------------------------------------------
void infoScreenFrame(){
  tft.fillScreen(ST7735_BLACK);
  // labels for info data
  tft.setCursor(4,9);
  cout << pstr("age,#sat,hdop,CSerr");  

  // labels for location data
  tft.setCursor(4,29);
  cout << pstr("lat,lon"); 

  //container for 'back' button
  tft.drawRect(61,113,38,12,ST7735_WHITE);
  tft.setCursor(63,115);
  tft.setTextColor(ST7735_GREEN);
  cout << "Back";
}  

//---------------------------------- Main Screen Data  ---------------------------------------------- 
void refreshMainScreen(gpsData &gd){
  prepContainer(17,7,116,ST7735_YELLOW);
  // display date and time in 1
  //cout consumes 318 bytes
  cout << setfill('0') << setw(2) << (int)gd.day << '/' << setw(2) <<(int)gd.month  << '/' << gd.year ;
  cout << ' ' << setw(2) <<  (int)gd.hrs << ':' << setw(2) << (int)gd.mins << ':' << setw(2) << (int)gd.secs;

  // display current speed in 2 
  prepContainer(7,28,36,ST7735_YELLOW);
  cout << setprecision(2) << gps.f_speed_kmph();

  // display low accuracy warning (58 bytes)
  prepContainer(36,18,8,ST7735_RED);
  if (gps.hdop() > hdop_thr) cout << '!';

  // display current course in 3
  prepContainer(117,28,36,ST7735_YELLOW);
  if (state.mode==2) cout << gd.distTo;
  else cout << setprecision(1) << gps.f_course();

  // draw course/courseTo in 4 (angle has to be in degrees)
  if (state.mode<2) drawAngle(gps.f_course(),ST7735_CYAN);
  //angle = gps.f_course();// current bearing relative to N
  //courseTo has to be in [-179:180] and f_course in [0 359]
  else drawAngle(gd.courseTo-gps.f_course(),ST7735_CYAN);
  //angle = gd.courseTo - radians(gps.f_course());// bearing to next WP
  //drawAngle(angle,ST7735_YELLOW);

  // trip time in 6 (before 5 to avoid repeating setfill('0')
  prepContainer(111,72,42,ST7735_GREEN);
  //142 bytes
  cout << (int)gd.tr_hrs << ':' << setw(2) << (int)gd.tr_mins << ':' << setw(2) << (int)gd.tr_secs;

  // trip distance in 5
  prepContainer(7,72,36,ST7735_GREEN);
  cout << setfill(' ') << setw(4) << gd.tripdistance;

  // number of points saved/loaded in 11
  prepContainer(16,51,18,ST7735_YELLOW);
  cout << gd.nbPTS;

  // update label of button 2 in all modes
  tft.setCursor(63,115);
  prepContainer(63,115,30,ST7735_YELLOW);
  if (state.trip_on) cout << pstr("Pause");
  else if (state.mode==3) cout << pstr("No SD");
  else cout << pstr("Start");
}

//------------------------------- Info Screen Data ---------------------------------
void refreshInfoScreen(gpsData &gd){
  unsigned short failed = 0;
  gps.stats(NULL, NULL, &failed);

  // if no fix yet, replace invalid hdop value by -1
  unsigned long fix_error = gps.hdop();
  if (fix_error == TinyGPS::GPS_INVALID_HDOP) fix_error=-1;

  //print statistics data for status monitoring
  prepContainer(4,18,130,ST7735_GREEN);
  cout << gd.loc_age << ' ' << gps.satellites() << ' ' << fix_error << ' ' << failed;

  //display location data (84 bytes)
  prepContainer(4,38,130,ST7735_CYAN);
  cout << setprecision(6) << gd.lat << ',' << gd.lon; 

  // debug
  tft.setCursor(4,48);
  cout << "fr= "<< freeMemory();//Remaining RAM display = 
  // version
  tft.setCursor(4,58);
  cout << pstr("v") << pstr(version);
  
}

//----------------------------Prepare Container------------------------------------
void prepContainer(int x,int y, int length, uint16_t color){
  tft.fillRect(x,y,length,8,ST7735_BLACK);
  tft.setCursor(x,y);
  tft.setTextColor(color);
}

//----------------------- Draw a Line at an angle theta ----------------------------
// theta must be in degrees
// this function takes 546 bytes
void drawAngle(float theta, uint16_t color){
  theta = radians(theta);
  static int old_x2; 
  static int old_y2;
  int x2 = (int)(78+29*sin(theta));
  int y2 = (int)(52-29*cos(theta));
  if (old_x2>0)
    tft.drawLine(78,52,old_x2,old_y2,ST7735_BLACK);// erase previous line 
  tft.drawLine(78,52,x2,y2,color);
  old_x2=x2;
  old_y2=y2;
}

//----------------- Draw a circle ------------------------------------------------
// from wikipedia, algorithm 2, smaller than algo 1 used in Adafruit lib

void drawCircle(byte cx, byte cy, byte radius, uint16_t color){
  int error = -radius;
  byte x = radius;//try byte type later
  byte y = 0;//try byte type later

  while (x >= y){
    plot8points(cx, cy, x, y, color);
    error += y;
    ++y;
    error += y;
    if (error >= 0) {
      error -= x;
      --x;
      error -= x;
    }
  }
}

void plot8points(byte cx, byte cy, byte x, byte y, uint16_t color)
{
  plot4points(cx, cy, x, y, color);
  if (x != y) plot4points(cx, cy, y, x, color);
}

void plot4points(byte cx, byte cy, byte x, byte y, uint16_t color)
{
  tft.drawPixel(cx + x, cy + y,color);
  if (x != 0) tft.drawPixel(cx - x, cy + y,color);
  if (y != 0) tft.drawPixel(cx + x, cy - y,color);
  tft.drawPixel(cx - x, cy - y,color);
}

//----------------------------------------------------------------

