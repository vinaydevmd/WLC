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

//Pin to reset DryRun mode
const int dryRunPin = 180;
const int manualModePin = 190;

/**************************************************************************************************************/
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
const int MaxTankPercentage = 100;
const int MaxDryCount = 80;

//Global Variables and Objects
ConfigureLib *m_pConfigureLib;
int  TanksSelected = 0;
bool IsConfiguration = false;
bool EnableDebug = true;
bool SumpMotorExists  = false;
bool BoreMotorExists = false;
int  PrimaryTankNo = -1;
bool DryRun = false;
bool ActivateDryRun = false;
bool ActivateManualMode = false;

//Didplay LCD Message 
void DisplayLCDMessage(bool clearDisplay = true,int timeMs = 500, bool firstLineOFF = false,int c1 = 0 ,int r1 = 0 ,String messageRow1 = "" ,
                       bool secondLineOFF = false,int c2 = 0,int r2 = 0 ,String messageRow2 = "");
                      
void setup() {

    const String logFunc = "setup()";
    int maxDryCount = 30;
    // put your setup code here, to run once:
    
    //Initialization
    InitializeLCD();

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

    //Dry Run Pin
    pinMode(dryRunPin, INPUT);
    pinMode(manualModePin, INPUT);
    
    //Logging
    Serial.begin(9600); // Starts the serial communication

    //Read EEPROM to check data exists
    if(EEPROM.read(DataSetAddress) == 1)
    {                    
      DisplayLCDMessage(true,1000,false,0,0,"Data Loading. . ");

      LogSerial(true,logFunc,false,"Data exists in EEPROM");

      //Read number of tanks in EEPROM at DataAddress = 1;
      TanksSelected =  EEPROM.read(DataAddress);

      LogSerial(false,logFunc,false,"Tanks Selected : ");
      LogSerial(true,logFunc,true,String(TanksSelected));
     
      String tempMsg1 = "Tanks Selected:";
      tempMsg1 += String(TanksSelected);
       
      DisplayLCDMessage(true,1000,false,0,0,tempMsg1);

      //Initialize Configuration Library
      m_pConfigureLib = new ConfigureLib(TanksSelected);
  
      //Loop data address to retrive all tank details
      int address = DataAddress++;
      while( address <= MaxDataAddress)
      {
        //Read all tank details from EEPROM
        for(int tankCount = 1; tankCount <= TanksSelected; tankCount++)
        {
          int btmToFillHeight = 0;
          int fillToSensorHeight = 0;
          int onPoint = 0;
          int offPoint = 0;
          bool isPrimary = false;
                  
          String tankName = "";

          if(tankCount == PrimaryTankNo)
              tankName = "Sump";
          else
          {
              tankName = "Tank";
              tankName += String(tankCount - 1);
          }
          
          address++;
          isPrimary = EEPROM.read(address);
          
          address++;
          btmToFillHeight = EEPROM.read(address);
  
          address++;
          fillToSensorHeight = EEPROM.read(address);

          address++;
          onPoint = EEPROM.read(address);

          address++;
          offPoint = EEPROM.read(address);

          if(m_pConfigureLib)
          {    
             if(m_pConfigureLib->AddTankDetails(tankName,tankCount,isPrimary,btmToFillHeight,fillToSensorHeight,onPoint,offPoint))
             {
               if(EnableDebug)
                {
                  LogSerial(false,logFunc,false,String("Added "));
                  LogSerial(false,logFunc,true,tankName);
                  LogSerial(false,logFunc,true,",B2F : ");
                  LogSerial(false,logFunc,true,String(btmToFillHeight));
                  LogSerial(false,logFunc,true,",F2S : ");
                  LogSerial(true,logFunc,true,String(fillToSensorHeight));
                }
             }
          }
        }

        break;// once all tank details are read just break the loop
      }

      address++;
      SumpMotorExists = EEPROM.read(address);

      address++;
      BoreMotorExists = EEPROM.read(address);

      address++;
      maxDryCount = EEPROM.read(address);

      m_pConfigureLib->SetMaxDryRunCount(maxDryCount);

      //Display loaded data
      for(int i = 1; i <= TanksSelected; i++)
      {
        DisplayTankDetails(i);
        delay(1000);
      }

      DisplayLCDMessage(true,600,false,0,0,"Initializing. . .");
         
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
     const String logFunc = "loop()";
       
    //Incorporate Manual Mode in controller
  /*  if(digitalRead(manualModePin) == true)
    {
      ActivateManualMode = true;
      LogSerial(false,logFunc,false,String("Manual Mode Pin pressed"));
    }
    else
    {
      ActivateManualMode = false; 
    }

    //Check for Dry Pin
    if(digitalRead(dryRunPin) == true)
    {
      ActivateDryRun = false;
      LogSerial(false,logFunc,false,String("DryRun Pin pressed"));
      return;
    }*/
    
     //Check for reset pin to reset the configration settings
     if(digitalRead(keypadResetPin) == true)
     {

        LogSerial(false,logFunc,false,String("Reset Pin pressed"));
           
        EEPROM.write(DataSetAddress,0);
        
        for(int i = 1; i<= MaxDataAddress ; i++)
          EEPROM.write(i,0);
        
        DataAddress = 1;

        //Switch off all motors
        if(SumpMotorExists)
          digitalWrite(sumpMotorPin, LOW);
      
        if(BoreMotorExists)
          digitalWrite(boreMotorPin, LOW);
      
        
        //Configuration Setup for each tank with parameters required
        SetupConfiguration();

        return;
     }

     bool upperTankON = false;
     bool upperTankOFF = false;
     bool primaryTankFilled = false;
     bool dryRun = false;

     //Actual logic commenting for some time
     for(int tankCount = 1; tankCount <= TanksSelected ; tankCount++)
     {
        bool primary = false;
        String tankName = "";

        if(m_pConfigureLib)
        {
            primary = m_pConfigureLib->IsTankPrimary(tankCount);
            tankName = m_pConfigureLib->GetTankName(tankCount);
        }
            
        double tankDistance = GetTankStatus(tankCount);
   
        double tankHeight = 0;
        if(m_pConfigureLib)
          tankHeight =  m_pConfigureLib->GetTankFillHeight(tankCount);

        //Take value of
        int TankLevel = (100- ((tankDistance / tankHeight) * 100)) ;

        if(TankLevel > 100)
          TankLevel = 100;
        else if(TankLevel < 0 )
          TankLevel = 0;
  
        ShowTankStatusInLCD(tankName,tankDistance,round(TankLevel));

        if(ActivateDryRun)
          m_pConfigureLib->SetCurrentValue(tankCount,round(TankLevel));
        else
          m_pConfigureLib->ResetDryCount(tankCount);
          
        if(tankDistance > ErrorReading)
        {
        continue;
        }

        if(m_pConfigureLib)
        {
          int drycount = m_pConfigureLib->GetDryCount(tankCount);
          
           if(ActivateDryRun)
              dryRun = m_pConfigureLib->IsTankDry(tankCount);
      
          LogSerial(false,logFunc,false,"Dry Count:");
          LogSerial(true,logFunc,true,String(drycount));
          LogSerial(false,logFunc,false,"Dry Run:");
          LogSerial(true,logFunc,true,String(dryRun));
          
          if(dryRun)
              break;
        }
     
      //Check primary tank/Sump filled status
      if(primary)
      {
        if(TankLevel <= m_pConfigureLib->GetTankOnPercentage(tankCount))
            primaryTankFilled = false;
         else
            primaryTankFilled = true;
      }
      else
      {
        //Check T2,T3 full staus
        if(!upperTankON)
        {
          if(TankLevel <= m_pConfigureLib->GetTankOnPercentage(tankCount))
              upperTankON = true;
          else
              upperTankON = false;
        }
        
        if(TankLevel >= m_pConfigureLib->GetTankOFFPercentage(tankCount))
            upperTankOFF = true;
        else
            upperTankOFF = false;
      }
    
    }

    //Check for Manual Mode before operating motors
    if(!ActivateManualMode)
    {
      //This controls sump and borewell pins
      CoreControllerLogic(primaryTankFilled,upperTankON,upperTankOFF,dryRun);
    }
    else
    {
      DisplayLCDMessage(false,1200,true,0,0,"",false,0,1,"Manual Mode");
    }
  
}

//Initialize LCD with welcome message
void InitializeLCD()
{
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);
    
   //Welcome Message
   DisplayLCDMessage(true,800,false,0,0,"Welcome MyTools",3,1,"Controller");
}

