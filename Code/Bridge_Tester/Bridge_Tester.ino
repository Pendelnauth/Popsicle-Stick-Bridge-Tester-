/*
This code is to control the bridge crusher








notes:


*/

#include "SPI.h"               //display and 24 bit ADCs
#include "Adafruit_GFX.h"      //display
#include "Adafruit_ILI9341.h"  //display
#include <SingleFileDrive.h>   //logged data
#include <LittleFS.h>          //logged data - no longer used
#include <EEPROM.h>            //Setting Storage
#include "pico/mutex.h"

#include <PID_v1.h>
#include "RPi_Pico_TimerInterrupt.h"  //not used at the moment

//Motor Driver Pin Defs
#define IN1 19     //pwm1
#define IN2 20     //pwm2
#define NSleep 21  //motor controller sleep
#define NFault 23  //fault ouput from controller
#define SNSOut 22  //current chop active
#define IDrive 27  //Current Readout

//Display Pin Defs
#define Display_MISO 0  //display miso line
#define Display_CS 1    //display chip select
#define Display_SCK 2   //display clock line
#define Display_MOSI 3  //display mosi line
#define Display_RST 4   //display reset
#define Display_DC 26   //DC is used for data direction control
//Display Touch Screen
//digital pins only
#define Display_X_Plus 24  //used for touch screen
#define Display_Y_Plus 25  //used for touch screen
//analog pos readout
#define Display_X_Minus 28  //used for touch screen
#define Display_Y_Minus 29  //used for touch screen

//RGB LED
#define LED_Red 7
#define LED_Green 5
#define LED_Blue 6

//ADC
#define ADC_MISO 8
#define ADC_MOSI 11
#define ADC_SCK 10
#define ADC_CS0 14
#define ADC_CS1 12
#define ADC_IRQ0 13
#define ADC_IRQ1 9

//Screens
#define MENU 0
#define SETUP_MENU 1
#define PARAMETERS 2
#define TEST_MENU 3

//EEPROM Locations
#define EEPROM_SETUP_SPEED 0
#define EEPROM_SETUP_PRELOAD 1
#define EEPROM_SETUP_LOAD_RATE 2

bool Active = 1;
bool Change_Dir = 0;
uint8_t SPEED = 255;
bool DIR = 0;
uint16_t Current_Timer = 0;
uint16_t Touch_Counter = 0;
uint16_t Vboost = 0;
bool okayToWrite = true;
uint8_t Screen = MENU;  //

int Setup_Speed = 255;                             //just a range for the motor speed
int Setup_Preload = 5;                             //The real value is * .1 so if = 51 then 51 = 5.1
volatile int LoadL, LoadR;                         //The real value is * .1 so if = 51 then 51 = 5.1
volatile int Tare_LoadL = 370, Tare_LoadR = 1040;  //The real value is * .1 so if = 51 then 51 = 5.1
double Load_Rate = 5;
volatile double Working_LoadL = 0, Working_LoadR = 0, Pass_LoadL, Pass_LoadR;
volatile bool Load_Updatable = 1;

volatile bool ADC_Read_Value_Flag0 = 0, ADC_Read_Value_Flag1 = 0;
volatile bool ADC_Busy = 0;

volatile double Max_Load = 0;
volatile double Load = 0;
uint32_t Elapsed_Time = 0;  //second since test start
volatile bool Test_Active = 0;
volatile bool Test_State = 0;  //1 if finished



Adafruit_ILI9341 Display = Adafruit_ILI9341(Display_CS, Display_DC, Display_RST);

mutex_t myMutex;

