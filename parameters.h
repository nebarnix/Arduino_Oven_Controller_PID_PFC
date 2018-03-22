#ifndef PARAMETERS_H
#define PARAMETERS_H

#define RELAY 11
#define minimumRelayTime 16 // don't waste throws of the switch
#define oneminute 60000 // 60,000 millis per minute
#define COMMAND_SIZE 128 //our command string length

#define TMIN 1 // min temp (C)
#define TMAX 1000 // max temp (C)
#define RMIN 0.1 // min temp rate (C)
#define RMAX 10000 // max temp rate (C)

//double aggKp=10, aggKi=0.05, aggKd=10;
//double aggKp=10, aggKi=.5, aggKd=10;
//double aggKp=1, aggKi=.05, aggKd=0;
//double aggKp=65, aggKi=0, aggKd=0;
//double aggKp=65.0, aggKi=0.1, aggKd=65.0/5.0;
//double aggKp=55.85, aggKi=0.240, aggKd=2152;
//double aggKp=5.585, aggKi=0.0240, aggKd=280; //D term way too noisey without filtering
//double aggKp=1.4561, aggKi=(aggKp/308.0), aggKd=(aggKp*20);
double aggKp=1.4561, aggKi=(aggKp/308.0), aggKd=(aggKp/5.0);
bool killFlag=0;


#endif
