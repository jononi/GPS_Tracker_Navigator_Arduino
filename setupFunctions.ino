void setupCommon(){
  DDRD |= _BV(6);// pin for display backlight enable (active high) 
  PORTD |= _BV(6);// enable backlight
  DDRC &= (_BV(0)|_BV(1)|_BV(2));//set A[0:2] as digital inputs
  PORTC |= (_BV(0)|_BV(1)|_BV(2));//activate pull-up res. on A[0:2]

  tft.initR();
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(orientation);
  tft.setTextColor(ST7735_GREEN);

  //GPS module
  Serial.begin(9600);//Hardware uart init at default baudrate for gps module
  Serial.println(PMTK_SET_NMEA_OUTPUT_RMCGGA);//delivers only RMC and GGA sentences...
  //Serial.println(PMTK_SET_NMEA_UPDATE_1HZ);  // at 1Hz update frequency
}

//----------------------------------- Mode Select ----------------------------------------------
void setupMode(){
  // first, display available modes labels
  tft.setCursor(10,10);
  cout << pstr("Select Mode:");
  tft.setCursor(5,54);
  cout << pstr("track");//alt: track
  tft.setCursor(57,54);
  cout << pstr("new");//alt: new trl
  tft.setCursor(109,54);
  cout << pstr("follow");//alt: use trl
  //this chunk is 118 bytes 
  tft.setCursor(5,64);
  cout << pstr("only");
  tft.setCursor(57,64);
  cout << pstr("trail");
  tft.setCursor(109,64);
  cout << pstr("trail");

  // second, draw a white rectangle around every option
  for(byte i=0; i <= 2; i++){
    int x = 52*i+3;//1st: 3, 2nd: 53, 3rd: 107
    tft.drawRect(x,52,40,22,ST7735_WHITE);
  }

  // then wait for a button press to select one of the modes
  state.button = 0;//reset state.button value to be safe
  while(!state.button){
    // waiting for any button press that will change state.mode to 0, 1 or 2
    eventListener(state);
  }

  // clear the rectangle around the non selected modes labels
  for(byte i=0; i <= 2; i++){
    int x = 52*i+3;//1st: 3, 2nd: 53, 3rd: 107
    if (i!=state.mode) tft.drawRect(x,52,40,22,ST7735_BLACK);
  }      
}

//---------------------------------- File Name select and opening -----------------------------------------------
void setupFile(StateReg *state, char *filename){
  byte append=0;//if 1 then will append to last file in T or E mode 

  //filename for T mode
  if (!state->mode) {
    filename[0]='T';
    filename[1]='R';
    filename[2]='K';
  }

  // display this menu for T and E modes only
  if (state->mode<2){
    // display new/append to file selection menu
    tft.setCursor(10,82);
    cout << pstr("File:");
    tft.setCursor(5,92);
    cout << pstr("Append");
    tft.setCursor(57,92);
    cout << pstr("New");

    state->button = 0;//reset state.button value to be safe
    //waiting for button 1 or 2 press
    while(!state->button) eventListener(*state);//repeat until a button is pressed
    if (state->button == 1) append = 1;//if button 1 pressed (right), append is enabled
    //else a new file is created
  }

  // display this menu for N mode only
  if (state->mode==2){
    tft.setCursor(5,92);
    cout << "Ok";
    tft.setCursor(57,92);
    cout << "Next";
  }


  // file name resolving
  for (byte i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    byte exist = SD.exists(filename);
    
    // if the name does exist and navigation mode active, display file name and
    // decide whether to open it or seek another one
    if (exist && state->mode==2) {
      //print filename
      prepContainer(3,82,72,ST7735_YELLOW);
      cout << filename;
      // wait for button
      state->button=0;
      while(!state->button) eventListener(*state);
      // if "Ok" pressed, open the file in read access and exit the loop
      if (state->button == 1) {
        sdFile.open(filename,ios::in);
        sdFile.ignore(20,'\n');// skip the first line that contains the header
        state->nxtWP_en = 1 + sw_thr;//enable loading 1st WP on 1st call of navigation func.
        break;//exit from for loop
      } 
    }
    
    // if the file with that name doesn't exist and tracking or exploration mode enabled...
    if(!exist && state->mode < 2) {
      uint8_t openmode = ios::out;
      if (append){  
        //...and append enabled, open previous file
        i--;
        filename[6] = i/10 + '0';
        filename[7] = i%10 + '0';
        openmode = ios::app;//open file in append mode
      }
      // open file
      sdFile.open(filename,openmode);
      if (!state->mode && !append) sdFile << pstr("type,time,lat,lon,alt,speed") << endl;//header line for trackpoints file
      if (state->mode==1 && !append) sdFile<<pstr("name,lat,lon")<<endl;//header line for waypoints file in new file only
      sdFile.flush();
      break;//exit from for loop
    }
    // when, in N mode, all filenames tested and no one selected: rewind
    if (i==99 && state->mode==2) i=-1;
  }
}


