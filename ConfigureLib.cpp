

// include the library code:
#include "ConfigureLib.h"

//Tank Class
//Default keep Motor on point to 30% and Motor OFF Point to 90%
Tank::Tank(String tankName,int no,bool primary,int height1,int height2,int onPoint = 30,int offPoint = 90) 
{
  
    TankName = tankName;
    TankNo = no;
    IsPrimaryTank = primary;
    BottomToFillHeight = height1;
    FillToSensorHeight = height2;

    Height = BottomToFillHeight + FillToSensorHeight;

    TankONPoint = onPoint;
    TankOFFPoint = offPoint;
}

/*******************************************************************************************************************************************************************************/

//Configuration Class
ConfigureLib::ConfigureLib(int tanks)
{
   //Inititalize the number of sensors going to configure
    if(tanks > 0)
    {
      MaxTanks = tanks;
    }
}

//Add Tank Details
bool ConfigureLib::AddTankDetails(String tankName,int no,bool primary,int height1,int height2)
{
  bool result = false;

  if(tankName != "")
  {
    m_pTank[no] = new Tank(tankName,no,primary,height1,height2);
    result = true;
  }
     
  return result;
}

//Get tank primary details
 bool ConfigureLib::IsTankPrimary(int tankNo)
 {
  if(m_pTank[tankNo])
     return  m_pTank[tankNo]->IsPrimaryTank;

  return false;
 }

//Get Tank fill height
 int ConfigureLib::GetTankFillHeight(int tankNo)
{
   int height = 0;
   
   if(m_pTank[tankNo])
     height = m_pTank[tankNo]->BottomToFillHeight;

  return height;
}

int ConfigureLib::GetFilltoSensorHeight(int tankNo)
{
  int height = 0;
   
   if(m_pTank[tankNo])
     height = m_pTank[tankNo]->FillToSensorHeight;

  return height;  
}

int ConfigureLib::GetBottomToFillHeight(int tankNo)
{
  int height = 0;
   
   if(m_pTank[tankNo])
     height = m_pTank[tankNo]->BottomToFillHeight;

  return height;  
}

//Get tank primary details
 int ConfigureLib::GetTankOnPercentage(int tankNo)
 {
  if(m_pTank[tankNo])
     return  m_pTank[tankNo]->TankONPoint;

  return 0;
 }

 //Get tank primary details
 int ConfigureLib::GetTankOFFPercentage(int tankNo)
 {
  if(m_pTank[tankNo])
     return  m_pTank[tankNo]->TankOFFPoint;

  return 0;
 }


String ConfigureLib::GetTankName(int tankNo)
{
  if(m_pTank[tankNo])
   return  m_pTank[tankNo]->TankName;

  return "";
}

