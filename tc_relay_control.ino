
#include <MAX31855.h>
#include <PID_v1.h>
#include "parameters.h"



double initialSetpoint = 0.0; // initial
double finalSetpoint = 0.0; // goal
double heating_rate = 0; // degree per minute
unsigned long initialTime = 0;

//Define Variables we'll be connecting to
double Setpoint, Output, OutputP;
double tempTC, tempCJC;
bool faultOpen, faultShortGND, faultShortVCC, x;

//Specify the links and initial tuning parameters
//Define the aggressive and conservative Tuning Parameters
//double aggKp=400, aggKi=0.1, aggKd=50;
//double aggKp=10, aggKi=0.05, aggKd=10;
// Each degree centigrade as one point towards the decision
// to turn on or off.  Kp scales this up toward 5000.
//double consKp=1, consKi=0.05, consKd=0;
PID myPID(&tempTC, &OutputP, &Setpoint, aggKp, aggKi, aggKd, DIRECT);

// Time window of 5000 milliseconds?
int killFlagCounter=0,WindowSize = 1000;
unsigned long windowStartTime; // each new increment

//MAX31855 temp(3, 4, 5); // measure and fault pins
MAX31855 temp(5, 4, 3); // measure and fault pins

void TC_Relay_Init()
{
  
  windowStartTime = millis();
  pinMode(RELAY,OUTPUT);
  
  //initialize the variables we're linked to
  Setpoint = initialSetpoint;
  //x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);

  //tell the PID to range between 0 and 100 percent
  myPID.SetOutputLimits(0.0, 100.0);
  myPID.SetSampleTime(1000); //1 second updates
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  
}


double GetSetpoint()
{
  return finalSetpoint;
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
  heating_rate = fRate;
}

void TC_Relay_Loop()
{
    unsigned long time_elapsed = millis() - initialTime;
  
    Setpoint = initialSetpoint + double(time_elapsed) * heating_rate / oneminute;
    
    if (Setpoint > finalSetpoint) 
    {
        Setpoint = finalSetpoint;
    }
  
    /************************************************
     * turn the output pin on/off based on pid output
     ************************************************/
    if(time_elapsed - windowStartTime > WindowSize)
    { //time to shift the Relay Window
        windowStartTime += WindowSize;
        //reportResult(Setpoint,tempTC,tempCJC,faultOpen,faultShortGND,faultShortVCC,Output,WindowSize);        
        double tempTCOld = tempTC;
        x = temp.readMAX31855(&tempTC, &tempCJC, &faultOpen, &faultShortGND, &faultShortVCC);
        
        if(faultOpen | faultShortGND | faultShortVCC)
           tempTC = tempTCOld; //discard result if we have a blip, else the offscale value will mess up our PID
        
        myPID.Compute(); //don't let the output change mid-pulse!
        Output = (OutputP/100.0)*WindowSize; //convert percentage to window size upon compute
        reportResult(Setpoint,tempTC,tempCJC,faultOpen,faultShortGND,faultShortVCC,Output,WindowSize);
        if(killFlag == true)
           {
           if(tempTC >= finalSetpoint)
              killFlagCounter++;
           else
              killFlagCounter = 0;
           
           if(killFlagCounter > 10) //10 consecutive seconds of temperature required to kill power
              {
              finalSetpoint = 0;
              killFlagCounter = 0;
              Serial.println("Reached Temp, killing heater");
              }
           }
         else
            killFlagCounter = 0;
           
    }
    
    if(Output > time_elapsed - windowStartTime)
    { // if we're in the "on" part of the duty cycle
        if(Output > minimumRelayTime)
        { // check to see if it's worth turning on.
            digitalWrite(RELAY,HIGH);
        }
        //reportResult(Setpoint,tempTC,tempCJC,faultOpen,faultShortGND,faultShortVCC,Output,WindowSize);
        //delay(minimumRelayTime);
        //reportResult(Setpoint,tempTC,tempCJC,faultOpen,faultShortGND,faultShortVCC,Output,WindowSize);
    } 
    else digitalWrite(RELAY,LOW);
}
