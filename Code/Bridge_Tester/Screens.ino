

////////////////////////////////////////////////////////////////////////////////////////

void Main_Menu() {
  Display.fillScreen(0);  //clear screen

  //header
  Display.setCursor(48, 5);
  Display.setTextColor(ILI9341_WHITE);
  Display.setTextSize(3);
  Display.print("Bridge Tester");
  Display.setTextSize(1);
  Display.setCursor(285, 2);
  Display.println("V1.00");

  //Test Button
  Display.fillRoundRect(32, 170, 256, 50, 8, ILI9341_DARKGREEN);
  Display.setCursor(125, 185);
  Display.setTextSize(3);
  Display.setTextColor(ILI9341_WHITE);
  Display.print("TEST");

  //Setup Button
  Display.fillRoundRect(32, 40, 256, 50, 8, ILI9341_BLUE);
  Display.setCursor(115, 55);
  Display.setTextSize(3);
  Display.setTextColor(ILI9341_WHITE);
  Display.print("Setup");

  //Parameters Button
  Display.fillRoundRect(32, 105, 256, 50, 8, ILI9341_BLUE);
  Display.setCursor(75, 118);
  Display.setTextSize(3);
  Display.setTextColor(ILI9341_WHITE);
  Display.print("Parameters");

  uint8_t Touch_OBJ = 0;
  Screen = MENU;

  delay(100);

  bool Filled1 = 0, Filled2 = 0, Filled3 = 0;

  while (Screen == MENU) {
    if (Check_Box_Touch(32, 40, 256, 50, 5, 100)) {  //Check_Box_Touch(32, 40, 256, 50, 5, 100)
      if (!Filled1) {
        Display.fillRoundRect(10, 60, 12, 12, 6, Display.color565(0, 150, 50));
        Display.fillRoundRect(10, 190, 12, 12, 6, Display.color565(0, 0, 0));
        Display.fillRoundRect(10, 125, 12, 12, 6, Display.color565(0, 0, 0));
        Filled1 = 1;
        Filled2 = 0;
        Filled3 = 0;
      }
      Screen = SETUP_MENU;
      Setup_Menu();
    } else if (Check_Box_Touch(32, 105, 256, 50, 5, 100)) {  //Check_Box_Touch(32, 105, 256, 50, 5, 100)
      if (!Filled2) {
        Display.fillRoundRect(10, 125, 12, 12, 6, Display.color565(0, 150, 50));
        Display.fillRoundRect(10, 190, 12, 12, 6, Display.color565(0, 0, 0));
        Display.fillRoundRect(10, 60, 12, 12, 6, Display.color565(0, 0, 0));

        Filled1 = 0;
        Filled2 = 1;
        Filled3 = 0;
        Screen = PARAMETERS;
        Parameters_Menu();
      }
    } else if (Check_Box_Touch(32, 170, 256, 50, 5, 100)) {  //Check_Box_Touch(32, 170, 256, 50, 5, 100)
      if (!Filled3) {
        Display.fillRoundRect(10, 190, 12, 12, 6, Display.color565(0, 150, 50));
        Display.fillRoundRect(10, 60, 12, 12, 6, Display.color565(0, 0, 0));
        Display.fillRoundRect(10, 125, 12, 12, 6, Display.color565(0, 0, 0));
        Filled1 = 0;
        Filled2 = 0;
        Filled3 = 1;
      }
      Screen = TEST_MENU;
      Test_Menu();
    } else {
      Display.fillRoundRect(10, 170, 12, 12, 6, Display.color565(0, 0, 0));
      Display.fillRoundRect(10, 40, 12, 12, 6, Display.color565(0, 0, 0));
      Display.fillRoundRect(10, 105, 12, 12, 6, Display.color565(0, 0, 0));
    }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Setup_Menu() {
  Display.fillScreen(0);  //clear screen

  //back button
  Display.fillRoundRect(5, 5, 100, 40, 8, Display.color565(0, 0, 150));
  Display.setCursor(22, 18);
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.print("Back");

  //load text
  Display.setCursor(10, 50);
  Display.print("LoadL:");
  Display.setCursor(10, 80);
  Display.print("LoadR:");

  //up/+ button for changing values
  Display.fillRoundRect(235, 5, 80, 40, 8, Display.color565(0, 120, 0));
  Display.setCursor(265, 15);
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.println("+");

  //down/- button for changing values
  Display.fillRoundRect(145, 5, 80, 40, 8, Display.color565(0, 120, 0));
  Display.setCursor(175, 15);
  Display.println("-");

  //speed text
  Display.setCursor(10, 120);
  Display.print("Speed:" + String(Setup_Speed));

  //load set point text
  Display.setCursor(10, 150);
  Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");

  //motor up button
  Display.fillRoundRect(5, 185, 150, 50, 8, Display.color565(0, 120, 0));
  Display.setTextSize(2);
  Display.setCursor(55, 195);
  Display.print("MOVE");
  Display.setCursor(55, 215);
  Display.print("DOWN");

  //motor down button
  Display.fillRoundRect(165, 185, 150, 50, 8, Display.color565(0, 120, 0));
  Display.setCursor(215, 195);
  Display.print("MOVE");
  Display.setCursor(227, 215);
  Display.print("UP");

  //Tare load readout
  Display.fillRoundRect(235, 100, 80, 40, 8, Display.color565(0, 150, 0));
  Display.setCursor(245, 110);
  Display.setTextSize(2);
  Display.print("Tare");
  Display.setTextSize(3);

  uint8_t Last_Click = 0;  //0 = null
  uint8_t Vboost_Multiplier = 1;
  bool Stop_Down = 0;
  float Last_LoadL = 0, Last_LoadR = 0;  // used for removing the previous text from the screen
  uint8_t Last_Speed = Setup_Speed;
  uint8_t Last_Preload = Setup_Preload;
  uint32_t Update_Load = millis() + 100;       //every 100ms update display
  uint32_t Button_Retrigger = millis() + 100;  //at least 100ms to retrigger any button

  uint8_t Avg_Counter = 0;
  float Avg_Local_LoadL, Avg_Local_LoadR;

  while (Screen == SETUP_MENU) {

    mutex_enter_blocking(&myMutex);  // Lock the mutex
    float localLoadL = Pass_LoadL;
    float localLoadR = Pass_LoadR;
    mutex_exit(&myMutex);

    if (Check_Box_Touch(0, 0, 115, 45, 5, 100)) {  //back button
      Screen = 255;
      Stop_Motor();
    } else if (Check_Box_Touch(10, 120, 160, 33, 5, 100)) {  //Speed

      Display.setTextSize(3);
      Display.setTextColor(Display.color565(0, 150, 0));
      //speed text
      Display.setCursor(10, 120);
      Display.print("Speed:" + String(Setup_Speed));

      Display.setTextColor(Display.color565(255, 255, 255));
      //load set point text
      Display.setCursor(10, 150);
      Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");
      Last_Click = 1;  //last clicked speed

    } else if (Check_Box_Touch(10, 150, 298, 33, 5, 100)) {  //Preload

      Display.setTextSize(3);
      Display.setTextColor(Display.color565(255, 255, 255));
      //speed text
      Display.setCursor(10, 120);
      Display.print("Speed:" + String(Setup_Speed));
      Display.setTextColor(Display.color565(0, 150, 0));
      //load set point text
      Display.setCursor(10, 150);
      Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");
      Last_Click = 2;  //last clicked preload

    } else if (Check_Box_Touch(235, 5, 80, 40, 5, 100)) {  //+
      if (millis() >= Button_Retrigger) {
        if (Vboost >= 30) {
          Vboost_Multiplier = 3;
          //delay(5);
          Button_Retrigger = millis() + 5;
        } else if (Vboost >= 10) {
          Vboost_Multiplier = 3;
          Button_Retrigger = millis() + 50;
          //delay(50);
        } else {
          Vboost_Multiplier = 1;
          Button_Retrigger = millis() + 100;
          //delay(100);
        }
        Vboost++;

        switch (Last_Click) {
          case 1:
            if (!(Setup_Speed >= 255)) {
              Setup_Speed += Vboost_Multiplier;
              if (Setup_Speed >= 255) {
                Setup_Speed = 255;
              }
              Display.fillRect(113, 118, 77, 30, 0);
              Display.setTextSize(3);
              Display.setTextColor(Display.color565(0, 150, 0));
              //speed text
              Display.setCursor(10, 120);
              Display.print("Speed:" + String(Setup_Speed));
              Display.setTextColor(Display.color565(255, 255, 255));
              //load set point text
              Display.setCursor(10, 150);
              Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");
            }
            break;
          case 2:
            if (!(Setup_Preload >= 200)) {
              Setup_Preload += Vboost_Multiplier;
              if (Setup_Preload >= 200) {
                Setup_Preload = 200;
              }
              Display.fillRect(150, 148, 150, 30, 0);
              Display.setTextSize(3);
              Display.setTextColor(Display.color565(255, 255, 255));
              //speed text
              Display.setCursor(10, 120);
              Display.print("Speed:" + String(Setup_Speed));
              Display.setTextColor(Display.color565(0, 150, 0));
              //load set point text
              Display.setCursor(10, 150);
              Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");
            }
            break;
        }
      }
    } else if (Check_Box_Touch(145, 5, 80, 40, 5, 100)) {  //-

      if (millis() >= Button_Retrigger) {
        if (Vboost >= 30) {
          Vboost_Multiplier = 3;
          Button_Retrigger = millis() + 5;
          //delay(5);
        } else if (Vboost >= 10) {
          Vboost_Multiplier = 3;
          Button_Retrigger = millis() + 50;
          //delay(50);
        } else {
          Vboost_Multiplier = 1;
          Button_Retrigger = millis() + 100;
          //delay(100);
        }
        Vboost++;
        switch (Last_Click) {
          case 1:
            if (!(Setup_Speed <= 0)) {
              Setup_Speed -= Vboost_Multiplier;
              if (Setup_Speed <= 0) {
                Setup_Speed = 0;
              }
              Display.fillRect(113, 118, 77, 30, 0);
              Display.setTextSize(3);
              Display.setTextColor(Display.color565(0, 150, 0));
              //speed text
              Display.setCursor(10, 120);
              Display.print("Speed:" + String(Setup_Speed));
              Display.setTextColor(Display.color565(255, 255, 255));
              //load set point text
              Display.setCursor(10, 150);
              Display.print("Preload:" + String(float(Setup_Preload) / 10.0) + "LB");
            }
            break;
          case 2:
            if (!(Setup_Preload <= 0)) {
              Setup_Preload -= Vboost_Multiplier;
              if (Setup_Preload <= 0) {
                Setup_Preload = 0;
              }
              Display.fillRect(150, 148, 150, 30, 0);
              Display.setTextSize(3);
              Display.setTextColor(Display.color565(255, 255, 255));
              //speed text
              Display.setCursor(10, 120);
              Display.print("Speed:" + String(Setup_Speed));
              Display.setTextColor(Display.color565(0, 150, 0));
              //load set point text
              Display.setCursor(10, 150);
              Display.print("Preload:" + String(float(Setup_Preload) / 10.0, 1) + "LB");
            }
            break;
        }
      }
    } else if (Check_Box_Touch(165, 185, 150, 50, 5, 100)) {  //motor move up
      Start_Motor();
      Update_Motor(Setup_Speed, 0);
    } else if (Check_Box_Touch(5, 185, 150, 50, 5, 100) && !Stop_Down) {  //motor move down
      Start_Motor();
      Update_Motor(Setup_Speed, 1);
    } else if (Check_Box_Touch(235, 100, 80, 40, 5, 100) && millis() >= Button_Retrigger) {  //tare button

      mutex_enter_blocking(&myMutex);  // Lock the mutex
      Tare_LoadR = (localLoadR * 10) + Tare_LoadR;
      Tare_LoadL = (localLoadL * 10) + Tare_LoadL;
      mutex_exit(&myMutex);

      Button_Retrigger = millis() + 1000;
    } else {
      Stop_Motor();
      Vboost = 0;
    }

    if (localLoadR + localLoadL >= (Setup_Preload * 0.1)) {  //check over load value
      Stop_Down = 1;
    } else {
      Stop_Down = 0;
    }

    Avg_Local_LoadL += localLoadL;
    Avg_Local_LoadR += localLoadR;
    Load = localLoadL + localLoadR;

    Avg_Counter++;

    if (millis() >= Update_Load) {

      localLoadL = Avg_Local_LoadL / Avg_Counter;
      localLoadR = Avg_Local_LoadR / Avg_Counter;

      Avg_Counter = 0;
      Avg_Local_LoadL = 0;
      Avg_Local_LoadR = 0;

      Update_Load = millis() + 500;
      while (!Load_Updatable)
        ;
      Load_Updatable = 0;



      Display.setTextSize(3);
      Display.setTextColor(0);
      Display.setCursor(117, 50);
      Display.print(Last_LoadL, 1);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.setCursor(117, 50);
      Display.print(localLoadL, 1);

      Display.setTextSize(3);
      Display.setTextColor(0);
      Display.setCursor(117, 80);
      Display.print(Last_LoadR, 1);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.setCursor(117, 80);
      Display.print(localLoadR, 1);
      Last_LoadR = localLoadR;
      Last_LoadL = localLoadL;
      Load_Updatable = 1;
    }
  }
  //put value save point here//
  EEPROM.update(EEPROM_SETUP_SPEED, uint8_t(Setup_Speed));
  EEPROM.update(EEPROM_SETUP_PRELOAD, uint8_t(Setup_Preload));
  EEPROM.commit();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int Graph_Points[500][2];  //format sample number, x cord, y cord
int Graph_Cycle_Num = 0;
void Test_Menu() {

  Display.fillScreen(0);  //clear screen
  Display.fillScreen(0);  // Clear screen to black

  // Back button
  Display.fillRoundRect(5, 5, 100, 40, 8, Display.color565(0, 0, 150));
  Display.setCursor(22, 18);
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.print("Back");

  //graph background
  Display.fillRect(136, 5, 185, 230, ILI9341_WHITE);

  //vertical scale
  Display.setTextSize(1);
  Display.setTextColor(ILI9341_WHITE);
  Display.setCursor(112, 2);
  Display.print("880");
  Display.setCursor(112, 25);
  Display.print("792");
  Display.setCursor(112, 48);
  Display.print("704");
  Display.setCursor(112, 71);
  Display.print("616");
  Display.setCursor(112, 94);
  Display.print("528");
  Display.setCursor(112, 117);
  Display.print("440");
  Display.setCursor(112, 140);
  Display.print("352");
  Display.setCursor(112, 163);
  Display.print("264");
  Display.setCursor(112, 186);
  Display.print("176");
  Display.setCursor(112, 209);
  Display.print(" 88");
  Display.setCursor(112, 229);
  Display.print("  0");

  //horizontal lines
  Display.drawLine(136, 28, 320, 28, Display.color565(200, 200, 200));  //top most line
  Display.drawLine(136, 51, 320, 51, Display.color565(200, 200, 200));
  Display.drawLine(136, 74, 320, 74, Display.color565(200, 200, 200));
  Display.drawLine(136, 97, 320, 97, Display.color565(200, 200, 200));
  Display.drawLine(136, 120, 320, 120, Display.color565(200, 200, 200));
  Display.drawLine(136, 143, 320, 143, Display.color565(200, 200, 200));
  Display.drawLine(136, 166, 320, 166, Display.color565(200, 200, 200));
  Display.drawLine(136, 189, 320, 189, Display.color565(200, 200, 200));
  Display.drawLine(136, 212, 320, 212, Display.color565(200, 200, 200));
  Display.drawLine(136, 235, 320, 235, Display.color565(200, 200, 200));

  //Vertical lines
  Display.drawLine(159, 5, 159, 235, Display.color565(200, 200, 200));
  Display.drawLine(182, 5, 182, 235, Display.color565(200, 200, 200));
  Display.drawLine(205, 5, 205, 235, Display.color565(200, 200, 200));
  Display.drawLine(228, 5, 228, 235, Display.color565(200, 200, 200));
  Display.drawLine(251, 5, 251, 235, Display.color565(200, 200, 200));
  Display.drawLine(274, 5, 274, 235, Display.color565(200, 200, 200));
  Display.drawLine(297, 5, 297, 235, Display.color565(200, 200, 200));
  Display.drawLine(320, 5, 320, 235, Display.color565(20, 20, 20));

  // Max text
  Display.setTextSize(3);
  Display.setTextColor(ILI9341_WHITE);
  Display.setCursor(30, 60);  // Below rectangle + 10 margin
  Display.print("MAX");
  Display.setCursor(10, 88);
  Display.println(Max_Load, 1);
  Display.setCursor(10, 116);
  Display.println("LOAD");
  Display.setCursor(10, 144);
  Display.println(Load, 1);

  //play button
  Display.fillRoundRect(30, 170, 50, 50, 8, Display.color565(0, 150, 0));
  Display.fillTriangle(45, 180, 45, 210, 70, 195, Display.color565(70, 70, 70));


  uint32_t Button_Retrigger = millis() + 150;
  uint32_t Time_Step = millis() + 1000;
  uint32_t Update_Load = millis() + 500;
  float Load_Clear = Load, Max_Load_Clear = Max_Load;

  uint8_t Avg_Counter = 0;
  float Avg_Load;
  uint32_t Button_Hold_Time = millis() + 5000;
  ;
  bool Click_In_Line = 0;
  Elapsed_Time = 0;

  for (int counter = 1; counter < Graph_Cycle_Num; counter++) {
    Display.drawLine(Graph_Points[counter - 1][0], Graph_Points[counter - 1][1], Graph_Points[counter][0], Graph_Points[counter][1], Display.color565(200, 0, 0));
  }

  while (Screen == TEST_MENU) {
    mutex_enter_blocking(&myMutex);  // Lock the mutex
    float localLoadL = Pass_LoadL;
    float localLoadR = Pass_LoadR;
    mutex_exit(&myMutex);
    Load = localLoadL + localLoadR;

    if (Check_Box_Touch(0, 0, 115, 45, 5, 100)) {  //back button
      Screen = 255;
      Update_Motor(0, 0);
      Stop_Motor();
    } else if ((Check_Box_Touch(136, 5, 185, 230, 5, 100) || Check_Box_Touch(0, 60, 320, 180, 5, 100))) {  //start stop test

      Click_In_Line = 1;
      if (millis() >= Button_Hold_Time) {
        Graph_Cycle_Num = 0;
        Screen = 255;
        Max_Load = 0;
        Load = 0;
        Test_Active = 0;
        Test_State = 0;
        Target_Load = 0;
        First_Time = 1;
        digitalWrite(LED_Red, HIGH);  //Set Green Test has ended
        digitalWrite(LED_Green, HIGH);  //Set Green Test has ended
        digitalWrite(LED_Blue, HIGH);  //Set Green Test has ended
        Update_Motor(0, 0);
        Stop_Motor();
      }
    } else {
      if ((millis() >= Button_Retrigger) && Click_In_Line) {

        if (Test_Active && !Test_State) {
          Display.fillRoundRect(30, 170, 50, 50, 8, Display.color565(0, 150, 0));
          Display.fillTriangle(45, 180, 45, 210, 70, 195, Display.color565(70, 70, 70));
          Test_Active = 0;
        } else if (!Test_Active && !Test_State) {
          Display.fillRoundRect(30, 170, 50, 50, 8, Display.color565(150, 0, 0));
          Display.fillTriangle(45, 180, 45, 210, 70, 195, Display.color565(70, 70, 70));
          Test_Active = 1;
        }
        Button_Retrigger = millis() + 500;
      }
      Click_In_Line = 0;
      Button_Hold_Time = millis() + 5000;  //5 second restart time
    }
    if ((millis() >= Time_Step) && !Test_State && Test_Active) {  //test must not be finished and must be currently running
      //int diff;
      Elapsed_Time += 1;
      //diff = millis() - Time_Step;
      Time_Step = millis() + 1000;

      if (Graph_Cycle_Num == 0) {
        Display.drawLine(map(0, 0, (880 / Load_Rate), 136, 266), map(0, 880, 0, 5, 230), map(Elapsed_Time, 0, (880 / Load_Rate), 136, 266), map(Load, 880, 0, 5, 235), Display.color565(200, 0, 0));
      } else {
        Display.drawLine(Graph_Points[Graph_Cycle_Num - 1][0], Graph_Points[Graph_Cycle_Num - 1][1], map(Elapsed_Time, 0, (880 / Load_Rate), 136, 266), map(Load, 880, 0, 5, 235), Display.color565(200, 0, 0));
      }
      //Display.drawPixel(map(Elapsed_Time, 0, (880 / Load_Rate), 136, 266), map(Load, 880, 0, 5, 235), Display.color565(200, 0, 0));
      Graph_Points[Graph_Cycle_Num][0] = map(Elapsed_Time, 0, (880 / Load_Rate), 136, 266);  //185,230
      Graph_Points[Graph_Cycle_Num][1] = map(Load, 880, 0, 5, 230);                          //185,230
      Graph_Cycle_Num++;
    }
    //if(Load + 20 >= Max_Load){
    //  Load = Load_Clear;
    //}

    if (Max_Load < Load) {
      Max_Load = Load;
    }

    Avg_Counter++;
    Avg_Load += Load;

    if (millis() >= Update_Load) {
      Update_Load = millis() + 500;

      Load = Avg_Load / Avg_Counter;
      Avg_Load = 0;
      Avg_Counter = 0;

      Display.setTextColor(0);
      Display.setCursor(10, 88);
      Display.println(Max_Load_Clear, 1);

      Display.setTextColor(ILI9341_WHITE);
      Display.setCursor(10, 88);
      Display.println(Max_Load, 1);
      Max_Load_Clear = Max_Load;

      Display.setTextColor(0);
      Display.setCursor(10, 144);
      Display.println(Load_Clear, 1);

      Display.setTextColor(ILI9341_WHITE);
      Display.setCursor(10, 144);
      Display.println(Load, 1);
      Load_Clear = Load;
    }
  }
  Update_Motor(0, 0);
  Stop_Motor();
  Display.fillScreen(0);  //clear
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Cancel_Test_Popup() {  //dont clear screen leave background
}

bool Check_Box_Touch(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Touch_Length, uint16_t Touch_Screen_Sample_Num) {
  Touch_Counter = 0;

  while ((Touch_Screen_Sample_x(Touch_Screen_Sample_Num) > x && Touch_Screen_Sample_x(Touch_Screen_Sample_Num) < w + x && Touch_Screen_Sample_y(Touch_Screen_Sample_Num) > y && Touch_Screen_Sample_y(Touch_Screen_Sample_Num) < h + y)) {
    Touch_Counter++;
    if (Touch_Counter >= Touch_Length) {
      return 1;
    }
  }
  return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Parameters_Menu() {
  Display.fillScreen(0);  // clear screen

  // Back button
  Display.fillRoundRect(5, 5, 100, 40, 8, Display.color565(0, 0, 150));
  Display.setCursor(22, 18);
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.print("Back");

  // Load Rate text
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.setCursor(10, 50);
  Display.print("Load Rate:");

  // Load Rate value
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.print(String(Load_Rate, 1) + "lb/s");  // display Load_Rate value with 1 decimal place

  // Up (+)/Down (-) buttons
  Display.fillRoundRect(235, 5, 80, 40, 8, Display.color565(0, 120, 0));
  Display.setCursor(265, 15);
  Display.setTextSize(3);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.println("+");

  Display.fillRoundRect(145, 5, 80, 40, 8, Display.color565(0, 120, 0));
  Display.setCursor(175, 15);
  Display.println("-");

  uint8_t Last_Click = 0;  // 0 = not clicked, 1 = Load Rate
  uint32_t Button_Retrigger = 0;

  while (Screen == PARAMETERS) {
    if (Check_Box_Touch(0, 0, 115, 45, 5, 100)) {  // Back button
      Screen = 255;
    } else if (Check_Box_Touch(10, 50, 200, 33, 5, 100)) {  // Load Rate text
      Last_Click = 1;
      Display.setTextColor(Display.color565(0, 150, 0));
      Display.setCursor(48, 50);
      Display.fillRect(175, 50, 145, 30, 0);  // clear previous value
      Display.setCursor(10, 50);
      Display.print("Load Rate:");
      Display.print(String(Load_Rate, 1) + "lb/s");                           // display Load_Rate value with 1 decimal place
    } else if (Check_Box_Touch(235, 5, 80, 40, 5, 100) && Last_Click == 1) {  // Up (+) button
      if (millis() >= Button_Retrigger) {
        if (Load_Rate < 9.9) {
          Load_Rate += 0.1;
          if (Load_Rate >= 9.9) {
            Load_Rate = 9.9;
          }
          Display.setTextColor(Display.color565(0, 150, 0));
          Display.setCursor(48, 50);
          Display.fillRect(175, 50, 145, 30, 0);  // clear previous value
          Display.setCursor(10, 50);
          Display.print("Load Rate:");
          Display.print(String(Load_Rate, 1) + "lb/s");  // display Load_Rate value with 1 decimal place
        }
        Button_Retrigger = millis() + 150;
      }
    } else if (Check_Box_Touch(145, 5, 80, 40, 5, 100) && Last_Click == 1) {  // Down (-) button
      if (millis() >= Button_Retrigger) {
        if (Load_Rate > 2.0) {
          Load_Rate -= 0.1;
          if (Load_Rate < 2.0) {
            Load_Rate = 2.0;
          }
          Display.setTextColor(Display.color565(0, 150, 0));
          Display.setCursor(48, 50);
          Display.fillRect(175, 50, 145, 30, 0);  // clear previous value
          Display.setCursor(10, 50);
          Display.print("Load Rate:");
          Display.print(String(Load_Rate, 1) + "lb/s");  // display Load_Rate value with 1 decimal place
        }
        Button_Retrigger = millis() + 150;
      }
    }
  }
  // Clear screen before returning
  Display.fillScreen(0);
  //save any settings
  EEPROM.update(EEPROM_SETUP_LOAD_RATE, uint8_t(Load_Rate * 10));
  EEPROM.commit();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int Touch_Screen_Sample_x(uint16_t Sample_Number) {
  int Output;
  float Sample_Total = 0;
  int Last_Sample = 0;
  int Sample = 0;

  pinMode(Display_Y_Minus, INPUT);
  pinMode(Display_Y_Plus, INPUT);
  pinMode(Display_X_Minus, OUTPUT_12MA);
  pinMode(Display_X_Plus, OUTPUT_12MA);

  digitalWrite(Display_X_Plus, HIGH);
  digitalWrite(Display_X_Minus, LOW);

  Last_Sample = analogRead(Display_Y_Minus);  //could cause problems if this first sample is garbage
  for (uint16_t Counter = 0; Counter < Sample_Number; Counter++) {
    Sample = analogRead(Display_Y_Minus);
    if ((Sample >= 100 + Last_Sample) && (Sample <= 100 - Last_Sample)) {  //out of bounds sample reject
      Counter--;
    } else {
      Sample_Total += analogRead(Display_Y_Minus);
    }
  }

  Output = map(Sample_Total / float(Sample_Number), 427, 3230, 320, 0);
  if (Output < 0 || Output > 320) {
    Output = 0;
  }
  return Output;
}

int Touch_Screen_Sample_y(uint16_t Sample_Number) {
  int Output;
  float Sample_Total = 0;
  int Last_Sample = 0;
  int Sample = 0;
  
  pinMode(Display_X_Minus, INPUT);
  pinMode(Display_X_Plus, INPUT);
  pinMode(Display_Y_Minus, OUTPUT_12MA);
  pinMode(Display_Y_Plus, OUTPUT_12MA);

  digitalWrite(Display_Y_Plus, HIGH);
  digitalWrite(Display_Y_Minus, LOW);

  Last_Sample = analogRead(Display_X_Minus);  //could cause problems if this first sample is garbage
  for (uint16_t Counter = 0; Counter < Sample_Number; Counter++) {
    Sample = analogRead(Display_X_Minus);
    if ((Sample >= 100 + Last_Sample) && (Sample <= 100 - Last_Sample)) {  //out of bounds sample reject
      Counter--;
    } else {
      Sample_Total += analogRead(Display_X_Minus);
    }
  }

  Output = 240 - map(Sample_Total / float(Sample_Number), 300, 3300, 240, 0);
  if (Output < 0 || Output > 240) {
    Output = 0;
  }
  return Output;
}
