
#include "ConfigureLib.h"
#include <LiquidCrystal.h>

//EEPROM is to store values offline even when device is power off
#include <EEPROM.h>
#include <stdio.h>


//PINS

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = 12, en = 13, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 

//US- Sensor PIN Details (used for SUMP/TANK1 for demo)
//Primary Tank sensors
const int primaryTrigPin = 6;
const int primaryEchoPin = 7;

//US-Sensors of Tank2
const int tank2TrigPin = 8;
const int tank2EchoPin = 9;


//US-Sensors of Tank3
const int tank3TrigPin = 10;
const int tank3EchoPin = 11;


//SUMP and BORE Motor Output through LED 
const int sumpMotorPin = A4;
const int boreMotorPin = A5;

//Buzzer if any of the level goes down
const int buzzerPin = 0;


//Pins for Keypad Operation
const int keypadUpPin = A0;
const int keypadDownPin = A1;
const int keypadOKPin = A2;

//Configuration or Reset Setting PIN
const int keypadResetPin = A3;

//Error reading for valus in inches
const int ErrorReading = 1000;

//Constant address for EEPROM
const int MaxDataAddress = 100;
//Consecetive address location of Tank details to store in EEPROM
const int DataSetAddress = 0;
int DataAddress = 1;

//Constant Global Variables
const int  MaxTanksSupported = 4;//Numbers
const int MaxTankHeight = 100; // inches
const int MaxTickCount  = 5;
const int PrimaryTankNo = 0;
const int MaxTankPercentage = 100;

//Global Variables and Objects
ConfigureLib *m_pConfigureLib;
int  TanksSelected = 0;
bool IsConfiguration = false;
bool EnableDebug = true;
bool SumpMotorExists  = false;
bool BoreMotorExists = false;

//Levels of Each tank in percentage
float leve0 = 0; //zero level or 5%
float leve20 = 0;//20 % of the tank height
float leve50 = 0;//50% of tank height
float leve80 = 0;//80% of tank height
float leve100 = 0;//50% of tank height


//Didplay LCD Message 
void DisplayLCDMessage(bool clearDisplay = true,int timeMs = 500, int c1 = 0 ,int r1 = 0 ,String messageRow1 = "" ,
                       int c2 = 0,int r2 = 0 ,String messageRow2 = "");
                       
/*
Bar Graph Logic To display Tank filled status
*/
// To Create Characters for Bar Graph

byte NoLevel[8] = {
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000
};

byte Leve20[8] = {
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b11111,
 0b11111
};

byte Leve50[8] = {
 0b00000,
 0b00000,
 0b00000,
 0b00000,
 0b11111,
 0b11111,
 0b11111,
 0b11111
};

byte Leve80[8] = {
 0b00000,
 0b00000,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111
};

byte Leve100[8] = {
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111,
 0b11111
};