void setup() {
  // put your setup code here, to run once:

  //15 right bar
  //28 top bar
  //27 left bar
  //26 bottom bar
  Serial.begin(115200);
  mutex_init(&myMutex);
  //LittleFS.begin();
  SPI.setRX(Display_MISO);
  SPI.setSCK(Display_SCK);
  SPI.setTX(Display_MOSI);

  Display.initSPI(63'000'000, SPI_MODE0);
  Display.begin();
  Display.setRotation(1);

  // Set up the USB disk share

  //singleFileDrive.onDelete(deleteCSV);
  //singleFileDrive.onPlug(plug);
  //singleFileDrive.onUnplug(unplug);
  //singleFileDrive.begin("data.csv", "Bridge Tester Data.csv");

  analogReadResolution(12);

  //Motor Controller Pin defs
  pinMode(IN1, OUTPUT_2MA);     //PWM0
  pinMode(IN2, OUTPUT_2MA);     //PWM1
  pinMode(NSleep, OUTPUT_2MA);  //place the Driver into Sleep mode when Low
  pinMode(NFault, INPUT);       //Fault Active low
  pinMode(SNSOut, INPUT);       //Current Chop Active
  pinMode(IDrive, INPUT);       //Current Readout from the Shunt amplifyer


  analogWriteFreq(50'000);

  //Settings
  EEPROM.begin(512);
  Setup_Speed = EEPROM.read(EEPROM_SETUP_SPEED);
  Setup_Preload = EEPROM.read(EEPROM_SETUP_PRELOAD);
  Load_Rate = float(EEPROM.read(EEPROM_SETUP_LOAD_RATE)) * 0.1;

  delay(5000);
}
uint8_t Register_location = 0;
void loop() {
  Main_Menu();
}

#define BUFFER_SIZE 20

double bufferL[BUFFER_SIZE], bufferR[BUFFER_SIZE];  // Array to hold the samples
int bufferIndex = 0;                                // Current index in the buffer
double newValueR = 0, newValueL = 0, sumR = 0, sumL;

#define PIN_INPUT 0
#define PIN_OUTPUT 25

//Define Variables we'll be connecting to
double Setpoint, Input, Output;

//Specify the links and initial tuning parameters
double Kp = 20, Ki = 4, Kd = 5;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);


void setup1() {

  //ADC
  SPI1.setRX(ADC_MISO);
  SPI1.setSCK(ADC_SCK);
  SPI1.setTX(ADC_MOSI);

  SPI1.begin();

  pinMode(ADC_CS0, OUTPUT_12MA);  //cs0 is top adc
  pinMode(ADC_CS1, OUTPUT_12MA);
  pinMode(ADC_IRQ0, INPUT);
  pinMode(ADC_IRQ1, INPUT);

  //RGB LED
  pinMode(LED_Red, OUTPUT_12MA);
  pinMode(LED_Green, OUTPUT_12MA);
  pinMode(LED_Blue, OUTPUT_12MA);

  digitalWrite(LED_Red, HIGH);
  digitalWrite(LED_Green, HIGH);
  digitalWrite(LED_Blue, HIGH);


  digitalWrite(ADC_CS0, HIGH);
  digitalWrite(ADC_CS1, HIGH);

  digitalWrite(ADC_CS0, LOW);
  SPI1.beginTransaction(SPISettings(20'000'000, MSBFIRST, SPI_MODE0));
  SPI1.transfer(0b01111000);  //rst register map
  SPI_Restart(ADC_CS0);
  SPI1.transfer(0b01000110);  //write register 0x01
  SPI1.transfer(0b11100000);  //set config0 internal ref and internal clk
  SPI_Restart(ADC_CS0);
  SPI1.transfer(0b01001010);  //write register 0x02
  SPI1.transfer(0b00000000);  //set config1 amclk = mclk osr 256
  SPI_Restart(ADC_CS0);
  SPI1.transfer(0b01001110);  //write register 0x03
  SPI1.transfer(0b10100001);  //set config2 gain to 8x hardware
  SPI_Restart(ADC_CS0);
  SPI1.endTransaction();
  digitalWrite(ADC_CS0, HIGH);

  digitalWrite(ADC_CS1, LOW);
  SPI1.beginTransaction(SPISettings(20'000'000, MSBFIRST, SPI_MODE0));
  SPI1.transfer(0b01111000);  //rst register map
  SPI_Restart(ADC_CS1);
  SPI1.transfer(0b01000110);  //write register 0x01
  SPI1.transfer(0b11100000);  //set config0 internal ref and internal clk
  SPI_Restart(ADC_CS1);
  SPI1.transfer(0b01001010);  //write register 0x02
  SPI1.transfer(0b00000000);  //set config1 amclk = mclk osr 256
  SPI_Restart(ADC_CS1);
  SPI1.transfer(0b01001110);  //write register 0x03
  SPI1.transfer(0b10100001);  //set config2 gain to 8x hardware
  SPI_Restart(ADC_CS1);
  SPI1.endTransaction();
  digitalWrite(ADC_CS1, HIGH);

  for (int i = 0; i < BUFFER_SIZE; i++) {
    bufferL[i] = 0;
    bufferR[i] = 0;
  }
  myPID.SetMode(AUTOMATIC);
}


