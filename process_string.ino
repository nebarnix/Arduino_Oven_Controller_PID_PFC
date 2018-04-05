/********************************************************
   process_string

   Just like in RepRap, I want functions to parse and use
   the serial commands.  Serial commands are delivered byte-
   by-byte, so I'd like to steal my well-tested "arduino-gcode"
   CNC control parser.  CNC Controller has been working well
   for almost 2 years now.
 *******************************************************/

#include "parameters.h"



/* gcode line parse results */
struct GcodeParser
{
  unsigned int Mseen;
  unsigned int Sseen;
  unsigned int Rseen;
  int M;
  float S;
  float R;
};

//init our string processing
void init_process_string()
{
  serial_count = 0;
  comment = false;
}

// Get a command and process it
void get_and_do_command()
{
  //read in characters if we got them.
  if (Serial.available())
  {
    c = Serial.read();
    if (c == '\r')
      c = '\n';
    // Throw away control chars except \n
    if (c >= ' ' || c == '\n')
    {

      //newlines are ends of commands.
      if (c != '\n')
      {
        // Start of comment - ignore any bytes received from now on
        if (c == ';')
          comment = true;

        // If we're not in comment mode, add it to our array.
        if (!comment)
          cmdbuffer[serial_count++] = c;
      }

    }
  }

  // Data runaway?
  if (serial_count >= COMMAND_SIZE)
    init_process_string();

  //if we've got a real command, do it
  if (serial_count && c == '\n')
  {
    Serial.println(cmdbuffer);
    // Terminate string
    cmdbuffer[serial_count] = 0;

    //process our command!
    process_string(cmdbuffer, serial_count);

    //clear command.
    init_process_string();

    // Say we're ready for the next one

    Serial.println("ok");
  }
}

/*
  #define PARSE_INT(ch, str, len, val) \
  case ch: \
  len = scan_int(str, &val); \
  break;

  #define PARSE_FLOAT(ch, str, len, val) \
  case ch: \
  len = scan_float(str, &val); \
  break;
*/
/*
#define PARSE_INT(ch, str, len, val, seen, flag) \
  case ch: \
  len = scan_int(str, &val, &seen, flag); \
  break;

#define PARSE_FLOAT(ch, str, len, val, seen, flag) \
  case ch: \
  len = scan_float(str, &val, &seen, flag); \
  break;
*/
//int scan_float(char *str, float *valp, unsigned int *seen, unsigned int flag)
int scan_float(char *str, float *valp, unsigned int *seen)
{
  float res;
  int len;
  char *end;

  res = (float)strtod(str, &end);

  len = end - str;

  if (len > 0)
  {
    *valp = res;
    *seen = 1;
  }
  else
    *valp = 0;

  return len;  /* length of number */
}

//int scan_int(char *str, int *valp, unsigned int *seen, unsigned int flag)
int scan_int(char *str, int *valp, unsigned int *seen)
{
  int res;
  int len;
  char *end;

  res = (int)strtol(str, &end, 10);
  len = end - str;

  if (len > 0)
  {
    *valp = res;
    *seen  = 1;
  }
  else
    *valp = 0;

  return len; /* length of number */
}

