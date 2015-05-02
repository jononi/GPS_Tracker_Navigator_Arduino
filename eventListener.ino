// 1 | 2 | 3 --> left:1, center:2, right:3


//combine keypad polling and gps feeding in one function that can run for a consequent time (few 100's ms)
void eventListener(StateReg &state){ 
  //state.addWP=0; // reset addWP button flag
  //byte button=0; //pressed button token
  state.button = 0;
  //byte refstate=mcp_read1(MCP23008_GPIO);//reference state of GPIO, 0x1F if no button pressed
  byte refstate = (PINC & 0x07);//read PINC reg. for the actual state of pins
  byte nbSentences=2; //reset expected nb of nmea sentences
  uint32_t eventListenerTimer=millis(); //reset timer
  //--------------------------------------------------------------------------------------------
  //check Serial buffer content for at most 250 ms, or until two NMEA sentences are validated
  while ((millis()-eventListenerTimer < listenTimeOut) && nbSentences){
    while (Serial.available()){
      if(gps.encode(Serial.read())){
        state.newfix=1;
        nbSentences--;
      }  
    }
    //--------------------------------------------------
    // serial buffer empty, poll the keypad GPIO now
    //byte event=mcp_read1(MCP23008_GPIO);
    byte event=refstate^(PINC & 0x07);//check any change of state on pins
    // if a button press has been detected previously, don't go here again
    if(event && !state.button){
      state.button = 1;
      while(!(event&0x01)){
        // button not yet identified
        event=event>>1;//shift to the right one time
        state.button++;//next candidate
      }//while for button identifying
    //--------------------------------------------------
      //button 1 pressed
      if (state.button==1){
        //select mode screen
        if (state.screen==3 ) state.mode = 0;// enable tracking mode
        if (!state.screen){
          if (state.mode==0) state.addTP = 1;//manually add TP in T mode
          if (state.mode==1) state.addWP = 1;//manually add WP in E mode
          if (state.mode==2) state.nxtWP_en = 1+sw_thr;//switch to next WP in N mode
        }
            
      } //button 1 events

      if (state.button==2){
        //select mode screen
        if (state.screen==3 ) state.mode = 1;// enable exploration mode
        // main screen,  not the no sd mode: toggle trip data update in all modes
        if (!state.screen && state.mode<3) state.trip_on^=1; 
        // stats screen, any mode
        if (state.screen==1)  {
          state.screen = 0; //make main screen active
          mainScreenFrame(state);//display main screen
        }
      }//button 2 events

      if (state.button==3){
        //select mode screen: enable navigation mode
        if (state.screen==3 ) state.mode = 2;
        //main screen, any mode
        if (!state.screen) {
          state.screen=1;//make stats screen active
          infoScreenFrame();//display stats screen
        }
      }//button 3 events

      // reset Timer, to have an active debounce delay of 250ms
      eventListenerTimer=millis();
    } // done with buttons actions
  } // big while (another round if button pressed)
} //function

