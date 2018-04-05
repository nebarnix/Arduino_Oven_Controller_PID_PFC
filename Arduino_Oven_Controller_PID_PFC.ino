/*
  Control_TC_Serial

  Sets temperature ramp based on serial inputs.  At the very
  least, it should take 2 arguments (setpoint and slope), and
  change the temperature.

  Based on PID Relay example and TC Shield examples.
  PID Relay example is weakly commented.  Analog example
  is much better.  Minimum time is ok?

  // How long before integral is flushed?
  // I guess it's the "Window Size" parameter -- 5000 millis.

  This work is licensed under a Creative Commons Attribution-ShareAlike 3.0 Unported License.
  http://creativecommons.org/licenses/by-sa/3.0/
*/

#include "parameters.h"

//Use an interrupt to detect the pulses from the zero crossing detector. The width of the pulse will straddle the actual crossing.
//When using zero-crossing triggered SSRs, we can pick which pulse to trigger, but we must trigger with the zero crossing pulse
ISR (PCINT0_vect) // handle pin change interrupt for D8 to D13 here
{
  if ((*portInputRegister(zeroCrossPort) & zeroCrossBit) == zeroCrossBit && zeroCrossStatus == 0) //zeroCrossPin went low to high
  {
    //digitalWrite(13,!digitalRead(13));
    *portOutputRegister(LEDPort) ^= LEDBit; //toggle LEDBit with XOR
    if (cycleOnCount > cycleCount) //overflow detection, pulses on can't be greater than total pulses!
    {
      cycleCount = 1;
      cycleOnCount = 0; //this will pretty much force an ON the next cycle. But seems smoother than forcing an OFF. 
    }

    if (((100.0 * cycleOnCount) / cycleCount) < round(OutputP)) //power level is too low, trigger this time to increase
    {
      *portOutputRegister(TRIACPort) |= TRIACBit; //set TRIAC pin high to trigger
      cycleOnCount++;
    }
    else
      *portOutputRegister(TRIACPort) &= ~TRIACBit; //Don't trigger triac (this isn't really needed)
      
    zeroCrossStatus = zeroCrossBit; //Mark status for next go around
    cycleCount++; //keep track of total number of zero crossings
  }
  else if ((*portInputRegister(zeroCrossPort) & zeroCrossBit) == 0 && zeroCrossStatus == zeroCrossBit) //zeroCrossPin went high to low
  {
    *portOutputRegister(TRIACPort) &= ~TRIACBit; //Turn off TRIAC trigger
    zeroCrossStatus = 0; //update status for next time
  }
  else //State of pin 10 didn't change and pin didn't change. Just update the state and bail.
    zeroCrossStatus = zeroCrossPort & zeroCrossBit; //Update pin state
}

void setup() {
  Serial.begin(38400);
  Serial.println("M0 Sx.x Rx.x to ramp-to-hold");
  Serial.println("M1 Sx.x to jump");
  Serial.println("M2 Sx.x Rx.x to ramp-to-kill");
  Serial.println("M3 Pause PID and Hold power");
  Serial.println("M4 Resume PID");
  TC_Relay_Init();

  //Set up Zero Crossing Pin as input, pullup, and interrupt
  pinMode(13, OUTPUT);  //LED is an output
  pinMode(ZEROCROSSPIN, INPUT);
  digitalWrite(ZEROCROSSPIN, HIGH); // enable pullup resistor

  zeroCrossBit = digitalPinToBitMask(ZEROCROSSPIN); //Set the bit for future use
  zeroCrossPort = digitalPinToPort(ZEROCROSSPIN);  //Set the port for future use
  TRIACBit = digitalPinToBitMask(TRIACPIN); //Set the bit for future use
  TRIACPort = digitalPinToPort(TRIACPIN);  //Set the port for future use

  LEDBit = digitalPinToBitMask(13); //Set the bit for future use
  LEDPort = digitalPinToPort(13);  //Set the port for future use
  
  noInterrupts();           // disable all interrupts
  //Setup Pin Interrupts
  *digitalPinToPCMSK(ZEROCROSSPIN) |= bit (digitalPinToPCMSKbit(ZEROCROSSPIN));  // enable pin
  PCIFR  |= bit (digitalPinToPCICRbit(ZEROCROSSPIN)); // clear any outstanding interrupt
  PCICR  |= bit (digitalPinToPCICRbit(ZEROCROSSPIN)); // enable interrupt for the group
  interrupts();             // enable all interrupts
}

void loop() {
  //unsigned long tic=micros(),toc;
  get_and_do_command();
  TC_Relay_Loop();
  //toc = micros();
  //Serial.println(toc-tic);
}