int32_t total;
uint32_t ADC_READING;
uint32_t Load_Time = 0, Last_Load_Time = 0;
uint32_t Target_Load_Update_Timer = 0, Load_Restart_Timer = 0, Start_Time_Delay = 0;


int Motor_Speed = 0;
float Active_Load_Rate = 0;
float Last_Load = 0;

double Target_Load = 0;
bool First_Time = 1;

void loop1() {

  if (Test_Active && !Test_State) {  //the test is running Test_Active && !Test_State
    Load_Time = millis();

    if (Target_Load_Update_Timer <= millis()) {
      Target_Load += Load_Rate / 100.0;
      Target_Load_Update_Timer = millis() + 10;  //update 10 times per second
    }

    //if (First_Time) {
    //  First_Time = 0;
    //  Start_Time_Delay = millis() + 10000;  //5 second wait to start
    //}


    if (Target_Load < (Working_LoadR + Working_LoadL)) {  //Active_Load_Rate > (Load_Rate)
      if (!(Motor_Speed <= 0)) {
        Motor_Speed = 0;
      }
    } else if (Target_Load > (Working_LoadR + Working_LoadL)) {
      if (!(Motor_Speed == 255)) {

        Motor_Speed++;
        if(Working_LoadR + Working_LoadL <= 70){
          delay(10);
        }
      }
    }



    Serial.println(Motor_Speed);
    Last_Load_Time = Load_Time;
    Update_Motor(Motor_Speed, 1);
    Start_Motor();

    if ((Motor_Current() < 0.1)) {  //
      if (Current_Timer >= 1000) {  //1000 cycles that the current is below threshold prevents false readings
        DIR = !DIR;
        Current_Timer = 0;
        Target_Load = 0;

        Test_State = 1;
        Test_Active = 0;
        Motor_Speed = 0;
        First_Time = 1;

        Update_Motor(255, 0);  //move motor up
        delay(100);
        while (Motor_Current() > 0.5)
          ;
      }
      Current_Timer++;
    } else {
      Current_Timer = 0;
    }

    //Update_Motor(255, 1);//needs full duty cycle to read the current
    if ((Working_LoadR >= 440) || (Working_LoadL >= 440)) {  //motor has reached end of travel
      Test_State = 1;
      Test_Active = 0;
      Motor_Speed = 0;
      Target_Load = 0;
      First_Time = 1;
      Update_Motor(255, 0);  //move motor up
      delay(100);
      digitalWrite(LED_Red, LOW);  //Set Red to indicate max load has been reached
      while (Motor_Current() > 0.5)
        ;
      Update_Motor(0, 0);
      Stop_Motor();

    } else if ((Test_State && !Test_Active) && Screen == TEST_MENU) {  //end test
      Motor_Speed = 0;
      digitalWrite(LED_Green, LOW);  //Set Green Test has ended
      Update_Motor(0, 0);
      Stop_Motor();
    }
  } else if ((Test_State || !Test_Active) && Screen == TEST_MENU) {
    Load_Time = millis();
    Last_Load_Time = Load_Time;
    Update_Motor(0, 0);
    Stop_Motor();
  }

  if (!ADC_Busy) {
    delay(1);            //
    ADC_Take_Reading();  //gets the adc value from the adc register
  }

  if (ADC_Read_Value_Flag0 && ADC_Read_Value_Flag1) {

    total = Get_ADC_Register(0, 0);
    while (!Load_Updatable)
      ;
    Load_Updatable = 0;

    if (bitRead(total, 23)) {
      ADC_READING = (~((total) | (0b11111111000000000000000000000000))) + 1;
    } else {
      ADC_READING = total;
    }
    ADC_READING = ADC_READING * 1.17;  //Input impedance compensation two values because of the use of the use of the interal clk
    newValueR = (float(ADC_READING) / 1264);

    total = Get_ADC_Register(1, 0);

    if (bitRead(total, 23)) {
      ADC_READING = (~((total) | (0b11111111000000000000000000000000))) + 1;
    } else {
      ADC_READING = total;
    }
    ADC_READING = ADC_READING * 1.3714;  //Input impedance compensation two values because of the use of the use of the interal clk
    newValueL = (float(ADC_READING) / 1264);

    ADC_Read_Value_Flag0 = 0;
    ADC_Read_Value_Flag1 = 0;
    ADC_Busy = 0;  //clear ADC busy flag

    //LoadL = int(newValueL * 10);
    //LoadR = int(newValueR * 10);

    /////////////////////
    sumR -= bufferR[bufferIndex];      // Subtract the oldest value from the sum
    bufferR[bufferIndex] = newValueR;  // Replace the oldest value with the new value
    sumR += newValueR;                 // Add the new value to the sum

    sumL -= bufferL[bufferIndex];      // Subtract the oldest value from the sum
    bufferL[bufferIndex] = newValueL;  // Replace the oldest value with the new value
    sumL += newValueL;                 // Add the new value to the sum

    bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;

    //if(bufferIndex >= BUFFER_SIZE){
    //  bufferIndex = 0;
    //}else{
    //  bufferIndex++;
    //}

    // Calculate the rolling average
    Working_LoadR = sumR / BUFFER_SIZE;
    Working_LoadL = sumL / BUFFER_SIZE;

    mutex_enter_blocking(&myMutex);  // Lock the mutex

    Working_LoadR = Working_LoadR - (Tare_LoadR * 0.1);
    Working_LoadL = Working_LoadL - (Tare_LoadL * 0.1);

    Pass_LoadL = Working_LoadL;
    Pass_LoadR = Working_LoadR;
    mutex_exit(&myMutex);

    Load_Updatable = 1;
  }
}