/*
  int scan_float(char *str, float *valp)
  {
  float res;
  int len;
  char *end;

  res = (float)strtod(str, &end);

  len = end - str;

  if (len > 0)
  {
     valp = res;
  }
  else
     valp = 0;

  return len;	// length of number
  }

  int scan_int(char *str, int *valp)
  {
  int res;
  int len;
  char *end;

  res = (int)strtol(str, &end, 10);
  len = end - str;

  if (len > 0)
  {
     valp = res;
  }
  else
     valp = 0;

  return len;	// length of number
  }
*/
void process_string(char instruction[], int size)
{

  GcodeParser gc;	/* string parse result */
  //double newSetpoint;
  //double newRate;
  int ind;
  int len;	/* length of parameter argument */

  gc.Mseen = 0;
  gc.Rseen = 0;
  gc.Sseen = 0;

  //If M0 then ramp
  //If M1 then move setpoint without ramp
  //If M2 then set flag to kill power after reaching target temperature
  //If M3 pause and hold power (for example, when opening the kilm
  //If M4 unpause and resume auto PID


  len = 0;
  /* scan the string for commands and parameters, recording the arguments for each,
     and setting the seen flag for each that is seen
  */

  for (ind = 0; ind < size; ind += (1 + len))
  {
    len = 0;
    switch (instruction[ind])
    {

      //PARSE_INT('M', &instruction[ind + 1], len, gc.M,gc.Seen,0x1);
      //PARSE_FLOAT('S', &instruction[ind + 1], len, gc.S,gc.Seen,0x2);
      //PARSE_FLOAT('R', &instruction[ind + 1], len, gc.R,gc.Seen,0x4);

      case 'M':
        len = scan_int(&instruction[ind + 1], &gc.M, &gc.Mseen);
        break;
      case 'S':
        len = scan_float(&instruction[ind + 1], &gc.S, &gc.Sseen);
        break;
      case 'R':
        len = scan_float(&instruction[ind + 1], &gc.R, &gc.Rseen);
        break;

        /*
          #define PARSE_INT(ch, str, len, val, seen, flag) \
          case ch: \
          len = scan_int(str, &val, &seen, flag); \
          break;

          #define PARSE_FLOAT(ch, str, len, val, seen, flag) \
          case ch: \
          len = scan_float(str, &val, &seen, flag); \
          break;
              default:
                break;*/
    }
  }

  if (gc.Mseen  == 0) //M was not seen!
  {
    
    Serial.println(", Invalid command");
    Serial.print("Seen = ");
  Serial.print(gc.Mseen);
  Serial.print(", M = ");
  Serial.print(gc.M);
  Serial.print(", S = ");
  Serial.print(gc.S);
  Serial.print(", R = ");
  Serial.print(gc.R);
  Serial.print(", Kill at temp = ");
  Serial.println(killFlag);
    return;
  }
  Serial.print("Seen = ");
  Serial.print(gc.Mseen);
  Serial.print(", M = ");
  Serial.print(gc.M);
  Serial.print(", S = ");
  Serial.print(gc.S);
  Serial.print(", R = ");
  Serial.print(gc.R);
  Serial.print(", Kill at temp = ");
  Serial.println(killFlag);

  if (gc.M <= 2) //if M0 M1 or M2
  {
    if (gc.Sseen)
    {
      if (gc.S >= TMIN && gc.S <= TMAX)
      {
        SetSetpoint(gc.S);
      }
    }
    if (gc.Rseen)
    {
      if (gc.R > RMIN && gc.R < RMAX)
      {

        SetRate(gc.R);
      }
    }

    if (gc.M == 2) //heat until
    {
      killFlag = true;
    }
    else if (gc.M == 1) //Jump to
    {
      //newRate = double(gc.R);
      //gc.R = 1000000000;
      //SetRate(1000000000); //there has to be a better way :P

      JumpSetpoint(gc.S);
      SetRate(0);
      killFlag = false;
    }
    else if (gc.M == 0) //Ramp to
      killFlag = false;
  }
  else if (gc.M == 3)
  {
    myPID.SetMode(MANUAL);
    hold = true;
    //pause setpoint at current
    //gc.S = Setpoint;
    //gc.R = 0;
    //JumpSetpoint(gc.S);

  }
  else if (gc.M == 4)
  {
    myPID.SetMode(AUTOMATIC);
    hold = false;
    //pause setpoint at current
    //gc.S = Setpoint;
    //gc.R = 0;
    //JumpSetpoint(gc-  >S);

  }



  return;
}

void reportResult(double Setpoint, double tempTC, double tempCJC,
                  bool faultOpen, bool faultShortGND, bool faultShortVCC,
                  double OutputP, int SampleInterval)
{
  // Report during each new window
  Serial.print(millis() / 1000.0);
  Serial.print("\t");
  Serial.print(Setpoint);
  Serial.print("\t");
  Serial.print(tempTC);
  Serial.print("\t");
  Serial.print(Setpoint - tempTC);
  Serial.print("\t");
  Serial.print(faultOpen);
  Serial.print(faultShortGND);
  Serial.print(faultShortVCC);
  Serial.print("\t"); //temp
  if (hold == false)
    Serial.println(OutputP);
  else
  {
    Serial.print(OutputP);
    Serial.print("\t");
    Serial.println("HOLD");
  }
  //Serial.print("\t");
  //Serial.println(SampleInterval);
}
