#ifndef configure
#define configure


//Check Aurdion library

#if(ARDUINO >= 100)
  #include "Arduino.h"
#else
  #include "wProgram.h"
#endif

#define MaxTankCount  5 


//Class Tank to keep all tank details
class Tank
{
  public:
      Tank(String tankName,int no,bool primary,int height1,int height2,int onPoint = 30,int offPoint = 90);
        
      //Store Tank details in class
      String TankName; // Tank Name
      int    TankNo; //Tank No details
      bool   IsPrimaryTank; //Nothing but Sump
      int    Height; //In inches
      int    BottomToFillHeight;
      int    FillToSensorHeight;
      float  FilledHeight;
      int    TankONPoint;
      int    TankOFFPoint;
      
 };


//Configuration class
class ConfigureLib
{

  public:
      //Constructor
      ConfigureLib(int sensors);
      
      //Variables
      int MaxTanks;
      
      //Methods
      bool AddTankDetails(String tankName,int no,bool primary,int height1,int height2,int onPoint,int OffPoint);

      bool IsTankPrimary(int tankNo);

      int GetTankFillHeight(int tankNo);

      int GetFilltoSensorHeight(int tankNo);

      int GetBottomToFillHeight(int tankNo);

      int GetTankOnPercentage(int tankNo);

      int GetTankOFFPercentage(int tankNo);

      String GetTankName(int tankNo);

private:
      Tank *m_pTank[MaxTankCount];

};

#endif