float GetTankStatus(int tankNo)
{
    const String logFunc = "GetTankStatus()";
    
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
          LogSerial(true,logFunc,false,"Sensor error reading");
          return distance;
      }

       LogSerial(true,logFunc,true,String(distance));
     
      previousValue += distance;
    
      delay(200);
    }

     LogSerial(true,logFunc,true,String(distance));
     
    //After all tick counts take average of distance
    distance = previousValue / MaxTickCount;
    
    //Reduce actual distance with Fill to sensor distance , to get actual fill distance
    float f2SHeight = 0;
    if(m_pConfigureLib)
     f2SHeight =  m_pConfigureLib->GetFilltoSensorHeight(tankNo);
    
    if(distance > 0)
    {
      distance = distance - f2SHeight;
      
      LogSerial(false,logFunc,false,"Tank ");
      LogSerial(false,logFunc,true,String(tankNo));
      LogSerial(false,logFunc,true,": ");
      LogSerial(true,logFunc,true,String(distance));
    }
    
  return distance;
}

//Set up configuration by taking all tank height details
void SetupConfiguration()
{
    const String logFunc = "SetupConfiguration()";
    int maxDryCount = 30;

    //Get information about sump and bore motors
    DisplayLCDMessage(true,1000,false,0,0,"Sump Motor Exists:");
    SumpMotorExists = GetUserYesNoInput(7,1);
    
    DisplayLCDMessage(true,1000,false,0,0,"Bore Pump Exists:");
    BoreMotorExists = GetUserYesNoInput(7,1);

    //Get user Input for Dry Run Max Count
    DisplayLCDMessage(true,1000,false,0,0,"DryRun Timeout:");
    maxDryCount = GetUserInput(7,1,MaxDryCount,maxDryCount);
    delay(400);
    
    //Please Enter tanks to be configured
    DisplayLCDMessage(true,300,false,0,0,"Number of Tanks");

    TanksSelected = GetUserInput(7,1,MaxTanksSupported,0);

    String tempMsg1 = "Tanks Selected "; 
    tempMsg1 += String(TanksSelected);

    DisplayLCDMessage(true,600,false,0,0,tempMsg1);
    
    //Initialize Configuration Library
    m_pConfigureLib = new ConfigureLib(TanksSelected);

    if(m_pConfigureLib)
        m_pConfigureLib->SetMaxDryRunCount(maxDryCount);
    
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
    for(int tankCount = 1; tankCount <= TanksSelected; tankCount++)
    {
      int btmToFillHeight = 0;
      int fillToSensorHeight = 0;
      bool isPrimary = false;
      //Default on and off points
      int onPoint  = 30;
      int offPoint = 90;
            
      String tankName = "";

      if(PrimaryTankNo == -1)
      {
        DisplayLCDMessage(true,1000,false,0,0,"Is Sump:");
        isPrimary = GetUserYesNoInput(7,1);
      }

      if(isPrimary)
        PrimaryTankNo = tankCount;
      
      if(tankCount == PrimaryTankNo)
          tankName = "Sump";
      else
      {
          tankName = "Tank";
          tankName += String(tankCount - 1); 
          //tankName += ":"; 
      }
          
      String displayMsg = "";
      displayMsg  += tankName + ": " + "Ht in inch";

      DisplayLCDMessage(true,200,false,0,0,displayMsg);
    
      DisplayLCDMessage(false,200,false,0,0,displayMsg,false,0,1,"B2F(inch):");
      
     // delay(200);
      btmToFillHeight = GetUserInput(11,1,MaxTankHeight,0);

      DisplayLCDMessage(false,200,false,0,0,displayMsg,false,0,1,"F2S(inch):");
     
      fillToSensorHeight = GetUserInput(11,1,MaxTankHeight,0);
 
      //Get user Input for tank filling percentage of each tank ON Point
      displayMsg  = tankName + ": " + "ON (%)";

      DisplayLCDMessage(true,200,false,0,0,displayMsg);
       
      onPoint = GetUserInput(7,1,MaxTankPercentage,onPoint);
      delay(400);

      //Get user Input for tank filling percentage of each tank OFF Point
      displayMsg  = tankName + ": " + "OFF (%)";

      DisplayLCDMessage(true,200,false,0,0,displayMsg);
      offPoint = GetUserInput(7,1,MaxTankPercentage,offPoint);
      delay(400);
      
      //Log data to serial port for debugging
      LogSerial(false,logFunc,false,"btmToFillHeight : ");
      LogSerial(true,logFunc,true,String(btmToFillHeight));
      LogSerial(false,logFunc,false,"fillToSensorHeight : ");
      LogSerial(true,logFunc,true,String(fillToSensorHeight));
      LogSerial(false,logFunc,false,"On Point :");
      LogSerial(true,logFunc,true,String(onPoint));
      LogSerial(false,logFunc,false,"OFF Point :");
      LogSerial(true,logFunc,true,String(offPoint));
    
     if(tankCount == PrimaryTankNo)
        isPrimary = true;
           
      if(m_pConfigureLib)
      {    
         if(m_pConfigureLib->AddTankDetails(tankName,tankCount,isPrimary,btmToFillHeight,fillToSensorHeight,onPoint,offPoint))
         {
           if(EnableDebug)
            {
              LogSerial(false,logFunc,false,"Added : ");
              LogSerial(true,logFunc,true,tankName);
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

        //Write Tank ON Percent details
        DataAddress++;
        EEPROM.write(DataAddress,onPoint);

        //Write Tank OFF Percent details
        DataAddress++;
        EEPROM.write(DataAddress,offPoint);
            
      }
    }

    //Write Sump motor status
    DataAddress++;
    EEPROM.write(DataAddress,SumpMotorExists);

    //Write Bore pump status
    DataAddress++;
    EEPROM.write(DataAddress,BoreMotorExists);

    //Write Dry Count status
    DataAddress++;
    EEPROM.write(DataAddress,maxDryCount);

    
    DisplayLCDMessage(true,1500,false,0,0,"Configuration",false,0,1,"Complete");

    LogSerial(true,logFunc,false,"Configuration Complete!!");
  
    DisplayLCDMessage(true,500,false,0,0,"Please wait. . .");

  //Display configured tank details to user
    for(int i = 1; i <= TanksSelected; i++)
    {
      DisplayTankDetails(i);
      delay(1500);
    }

    //Display the status of sump and bore motors
    String tmpMsg1 = "";
    if(SumpMotorExists)
        tmpMsg1 = "Sump Motor: Yes";
    else
        tmpMsg1 = "Sump Motor: No";
      
    DisplayLCDMessage(true,1000,false,0,0,tmpMsg1);

    if(BoreMotorExists)
        tmpMsg1 = "Bore Pump: Yes";
    else
        tmpMsg1 = "Bore Pump: No";
        
    DisplayLCDMessage(true,1000,false,0,0,tmpMsg1);

    DisplayLCDMessage(true,500,false,0,0,"Initializing. . .");
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

   int okKeyState =  digitalRead(keypadOKPin);

   // Run while loop till user press enter key
   while(!okKeyState)
    {
        if(digitalRead(keypadOKPin) == true)
        {
          delay(500);
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

    // Run while loop till user press enter key    
    while(!okKeyState)
    {
       if(digitalRead(keypadOKPin) == true)
       {
         delay(600);
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

//Set tank status w.r.t value as signal level
void ShowTankStatusInLCD(String tankName,float val,int tanklevel)
{
  String tempMsg1 = "";
 
  if(val > ErrorReading)
  {
    tempMsg1 = tankName;
    tempMsg1 += ":";
    tempMsg1 += "Error !!";
    DisplayLCDMessage(false,1000,false,0,0,tempMsg1,true);
  }
  else
  {
    tempMsg1 = tankName;
    tempMsg1 += ": ";
    tempMsg1 += String(tanklevel);
    tempMsg1 += "%";   
     
    DisplayLCDMessage(false,2000,false,0,0,tempMsg1,true);
  }
  
}

void CoreControllerLogic(bool primaryTankFilled,bool upperTankON,bool upperTankOFF,bool dryRun)
{
  const String logFunc = "CoreControllerLogic()";

  if(dryRun)
  {
     LogSerial(true,logFunc,false,"Dry Run Activated !!");
     DisplayLCDMessage(false,1200,true,0,0,"",false,0,1,"Dry Run Detected !!");
  }

  if( (!upperTankON && upperTankOFF ) || dryRun)
  {
    //SUMP & BORE Motor OFF

    if(SumpMotorExists)
      digitalWrite(sumpMotorPin, LOW);
    
    if(BoreMotorExists)
      digitalWrite(boreMotorPin, LOW);
  
    DisplayLCDMessage(false,500,true,0,0,"",false,0,1,"Motors OFF");

  }
  else
  {
    if(primaryTankFilled)
    {
      if(upperTankON)
      {
        //SUMP MOTOR ON
        if(SumpMotorExists)
        {
          digitalWrite(sumpMotorPin, HIGH);
          DisplayLCDMessage(false,800,true,0,0,"",false,0,1,"Sump Motor ON");
          ActivateDryRun = true;
        }
        
        //Bore pump OFF
        if(BoreMotorExists)
        {
          digitalWrite(boreMotorPin, LOW);
          DisplayLCDMessage(false,800,true,0,0,"",false,0,1,"Bore Pump OFF");
        }
      }
    }
    else
    {
      if(upperTankON)
      {
        //SUMP Motor OFF
         if(SumpMotorExists)
         {
            digitalWrite(sumpMotorPin, LOW);
            DisplayLCDMessage(false,800,true,0,0,"",false,0,1,"Sump Motor OFF");
         }
        
       //Bore pump ON
        if(BoreMotorExists)
        {
          digitalWrite(boreMotorPin, HIGH);
          DisplayLCDMessage(false,800,true,0,0,"",false,0,1,"Bore Pump ON");
          ActivateDryRun = true;
        }
        
      }
      else if(upperTankOFF)
      {
         //Bore MOTOR OFF
          if(BoreMotorExists)
            digitalWrite(boreMotorPin, LOW);
  
         //SUMP MOTOR OFF
         if(SumpMotorExists)
            digitalWrite(sumpMotorPin, LOW);

         LogSerial(true,logFunc,false,"upperTankOFF");
 
         DisplayLCDMessage(false,800,true,0,0,"",false,0,1,"Motors OFF");

      }
    }
   
  }
     
}


//Display message to LCD
void DisplayLCDMessage(bool clearDisplay,int timeMs,bool firstLineOFF,int c1 ,int r1 ,String messageRow1 ,
                       bool secondLineOFF,int c2 ,int r2 ,String messageRow2)
{

    if(clearDisplay)
      lcd.clear();
    
    if(!firstLineOFF)
    { 
      for(int cur = 0; cur <= 16; cur ++)
      {
        lcd.setCursor(cur,0);
        lcd.print(" ");
      }
        
      lcd.setCursor(c1,r1);
      lcd.print(messageRow1);
    }
    
    if(!secondLineOFF)
    {
      for(int cur = 0; cur <= 16; cur ++)
      {
        lcd.setCursor(cur,1);
        lcd.print(" ");
      }
      
      lcd.setCursor(c2,r2);
      lcd.print(messageRow2);
    }
      
    delay(timeMs);
}

//Show Tank Details
void DisplayTankDetails(int no)
{
  if(m_pConfigureLib)
  {
      String message1;
      String message2;
      
      message1 = m_pConfigureLib->GetTankName(no);
      message1 += ":";
      message1 += " B2FHt:";
      message1 += String(m_pConfigureLib->GetBottomToFillHeight(no));
      
      message2 += "F2SHt:";
      message2 += String(m_pConfigureLib->GetFilltoSensorHeight(no));
       
      if(m_pConfigureLib->IsTankPrimary(no))
        message2 += " Prmry:Y";
      else
        message2 += " Prmry:N";
      
      DisplayLCDMessage(true,1000,false,0,0,message1,false,0,1,message2);
  }
  
}

void LogSerial(bool nextLine,String function,bool flow,String msg)
{
   if(EnableDebug)
   {
      if(!flow)
        msg = function + " : " + msg;
      
      if(nextLine)
        Serial.println(msg);
       else
        Serial.print(msg);
   }
}



