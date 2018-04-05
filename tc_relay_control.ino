
#include <MAX31855.h>
#include <PID_v1.h>
#include "parameters.h"



void TC_Relay_Init()
{

  windowStartTime = millis();
  pinMode(TRIACPIN, OUTPUT);

  //initialize the variables we're linked to
  Setpoint = initialSetpoint;
  //x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);

  //tell the PID to range between 0 and 100 percent
  myPID.SetOutputLimits(0.0, 100.0);
  myPID.SetSampleTime(SampleInterval); //1 second updates
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

}


double GetSetpoint()
{
  return finalSetpoint;
}

void JumpSetpoint(double fSetPoint)
{
  initialTime = millis();
  windowStartTime = 0;
  //initialSetpoint = tempTC; //This really messes things up when you just want to change the rate!
  initialSetpoint = fSetPoint;
  finalSetpoint = fSetPoint;
  heating_rate = 0; //if we're jumping setpoint we're not ramping
}

void SetSetpoint(double fSetPoint)
{
  initialTime = millis();
  windowStartTime = 0;
  //initialSetpoint = tempTC; //This really messes things up when you just want to change the rate!
  initialSetpoint = Setpoint;
  finalSetpoint = fSetPoint;
}

double GetRate()
{
  initialTime = millis();
  windowStartTime = 0;
  //initialSetpoint = tempTC;
  initialSetpoint = Setpoint;
  return heating_rate;
}

void SetRate(double fRate)
{
  if(finalSetpoint > initialSetpoint)  
    heating_rate = fRate;
  else 
    heating_rate = -fRate;
}

void TC_Relay_Loop()
{
  unsigned long time_elapsed = millis() - initialTime;
  
  
    
  //Sample time up, update all variables
  if (time_elapsed - windowStartTime > SampleInterval)
  {
    //tic = micros();   
    //Calc ramp
    Setpoint = initialSetpoint + double(time_elapsed) * heating_rate / oneminute;
    //we've reached our goal, so stop ramping
    if ((Setpoint > finalSetpoint && heating_rate > 0) || (Setpoint < finalSetpoint && heating_rate < 0)) 
      {
      initialSetpoint = finalSetpoint;
      heating_rate=0;
      }
    
    windowStartTime += SampleInterval;
    //reportResult(Setpoint,tempTC,tempCJC,faultOpen,faultShortGND,faultShortVCC,Output,SampleInterval);
    double tempTCOld = tempTC, Temp1, Temp2, Temp3;


    do {
      
      x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);      
      while (tempTC == 9999) x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);
      Temp1 = tempTC;

      x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);
      Temp2 = tempTC;

      x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);
      Temp3 = tempTC;

    } while (abs(Temp1 - Temp2) > 1.0 || abs(Temp2 - Temp3) > 1.0); //repeat until the differences between the readings are below 1 degree
    
    tempTC = (Temp1 + Temp2 + Temp3) / 3.0; //average three readings together
    /*Serial.print(Temp1);
      Serial.print(' ');
      Serial.print(Temp2);
      Serial.print(' ');
      Serial.println(Temp3);*/
    //if(faultOpen | faultShortGND | faultShortVCC)
    //  tempTC = tempTCOld; //discard result if we have a blip, else the offscale value will mess up our PID (above filter should work)
    OutputPPrev = OutputP;
    myPID.Compute(); //this will update OutputP (0-100 output percent)
    if(round(OutputP) != round(OutputPPrev)) // Only restart algorithm on a power change
      cycleCount = 0; //this will force a restart of the algorithm on a power level change. This is handled in the interrupt!
    
    reportResult(Setpoint, tempTC, tempCJC, faultOpen, faultShortGND, faultShortVCC, OutputP, SampleInterval);
    
    if (killFlag == true)
    {
      if (tempTC >= finalSetpoint)
        killFlagCounter++;
      else
        killFlagCounter = 0;

      if (killFlagCounter > 10) //10 consecutive seconds of temperature required to kill power
      {
        finalSetpoint = 0;
        initialSetpoint = finalSetpoint;
        heating_rate=0;
        killFlagCounter = 0;
        killFlag = false;
        Serial.println("Reached Temp, killing heater");
      }
    }
    else
      killFlagCounter = 0;
  }
}