void setup() {
  
  // put your setup code here, to run once:

    //Initialization
    InitializeLCD();

    //Bar Graph Initialization
  /*  lcd.createChar(0, NoLevel);
    lcd.createChar(1, Leve20);
    lcd.createChar(2, Leve50);
    lcd.createChar(3, Leve80);
    lcd.createChar(4, Leve100);
    */

    
    //UltaSound Sensor pin for primary tank (sump / Tank1)
    pinMode(primaryTrigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(primaryEchoPin, INPUT); // Sets the echoPin as an Input

    //UltraSound sensor pin for tank2
    pinMode(tank2TrigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(tank2EchoPin, INPUT); // Sets the echoPin as an Input


    //UltraSound sensor pin for tank3
    pinMode(tank3TrigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(tank3EchoPin, INPUT); // Sets the echoPin as an Input


    //Motor Led's
    pinMode(sumpMotorPin, OUTPUT);
    pinMode(boreMotorPin, OUTPUT);

    //Buzer pin
    pinMode(buzzerPin, OUTPUT);

    //Keypad Pins
    pinMode(keypadUpPin, INPUT);
    pinMode(keypadDownPin, INPUT);
    pinMode(keypadResetPin, INPUT);
    pinMode(keypadOKPin, INPUT);
  
    //Logging
    Serial.begin(9600); // Starts the serial communication

    //Read EEPROM to check data exists
    if(EEPROM.read(DataSetAddress) == 1)
    {
    //  lcd.clear();
    //  lcd.setCursor(0,0);
     // lcd.print("Data Loading. . ");

//void DisplayLCDMessage(bool clearDisplay = false,int c1 = 0 ,int r1 = 0 ,String messageRow1 = "" ,
//                       int c2 = 0,int r2 = 0 ,String messageRow2 = "");
                       
      DisplayLCDMessage(true,1000,0,0,"Data Loading. . ");
      
     // delay(500);

      if(EnableDebug)
        Serial.println("Data exists in EEPROM");

      //Read number of tanks in EEPROM at DataAddress = 1;
      TanksSelected =  EEPROM.read(DataAddress);

       if(EnableDebug)
       {
          Serial.print("Tanks Selected : ");
          Serial.println(TanksSelected);
       }
      
      //display user number of tanks selected
      //lcd.clear();
      //lcd.setCursor(0,0);
     // lcd.print(TanksSelected);
      //lcd.println(" Tanks Selected");
      //delay(1000);

      char tempMsg1[LCD_CHAR_LENGTH];  
      sprintf(tempMsg1, "Tanks Selected:%d", TanksSelected); // send data to the buffer
      
      DisplayLCDMessage(true,1000,0,0,tempMsg1);
       
      
      //Initialize Configuration Library
      m_pConfigureLib = new ConfigureLib(TanksSelected,&lcd);
  
      //Loop data address to retrive all tank details
      int address = DataAddress++;
      while( address <= MaxDataAddress)
      {
        //Read all tank details from EEPROM
        for(int tankCount = 0; tankCount < TanksSelected; tankCount++)
        {
          int btmToFillHeight = 0;
          int fillToSensorHeight = 0;
          bool isPrimary = false;
          
          char tName[10] = "";

          if(tankCount == PrimaryTankNo)
            strcpy(tName,"Sump:");
          else
            strcpy(tName,"Tank%d:");
          
          String tankName = FormatIntMessage(tName,tankCount);
 
          address++;
          isPrimary = EEPROM.read(address);
          
          address++;
          btmToFillHeight = EEPROM.read(address);
  
          address++;
          fillToSensorHeight = EEPROM.read(address);

          if(EnableDebug)
          {
            Serial.println(tankName);
            Serial.println(btmToFillHeight);
            Serial.println(fillToSensorHeight);
          }
          
          if(m_pConfigureLib)
          {    
             if(m_pConfigureLib->AddTankDetails(tankName,tankCount,isPrimary,btmToFillHeight,fillToSensorHeight))
             {
               if(EnableDebug)
                {
                    Serial.print("Added ");
                    Serial.println(tankName);
                }
             }
          }
        }

        break;// once all tank details are read just break the loop
      }

      //Display loaded data
      for(int i = 0; i < TanksSelected; i++)
      {
        m_pConfigureLib->DisplayTankDetails(i);
        delay(1000);
      }

     // lcd.clear();
     //lcd.setCursor(0,0);
      //lcd.print("Initializing. . .");
      //delay(500);

      DisplayLCDMessage(true,600,0,0,"Initializing. . .");
         
    }
    else
    {
      //Configuration Setup for each tank with parameters required
      SetupConfiguration();
    }
    
}

// put your main code here, to run repeatedly:
void loop() 
{
    //Incorporate Manual Mode in controller
    
  
     //Check for reset pin to reset the configration settings
     if(digitalRead(keypadResetPin) == true)
     {
        if(EnableDebug)
          Serial.println("Reset Pin pressed");
       
        EEPROM.write(DataSetAddress,0);
        
         for(int i = 1; i<= MaxDataAddress ; i++)
            EEPROM.write(i,0);

        DataAddress = 1;
    
        //Configuration Setup for each tank with parameters required
        SetupConfiguration();
     }

     bool upperTankON = false;
     bool upperTankOFF = false;
     bool primaryTankFilled = false;

     //Actual logic commenting for some time
     for(int tankCount = 0; tankCount < TanksSelected ; tankCount++)
     {
        bool primary = false;

        if(m_pConfigureLib)
            primary = m_pConfigureLib->IsTankPrimary(tankCount);
            
        float tankDistance = GetTankStatus(tankCount);

       //if(m_pConfigureLib)
        //  m_pConfigureLib->SetTankFilledHeight(tankCount,tankDistance);

        char tName[10] = "";

        if(tankCount == PrimaryTankNo)
          strcpy(tName,"Sump:");
        else
          strcpy(tName,"Tank%d:");
            
        String tankName = FormatIntMessage(tName,tankCount);

        if(EnableDebug)
        {
          Serial.print("Tank Status :");
          Serial.print(tankName);
          Serial.println(tankDistance);
        }
        
        
        //String message1 = tankName;
        //String message2 = "";
        
        //char tempMsg[LCD_CHAR_LENGTH];
        //sprintf(tempMsg, "Filled :%.2f inch", tankStatus); // send data to the buffer
        
        //message2 = tankStatus;
       
        //Display count in 7th position of second row
        //m_pConfigureLib->DisplayLCDMessage(0,0,message1);

 
        float tankHeight = 0;
        if(m_pConfigureLib)
          tankHeight =  m_pConfigureLib->GetTankFillHeight(tankCount);

  /*
        //Calculate levels dynamically
        leve0 = (tankHeight *0.05); //zero level or 5%
        leve20 = (tankHeight * 0.20);//20 % of the tank height
        leve50 = (tankHeight * 0.50);//50% of tank height
        leve80 = (tankHeight * 0.80);//80% of tank height
        leve100 = tankHeight;//50% of tank height
        */

        
       int TankLevel = (tankDistance / tankHeight)*100 ;

       Serial.print("Level");
       Serial.println(TankLevel);
  
       ShowTankStatusInLCD(tankName,tankDistance,TankLevel);
       // delay(1000);
        
        if(tankDistance > ErrorReading)
        {
          continue;
        }

        //Check primary tank/Sump filled status
        if(primary)
        {
          if(tankDistance >= leve80 )//leve80
              primaryTankFilled = false;
           else
              primaryTankFilled = true;
        }
        else
        {
          //Check T2,T3 full staus
          if(!upperTankON)
          {
            if(tankDistance >= leve80)//leve80
                upperTankON = true;
            else
                upperTankON = false;
          }
          
          if(tankDistance <= leve20)
              upperTankOFF = true;
          else
              upperTankOFF = false;

        }
      
     }
     
     //This controls sump and borewell pins
     CoreControllerLogic(primaryTankFilled,upperTankON,upperTankOFF);
  
}

//Initialize LCD with welcome message
void InitializeLCD()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    //lcd.clear();
    
   
  /*  lcd.setCursor(0,0);
    lcd.print();
    lcd.setCursor(3,1); 
    lcd.print();
    delay(500);*/

   //Welcome Message
   DisplayLCDMessage(true,800,0,0,"Welcome MyTools",3,1,"Controller");
}

float GetTankStatus(int tankNo)
{
    float distance = 0;
    long duration = 0;
  
    int trigPin = 0;
    int echoPin = 0;
  
    switch(tankNo)
    {
      case 1: 
             trigPin = primaryTrigPin;
             echoPin = primaryEchoPin;
             break;
      case 2:
             trigPin = tank2TrigPin;
             echoPin = tank2EchoPin;
             break;
     case 3:
             trigPin = tank3TrigPin;
             echoPin = tank3EchoPin;
             break;
    }
  
  /*
    Serial.print("trigPin: ");
    Serial.println(trigPin);
    Serial.print("echoPin: ");
    Serial.println(echoPin);
    */   
  
      //Do 2- 3 ticks to confirm the reading before considering as final reading
      //To avoid error readings
    
      float previousValue = -1;
      
      for(int tickCount = 0; tickCount < MaxTickCount ; tickCount++)
      {
        // Clears the trigPin
        digitalWrite(trigPin, LOW);
        delayMicroseconds(2);
        
        // Sets the trigPin on HIGH state for 10 micro seconds
        digitalWrite(trigPin, HIGH);
        delayMicroseconds(100);
        digitalWrite(trigPin, LOW);
        
        // Reads the echoPin, returns the sound wave travel time in microseconds
        duration = pulseIn(echoPin, HIGH);
      
        // Calculating the distance
        distance= duration*0.034/2;
        
        //distance in inches
        distance = distance * 0.393701;
    
         //If distance > 0 like 3274849 then make
        if(distance > ErrorReading)
        {
            return distance;
        }
        
       // if(previousValue == -1)
          previousValue += distance;
       /* 
        if(previousValue != distance)
        {
           Serial.println("previous value != distance");
           tickCount = 0;
           previousValue = distance;
        }
        */
    
       /* if(EnableDebug)
        {
          Serial.print("Tank : ");
          Serial.println(tankNo);
          Serial.print("Previous Value: ");
          Serial.print(previousValue);
          Serial.print("distance: ");
          Serial.print(distance);
          Serial.print("Tick Count :");
          Serial.println(tickCount);
        }*/
      
        //Get Bottom to Fill height
       // float b2FHeight = 0;
       // if(m_pConfigureLib)
       //  b2FHeight =  m_pConfigureLib->GetTankFillHeight(tankNo);
        
    
         delay(100);
     }

     //After all tick counts take average of distance
     distance = previousValue / MaxTickCount;
  
   //Reduce actual distance with Fill to sensor distance , to get actual fill distance
      float f2SHeight = 0;
      if(m_pConfigureLib)
         f2SHeight =  m_pConfigureLib->GetFilltoSensorHeight(tankNo);
    
      if(distance > 0)
      {
        distance = distance - f2SHeight;
        Serial.print("distance > 0");
      }
  
      if(EnableDebug)
      {
        // Prints the distance on the Serial Monitor
        Serial.print("Distance: ");
        Serial.println(distance);
      }
    
   return distance;
}

//Set up configuration by taking all tank height details
void SetupConfiguration()
{
    //Please Enter tanks to be configured

    DisplayLCDMessage(true,300,0,0,"Number of Tanks");

    TanksSelected = GetUserInput(7,1,MaxTanksSupported,0);

    char tempMsg1[LCD_CHAR_LENGTH];  
    sprintf(tempMsg1,"Tanks Selected:%d",TanksSelected); // send data to the buffer

    DisplayLCDMessage(true,600,0,0,tempMsg1);

    if(EnableDebug)
    {
      Serial.print("Tank Selected :");
      Serial.println(TanksSelected);
    }
    
    //Initialize Configuration Library
    m_pConfigureLib = new ConfigureLib(TanksSelected,&lcd);
    
    //display user number of tanks selected
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print(TanksSelected);
    //lcd.println(" Tanks Selected");
    //delay(1000);

    if(EnableDebug)
      Serial.println("Configuration start for selected Tank count ");

    //Store all details at EEPROM 
    EEPROM.write(DataSetAddress,1); 
  
    //Wrie tank count
    EEPROM.write(DataAddress,TanksSelected); 

    //1. Change tank1 as sump by default and dont give acces to primary tank
    //2. Allow user to select only sump or has borewell
    //3. Allow peak hours tank fill option and set time
    //4. Set time not to switch on motor on unusual hours
    //5. Dry Run.
    
    // Take user input using loop for number of tanks selected
    for(int tankCount = 0; tankCount < TanksSelected; tankCount++)
    {
      int btmToFillHeight = 0;
      int fillToSensorHeight = 0;
      bool isPrimary = false;
      //Default on and off points
      int onPoint  = 30;
      int offPoint = 90;
      
      char tName[10] = "";
      
      if(tankCount == PrimaryTankNo)
        strcpy(tName,"Sump:");
      else
        strcpy(tName,"Tank%d:");
            
      String tankName = FormatIntMessage(tName,tankCount);
      String displayMsg = "";
      displayMsg  += tankName + "Ht in inch";

      if(EnableDebug)
        Serial.println(displayMsg);
      
      //lcd.clear();
      //lcd.setCursor(0,0);
     // lcd.print(displayMsg);

      DisplayLCDMessage(true,200,0,0,displayMsg);
      
      //lcd.setCursor(0,1);
      //lcd.print();

      DisplayLCDMessage(false,200,0,0,displayMsg,0,1,"B2F(inch):");
      
     // delay(200);
      btmToFillHeight = GetUserInput(11,1,MaxTankHeight,0);

     // lcd.setCursor(0,1);
     // lcd.print("F2S(inch):");
     // delay(200);
      DisplayLCDMessage(false,200,0,0,displayMsg,0,1,"F2S(inch):");
     
      fillToSensorHeight = GetUserInput(11,1,MaxTankHeight,0);
 

      //Get user Input for tank filling percentage of each tank ON Point
      //lcd.clear();
      //lcd.setCursor(0,0);
      displayMsg  = tankName + "ON (%)";
      //lcd.print(displayMsg);

      DisplayLCDMessage(true,200,0,0,displayMsg);
       
      onPoint = GetUserInput(7,1,MaxTankPercentage,onPoint);
      delay(400);

      //Get user Input for tank filling percentage of each tank OFF Point
      //lcd.clear();
      //lcd.setCursor(0,0);
      displayMsg  = tankName + "OFF (%)";
      //lcd.print(displayMsg);
      DisplayLCDMessage(true,200,0,0,displayMsg);
      offPoint = GetUserInput(7,1,MaxTankPercentage,offPoint);
      delay(400);
      
       if(EnableDebug)
      {
        Serial.print("btmToFillHeight : ");
        Serial.println(btmToFillHeight);
        Serial.print("fillToSensorHeight :");
        Serial.println(fillToSensorHeight);
        Serial.print("On Point :");
        Serial.println(onPoint);
        Serial.print("OFF Point :");
        Serial.println(offPoint);
      }
      

      //Get Yes or No boolean result
     /* lcd.clear();
      lcd.setCursor(0,0);
      displayMsg  = tankName + "Is Primary :";
      lcd.print(displayMsg);
      isPrimary = GetUserYesNoInput(7,1);
      delay(400);*/

     if(tankCount == PrimaryTankNo)
        isPrimary = true;
           
      if(m_pConfigureLib)
      {    
         if(m_pConfigureLib->AddTankDetails(tankName,tankCount,isPrimary,btmToFillHeight,fillToSensorHeight))
         {
           if(EnableDebug)
            {
              Serial.print("Added ");
              Serial.println(tankName);
            }
         }

        //Write Tank primary details
        DataAddress++;
        EEPROM.write(DataAddress,isPrimary); 


        //Write Tank btmToFillHeight details
        DataAddress++;
        EEPROM.write(DataAddress,btmToFillHeight); 

        //Write Tank fillToSensorHeight details
        DataAddress++;
        EEPROM.write(DataAddress,fillToSensorHeight);
             
      }
    }

    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print("Configuration");
    //lcd.setCursor(0,1);
    //lcd.print("Complete");
    //delay(1500);
     DisplayLCDMessage(true,1500,0,0,"Configuration",0,1,"Complete");

    if(EnableDebug)
      Serial.println("Configuration Complete!!");
    
    //lcd.clear();
    //lcd.setCursor(0,0);
    //lcd.print("Please wait. . .");
    //delay(500);
     DisplayLCDMessage(true,500,0,0,"Please wait. . .");

  //Display configured tank details to user
    for(int i = 0; i < TanksSelected; i++)
    {
      m_pConfigureLib->DisplayTankDetails(i);
      delay(1500);
    }

   /* lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Initializing. . .");
    delay(500);*/

    DisplayLCDMessage(true,500,0,0,"Initializing. . .");
}

//Get user input with respect to max value and display cursor in selected rows and column values
int GetUserInput(int col,int row,int maxValue,int initValue)
{
   int result = 0;
   int digit = 0;

   //Clear LCD having digits
   lcd.setCursor(col,row);
   lcd.print(" ");
   lcd.setCursor(col+1,row);
   lcd.print(" ");

   lcd.setCursor(col,row);  
   digit = initValue;

   //lcd.print(initValue);
   
   int okKeyState =  digitalRead(keypadOKPin);

   if(EnableDebug)
      Serial.println(okKeyState);
  
   // Run while loop till user press enter key
   while(!okKeyState)
    {
        if(digitalRead(keypadOKPin) == true)
        {
          delay(500);

          if(EnableDebug)
            Serial.println("Break if OK key press");
            
          break;
        }
        //Take first input
        else if(digitalRead(keypadUpPin) == true)
        {
          if(++digit > maxValue)
            digit = maxValue;
            
          delay(400);
         
          lcd.setCursor(col,row);     
          lcd.print(digit);  
        }
        else if(digitalRead(keypadDownPin) == true)
        {
            if(--digit < 0)
              digit = 0;
                       
            delay(400); 

            //Make sure second digit is cleared as and when it swithches back to single digit
            lcd.setCursor(col+1,row);
            lcd.print(" ");

            lcd.setCursor(col,row);
            lcd.print(digit);  
        }   
    }

    result = digit;

    if(EnableDebug)
      Serial.println(result);
   
  return result;
}

//Get user input interm of Yes or No and returns boolean value
bool GetUserYesNoInput(int col,int row)
{

   bool result = false;
   char value = {'Y'};
   
   //Clear LCD having digits
   lcd.setCursor(col,row);
   lcd.print(" ");

   lcd.setCursor(col+1,row);
   lcd.print(" ");
   
   int okKeyState =  digitalRead(keypadOKPin);

   if(EnableDebug)
      Serial.println(okKeyState);
  
    // Run while loop till user press enter key    
    while(!okKeyState)
    {
       if(digitalRead(keypadOKPin) == true)
       {
         delay(600);

         if(EnableDebug)
            Serial.println("Break if OK key press");
            
          break;
       }
      //Take first input
      else if(digitalRead(keypadUpPin) == true)
      {    
          value = 'Y';
          lcd.setCursor(col,row);     
          lcd.print(value);
          result = true;
      }
      else if(digitalRead(keypadDownPin) == true)
      {
          value = 'N';   
          lcd.setCursor(col,row);
          lcd.print(value);
          result = false;
      }
    }

    return result;
}

//Formats integer messages
String FormatIntMessage(char* msg,int value)
{
  String message = "";
  
  char tempMsg[LCD_CHAR_LENGTH];
  sprintf(tempMsg, msg, value); // send data to the buffer
      message = tempMsg;

  return message;
}


//Set tank status w.r.t value as signal level
void ShowTankStatusInLCD(String tankName,float val,int tanklevel)
{
  
  if(val > ErrorReading)
  {
    DisplayLCDMessage(true,1000,0,0,tankName,0,1,"Sensor Error !!");
    return;
  }
  else
  {
    char tempMsg1[LCD_CHAR_LENGTH];  
    char *tName = new char[tankName.length() + 1];
    strcpy(tName, tankName.c_str());
    
    // strcpy(tName,tankName);
    sprintf(tempMsg1, "% %d %s",tName,tanklevel,"%"); // send data to the buffer
    DisplayLCDMessage(true,1000,0,0,tempMsg1);

     if(EnableDebug)
      Serial.println(tempMsg1);
    
    delete [] tName;
  }
  
  /*
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(message1);


    if(val > ErrorReading)
    {
      lcd.setCursor(0,1);
      lcd.print("Sensor Error !!");
      return;
    }
    
    else
    {  
      lcd.setCursor(0, 1);
      lcd.print("LOW");
      lcd.setCursor(11, 1);
      lcd.print("HIGH");
    }
    */

/*
    Serial.print(leve0);
    Serial.print(leve20);
    Serial.print(leve50);
    Serial.print(leve80);
    Serial.print(leve100);
    Serial.print(val);
    */

/*
    //Set value for level10
    if(val >= leve100 ) // water level reaching to empty
    {
      lcd.setCursor(3, 1); 
      lcd.write(byte(0));
      lcd.setCursor(4, 1); 
      lcd.write(byte(0));
      lcd.setCursor(5, 1); 
      lcd.write(byte(0));
      lcd.setCursor(6, 1);
      lcd.write(byte(0));
      lcd.setCursor(7, 1);
      lcd.write(byte(0));
      lcd.setCursor(8, 1);
      lcd.write(byte(0));
      lcd.setCursor(9, 1);
      lcd.write(byte(0));
      lcd.setCursor(10, 1);
      lcd.write(byte(0));
      lcd.setCursor(0,0);

    }
    else if(val <= leve100 && val > leve80 ) //Water level reaches near to 20%
    {
      delay(50);
      lcd.setCursor(3, 1); 
      lcd.write(byte(1));
      lcd.setCursor(4, 1); 
      lcd.write(byte(1));
      lcd.setCursor(5, 1); 
      lcd.write(byte(0));
      lcd.setCursor(6, 1);
      lcd.write(byte(0));
      lcd.setCursor(7, 1);
      lcd.write(byte(0));
      lcd.setCursor(8, 1);
      lcd.write(byte(0));
      lcd.setCursor(9, 1);
      lcd.write(byte(0));
      lcd.setCursor(10, 1);
      lcd.write(byte(0));
      lcd.setCursor(0,0);
    }
    else if(val <= leve80 && val> leve50 ) //Water level is getting close to 50% of the the tank
    {
      lcd.setCursor(3, 1); 
      lcd.write(byte(1));
      lcd.setCursor(4, 1); 
      lcd.write(byte(1));
      lcd.setCursor(5, 1); 
      lcd.write(byte(2));
      lcd.setCursor(6, 1); 
      lcd.write(byte(2));
      lcd.setCursor(7, 1);
      lcd.write(byte(0));
      lcd.setCursor(8, 1);
      lcd.write(byte(0));
      lcd.setCursor(9, 1);
      lcd.write(byte(0));
      lcd.setCursor(10, 1);
      lcd.write(byte(0));
    }
    else if(val <= leve50 && val > leve20 ) //water level nearing to level 50%
    { 
      lcd.setCursor(3, 1); 
      lcd.write(byte(1));
      lcd.setCursor(4, 1); 
      lcd.write(byte(1));
      lcd.setCursor(5, 1); 
      lcd.write(byte(2));
      lcd.setCursor(6, 1); 
      lcd.write(byte(2));
      lcd.setCursor(7, 1);
      lcd.write(byte(3));
      lcd.setCursor(8, 1);
      lcd.write(byte(3));
      lcd.setCursor(9, 1);
      lcd.write(byte(0));
      lcd.setCursor(10, 1);
      lcd.write(byte(0));
    }
   else if( val <= leve20  ) //water level greater than 20 is nothing more than 80% fill
    {
      lcd.setCursor(3, 1); 
      lcd.write(byte(1));
      lcd.setCursor(4, 1); 
      lcd.write(byte(1));
      lcd.setCursor(5, 1); 
      lcd.write(byte(2));
      lcd.setCursor(6, 1); 
      lcd.write(byte(2));
      lcd.setCursor(7, 1);
      lcd.write(byte(3));
      lcd.setCursor(8, 1);
      lcd.write(byte(3));
      lcd.setCursor(9, 1);
      lcd.write(byte(4));
      lcd.setCursor(10, 1);
      lcd.write(byte(4));
  }

  */

  //delay(1000);
  
}

void CoreControllerLogic(bool primaryTankFilled,bool upperTankON,bool upperTankOFF)
{

 // Serial.println(primaryTankFilled);
  //Serial.println(upperTankON);
  //Serial.println(upperTankOFF);
  
  if(!upperTankON && upperTankOFF )
  {
    //SUMP & BORE Motor OFF
    digitalWrite(sumpMotorPin, LOW);
    digitalWrite(boreMotorPin, LOW);
  }
  else
  {
    if(primaryTankFilled)
    {
      if(upperTankON)
      {
        //SUMP MOTOR ON
        digitalWrite(sumpMotorPin, HIGH);
        //Bore pump OFF
        digitalWrite(boreMotorPin, LOW);
      }
    }
    else
    {
      if(upperTankON)
      {
        //SUMP Motor OFF
        digitalWrite(sumpMotorPin, LOW);
       //Bore pump ON
        digitalWrite(boreMotorPin, HIGH);
      }
      else if(upperTankOFF)
      {
         //Bore MOTOR OFF
         digitalWrite(boreMotorPin, LOW);

         //SUMP MOTOR OFF
         digitalWrite(sumpMotorPin, LOW);
      }
    }
   
  }
     
}


//Display message to LCD
void DisplayLCDMessage(bool clearDisplay,int timeMs, int c1 ,int r1 ,String messageRow1 ,
                       int c2 ,int r2 ,String messageRow2)
{

 if(clearDisplay)
    lcd.clear();
    
  lcd.setCursor(c1,r1);
  lcd.print(messageRow1);
  lcd.setCursor(c2,r2);
  lcd.print(messageRow2);
    
  delay(timeMs);
  
}