//Main_Menu();

/*
  Display.fillScreen(ILI9341_BLACK);
  Display.setCursor(48, 5);
  Display.setTextColor(ILI9341_WHITE);
  Display.setTextSize(3);
  Display.print("Bridge Tester");
  Display.setTextSize(1);
  Display.setCursor(285, 2);
  Display.println("V1.00");

  Display.fillRoundRect(20, 170, 280, 50, 8, Display.color565(0, 120, 0));
  Display.setTextSize(3);
  Display.setCursor(115, 185);
  Display.setTextColor(Display.color565(255, 255, 255));
  Display.print("START");
  Active = 0;

  Display.fillRoundRect(200, 50, 50, 50, 8, ILI9341_GREEN);
  Display.fillRoundRect(200, 120, 50, 50, 8, ILI9341_RED);

  Display.setCursor(20, 60);
  Display.setTextColor(ILI9341_WHITE);
  Display.print(SPEED);
  */


/*
    if (Check_Box_Touch(200, 50, 50, 50, 1, 100)) {  //up button/Green
    if (SPEED >= 255) {
      SPEED = 0;
    } else {
      SPEED++;
    }
    Display.fillRoundRect(20, 30, 55, 55, 0, Display.color565(0, 0, 0));

    Display.setCursor(20, 60);
    Display.setTextColor(ILI9341_WHITE);
    Display.print(SPEED);
    if (Vboost >= 30) {
    } else if (Vboost >= 10) {
      delay(40);
    } else {
      delay(100);
      Vboost++;
    }
  } else if (Check_Box_Touch(200, 120, 50, 50, 1, 100)) {  //Down button/RED
    if (SPEED <= 0) {
      SPEED = 255;
    } else {
      SPEED--;
    }
    Display.fillRoundRect(20, 30, 55, 55, 0, Display.color565(0, 0, 0));
    Display.setCursor(20, 60);
    Display.setTextColor(ILI9341_WHITE);
    Display.print(SPEED);
    if (Vboost >= 30) {
    } else if (Vboost >= 10) {
      delay(40);
    } else {
      delay(100);
      Vboost++;
    }
  } else {
    Vboost = 0;
  }

  if (Check_Box_Touch(20, 170, 280, 50, 4, 50)) {  //checks if a touch has been detected for a given amount of time in a region
    //Serial.println("ADC Reading" + String(ADC_Take_Reading(0)));
    if (Active) {
      Stop_Motor();
      Display.fillRoundRect(20, 170, 280, 50, 8, Display.color565(0, 120, 0));
      Display.setTextSize(3);
      Display.setCursor(115, 185);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.print("START");
      Active = 0;
    } else {
      Start_Motor();
      Display.fillRoundRect(20, 170, 280, 50, 8, Display.color565(150, 0, 0));
      Display.setTextSize(3);
      Display.setCursor(125, 185);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.print("STOP");
      Active = 1;
    }
    delay(500);
  }

  if ((Motor_Current() < 0.1)) {  //
    if (Current_Timer >= 500) {   //500 cycles that the current is below threshold prevents false readings
      DIR = !DIR;
      Current_Timer = 0;
    }
    Current_Timer++;
  } else {
    Current_Timer = 0;
  }
  Update_Motor(SPEED, DIR);
  */


