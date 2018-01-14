

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

    DryRunCount = 0;
}

/*******************************************************************************************************************************************************************************/

//Configuration Class
ConfigureLib::ConfigureLib(int tanks)
{
   //Inititalize the number of sensors going to configure
    if(tanks > 0)
    {
      MaxTanks = tanks;
      MaxDryRunCount = 30;
    }
}

//Add Tank Details
bool ConfigureLib::AddTankDetails(String tankName,int no,bool primary,int height1,int height2,int onPoint,int OffPoint)
{
  bool result = false;

  if(tankName != "")
  {
    m_pTank[no] = new Tank(tankName,no,primary,height1,height2,onPoint,OffPoint);
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
    String tankName = "";

    if(m_pTank[tankNo]->IsPrimaryTank)
        tankName = "Sump";
    else
    {
        tankName = "Tank";
        tankName += String(tankNo - 1);
    }
          

  return tankName;
}

void ConfigureLib::ResetDryCount(int tankNo)
{
  if(m_pTank[tankNo])
    m_pTank[tankNo]->DryRunCount = 0;
}

void ConfigureLib::SetCurrentValue(int tankNo,float value)
{
  if(m_pTank[tankNo])
  {
    if(value <= m_pTank[tankNo]->currentValue)
      m_pTank[tankNo]->DryRunCount++;
    else
      m_pTank[tankNo]->DryRunCount = 0;
  
    m_pTank[tankNo]->currentValue = value;
  }
  
}

int ConfigureLib::GetDryCount(int tankNo)
{
  if(m_pTank[tankNo])
    return m_pTank[tankNo]->DryRunCount;
}

bool ConfigureLib::IsTankDry(int tankNo)
{
  bool result = false;

  if(m_pTank[tankNo])
  {
    if(m_pTank[tankNo]->DryRunCount >= MaxDryRunCount)
        result = true;
  }

  return result;
}

void ConfigureLib::SetMaxDryRunCount(int value)
{
  MaxDryRunCount = value;
}

