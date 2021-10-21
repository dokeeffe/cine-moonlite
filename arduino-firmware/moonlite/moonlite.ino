#include <AccelStepper.h>

// Motor pin definitions:
#define motorPin1  2      // IN1 on the ULN2003 driver
#define motorPin2  3      // IN2 on the ULN2003 driver
#define motorPin3  4     // IN3 on the ULN2003 driver
#define motorPin4  5     // IN4 on the ULN2003 driver

// Define the AccelStepper interface type; 4 wire motor in half step mode:
#define MotorInterfaceType 8

// Initialize with pin sequence IN1-IN3-IN2-IN4 for using the AccelStepper library with 28BYJ-48 stepper motor:
AccelStepper stepper = AccelStepper(MotorInterfaceType, motorPin1, motorPin3, motorPin2, motorPin4);

#define HOMEPOSITION 20000
#define MAXSPEED 500
#define SPEEDMULT 3
#define MAXCOMMAND 8
#define BACKLASHSTEPS 27

char inChar;
char cmd[MAXCOMMAND];
char param[MAXCOMMAND];
char line[MAXCOMMAND];
long pos;
int isRunning = 0;
int previousDirection = 0;
long backlashApplied = 0;
int speed = 32;
int eoc = 0;
int idx = 0;
long millisLastMove = 0;

void setup()
{
  Serial.begin(9600);

  stepper.setMaxSpeed(MAXSPEED);
  stepper.setSpeed(MAXSPEED);
  stepper.setAcceleration(MAXSPEED * .75);
  stepper.enableOutputs();
  stepper.setCurrentPosition(HOMEPOSITION);
  memset(line, 0, MAXCOMMAND);
  millisLastMove = millis();
}



void loop() {
  // run the stepper if there's no pending command and if there are pending movements
  if (!Serial.available())  {
    if (isRunning) {
      stepper.run();
      millisLastMove = millis();
    }
    else {
      // reported on INDI forum that some steppers "stutter" if disableOutputs is done repeatedly
      // over a short interval; hence we only disable the outputs and release the motor some seconds
      // after movement has stopped
      if ((millis() - millisLastMove) > 1000) {
        stepper.disableOutputs();
      }
    }

    if (stepper.distanceToGo() == 0) {
      stepper.run();
      isRunning = 0;
      if (backlashApplied != 0) {
        stepper.setCurrentPosition(stepper.currentPosition() - backlashApplied);
        backlashApplied = 0;
      }
    }
  }
  else {

    // read the command until the terminating # character
    while (Serial.available() && !eoc) {
      inChar = Serial.read();
      if (inChar != '#' && inChar != ':') {
        line[idx++] = inChar;
        if (idx >= MAXCOMMAND) {
          idx = MAXCOMMAND - 1;
        }
      }
      else {
        if (inChar == '#') {
          eoc = 1;
        }
      }
    }
  }

  // process the command we got
  if (eoc) {
    memset(cmd, 0, MAXCOMMAND);
    memset(param, 0, MAXCOMMAND);

    int len = strlen(line);
    if (len >= 2) {
      strncpy(cmd, line, 2);
    }

    if (len > 2) {
      strncpy(param, line + 2, len - 2);
    }

    memset(line, 0, MAXCOMMAND);
    eoc = 0;
    idx = 0;

    // LED backlight value, always return "00"
    if (!strcasecmp(cmd, "GB")) {
      Serial.print("00#");
    }

    // home the motor, hard-coded, ignore parameters since we only have one motor
    if (!strcasecmp(cmd, "PH")) {
      stepper.setCurrentPosition(HOMEPOSITION);
      stepper.moveTo(0);
      isRunning = 1;
    }

    // firmware value, always return "10"
    if (!strcasecmp(cmd, "GV")) {
      Serial.print("10#");
    }

    // get the current motor position
    if (!strcasecmp(cmd, "GP")) {
      pos = stepper.currentPosition();
      char tempString[6];
      sprintf(tempString, "%04X", pos);
      Serial.print(tempString);
      Serial.print("#");
    }

    // get the new motor position (target)
    if (!strcasecmp(cmd, "GN")) {
      pos = stepper.targetPosition();
      char tempString[6];
      sprintf(tempString, "%04X", pos);
      Serial.print(tempString);
      Serial.print("#");
    }

    // get the current temperature, hard-coded
    if (!strcasecmp(cmd, "GT")) {
      Serial.print("20#");
    }

    // get the temperature coefficient, hard-coded
    if (!strcasecmp(cmd, "GC")) {
      Serial.print("02#");
    }

    // get the current motor speed, speed change not supported
    if (!strcasecmp(cmd, "GD")) {
      char tempString[6];
      sprintf(tempString, "%02X", speed);
      Serial.print(tempString);
      Serial.print("#");
    }

    // set speed. Unsupported
    if (!strcasecmp(cmd, "SD")) {
      speed = hexstr2long(param);
      stepper.setSpeed(MAXSPEED);
      stepper.setMaxSpeed(MAXSPEED);
    }

    // whether half-step is enabled or not, always return "00"
    if (!strcasecmp(cmd, "GH")) {
      Serial.print("00#");
    }

    // motor is moving - 01 if moving, 00 otherwise
    if (!strcasecmp(cmd, "GI")) {
      if (abs(stepper.distanceToGo()) > 0) {
        Serial.print("01#");
      } else {
        Serial.print("00#");
      }
    }

    // set current motor position
    if (!strcasecmp(cmd, "SP")) {
      pos = hexstr2long(param);
      stepper.setCurrentPosition(pos);
    }

    // set new motor position and compensate for backlash
    if (!strcasecmp(cmd, "SN") and !isRunning) {
      long curPos = stepper.currentPosition();
      pos = hexstr2long(param);
      if (curPos != pos) {
        int newDirection = 1;
        if (pos < curPos) {
          newDirection = -1;
        }
        if (previousDirection != 0) {
          if (newDirection != previousDirection) {
            pos = pos + (BACKLASHSTEPS * newDirection);
            backlashApplied = BACKLASHSTEPS * newDirection;
          }
        }
        previousDirection = newDirection;
        stepper.moveTo(pos);
      }
    }


    // initiate a move
    if (!strcasecmp(cmd, "FG") and !isRunning) {
      isRunning = 1;
      stepper.enableOutputs();
    }

    // stop a move
    if (!strcasecmp(cmd, "FQ")) {
      isRunning = 0;
      stepper.moveTo(stepper.currentPosition());
      stepper.run();
    }
  }
}


long hexstr2long(char *line) {
  long ret = 0;

  ret = strtol(line, NULL, 16);
  return (ret);
}