/*
  if (Check_Box_Touch(200, 50, 50, 50, 1, 100)) {  //up button/Green
    if (SPEED >= 255) {
      SPEED = 0;
    } else {
      SPEED++;
    }
    Display.fillRoundRect(20, 30, 55, 55, 0, Display.color565(0, 0, 0));

    Display.setCursor(20, 60);
    Display.setTextColor(ILI9341_WHITE);
    Display.print(SPEED);
    if (Vboost >= 30) {
    } else if (Vboost >= 10) {
      delay(40);
    } else {
      delay(100);
      Vboost++;
    }
  } else if (Check_Box_Touch(200, 120, 50, 50, 1, 100)) {  //Down button/RED
    if (SPEED <= 0) {
      SPEED = 255;
    } else {
      SPEED--;
    }
    Display.fillRoundRect(20, 30, 55, 55, 0, Display.color565(0, 0, 0));
    Display.setCursor(20, 60);
    Display.setTextColor(ILI9341_WHITE);
    Display.print(SPEED);
    if (Vboost >= 30) {
    } else if (Vboost >= 10) {
      delay(40);
    } else {
      delay(100);
      Vboost++;
    }
  } else {
    Vboost = 0;
  }

  if (Check_Box_Touch(20, 170, 280, 50, 4, 50)) {  //checks if a touch has been detected for a given amount of time in a region
    //Serial.println("ADC Reading" + String(ADC_Take_Reading(0)));
    if (Active) {
      Stop_Motor();
      Display.fillRoundRect(20, 170, 280, 50, 8, Display.color565(0, 120, 0));
      Display.setTextSize(3);
      Display.setCursor(115, 185);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.print("START");
      Active = 0;
    } else {
      Start_Motor();
      Display.fillRoundRect(20, 170, 280, 50, 8, Display.color565(150, 0, 0));
      Display.setTextSize(3);
      Display.setCursor(125, 185);
      Display.setTextColor(Display.color565(255, 255, 255));
      Display.print("STOP");
      Active = 1;
    }
    delay(500);
  }
  */


/*
  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));

  digitalWrite(ADC_CS1, LOW);
  SPI1.transfer(0b01111000);  //device rst
  SPI_Restart(ADC_CS1);
  SPI1.transfer(0b01001010);  //write register 0x02
  SPI1.transfer(0b00110000);  //set config1 amclk = mclk osr 256
  SPI_Restart(ADC_CS1);
  SPI1.transfer(0b01001110);  //write register 0x03
  SPI1.transfer(0b11001111);  //set config2 gain to 16x hardware
  SPI_Restart(ADC_CS1);
  digitalWrite(ADC_CS1, HIGH);
  SPI.endTransaction();

  SPI1.beginTransaction(SPISettings(15'000'000, MSBFIRST, SPI_MODE0));
  digitalWrite(ADC_CS0, LOW);
  SPI1.transfer(0b01111000);  //device rst
  SPI_Restart(ADC_CS0);
  SPI1.transfer(0b01001010);  //write register 0x02
  SPI1.transfer(0b00110000);  //set config1 amclk = mclk osr 256
  SPI_Restart(ADC_CS0);
  SPI1.transfer(0b01001110);  //write register 0x03
  SPI1.transfer(0b11001111);  //set config2 gain to 16x hardware
  SPI_Restart(ADC_CS0);
  digitalWrite(ADC_CS0, HIGH);
  SPI.endTransaction();
  */