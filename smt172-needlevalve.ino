// needlevalve control and a smt172 temperature sensor
//by nrbrt
//GPLv3 applies

#include <AccelStepper.h>
#include <SMT172.h>

//Define stepper motor connections
#define dirPin 12
#define stepPin 11

#define home_switch 10 // Pin 10 connected to Home Switch (MicroSwitch (NO))

//Create stepper object
AccelStepper stepper(1,stepPin,dirPin); //motor interface type must be set to 1 when using a driver.

long last_update = 0;
long time_now;
float temp = 0;
String str;
int iCount, i;
String sParams[6];
bool calibration_done = false;
bool debug = false;
bool moving = false;

// Stepper Travel Variables

long initial_homing=-1;  // Used to Home Stepper at startup


void setup() {
  Serial.begin(115200);
  pinMode(home_switch, INPUT_PULLUP);
 
}


void loop() {
  time_now = millis();
  
  //check if the motor is running to a position or not
  if(stepper.distanceToGo() != 0){
     stepper.run();
     moving = true;
   }else{
    moving = false;
   }

  //if 1 second has passed, since last measurement and the motor is not moving, read temperature
  //again and send it 
  if((time_now - last_update >= 1000) && !moving){
        getTemp();
        Serial.println(temp);
        last_update = millis();
  }

  if(Serial.available() > 0){
     str = Serial.readStringUntil('\n');
     iCount = StringSplit(str,':',sParams,6);
     stepper.run();
     
     //check homeswitch
     if(sParams[0] == "es"){
        CheckHomeswitch();
     }
     stepper.run();   
     
     //calibrate zero position needlevalve
     if(sParams[0] == "cal"){
        CalibrateNeedelValve(sParams[1].toInt());
     }
     stepper.run();
     
     //move non-blocking to absolute position
     if(sParams[0] == "move"){
        MoveTo(sParams[1].toInt());
     }
     stepper.run();
     
     //move blocking to absolute position
     if(sParams[0] == "pos"){
        MoveToPosition(sParams[1].toInt());
     }
     stepper.run();
  }
stepper.run();
}

//find sensor and get temperature
void getTemp(){
  SMT172::startTemperature(0.001);
  repeat:
  switch (SMT172::getStatus()) {
    case 0: goto repeat;
    case 1: 
      temp = SMT172::getTemperature();
      break;
    case 2:
      Serial.println("no sensor");
  }
}


//split string
int StringSplit(String sInput, char cDelim, String sParams[], int iMaxParams)
{
    int iParamCount = 0;
    int iPosDelim, iPosStart = 0;
    stepper.run();
    do {
        // Searching the delimiter using indexOf()
        iPosDelim = sInput.indexOf(cDelim,iPosStart);
        if (iPosDelim > (iPosStart+1)) {
            // Adding a new parameter using substring() 
            sParams[iParamCount] = sInput.substring(iPosStart,iPosDelim);
            iParamCount++;
            // Checking the number of parameters
            if (iParamCount >= iMaxParams) {
                return (iParamCount);
            }
            iPosStart = iPosDelim + 1;
        }
    } while (iPosDelim >= 0);
    if (iParamCount < iMaxParams) {
        // Adding the last parameter as the end of the line
        sParams[iParamCount] = sInput.substring(iPosStart);
        iParamCount++;
        stepper.run();
    }
    stepper.run();
    return (iParamCount);
}


//non-blocking move to absolute postion
void MoveTo(int steps){
  //only move if calibrated
  if(calibration_done){
    //if we are not moving, move immediately, otherwise stop first, then move
    if(!moving){
      stepper.setAcceleration(1000.0);
      stepper.setMaxSpeed(180.0);
      stepper.setSpeed(180); 
      stepper.moveTo(steps);    
      stepper.run();
    }else{
      stepper.stop();
      stepper.setMaxSpeed(180.0);
      stepper.setAcceleration(1000.0);
      stepper.setSpeed(180);
      stepper.moveTo(steps);       
      stepper.run();
    }
  }else{
    Serial.println('not calibrated');
  }
  
}


//blocking move to absolute position
void MoveToPosition(int absolutepos){
  //set speed
  stepper.setMaxSpeed(180.0);
  stepper.setAcceleration(1000.0);
  stepper.setSpeed(180);
  //only move if calibrated
  if(calibration_done){
    //if the motor is not moving, move immediately, otherwise stop first, then move
    if(!moving){
      stepper.runToNewPosition(absolutepos);
    }else{
      stepper.stop();
      stepper.runToNewPosition(absolutepos);
    }
  }
}


//check current state homing switch
void CheckHomeswitch(){
  Serial.print("es:");
  Serial.println(digitalRead(home_switch));
  stepper.run();
}


//calibrate zero position, using the homing switch as a fixed point from where to count steps back to the closed
//position of the needle valve and set zero
void CalibrateNeedelValve(int steps_past_home){
   //if the motor is moving for some reason, stop first
   if(moving){
    stepper.stop();
   }
   //set the speed to use during the calibration
   stepper.setMaxSpeed(100.0);
   stepper.setAcceleration(100.0);
  
   //homing switch not initially triggered NO (normally open)
   while (digitalRead(home_switch)) {  // Make the Stepper move CW until the switch is activated   
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing++;  // Increase by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    delay(5);
   }

   stepper.setCurrentPosition(0);  // Set the current position as zero for now
   stepper.setMaxSpeed(100.0);      // Set Max Speed of Stepper (Slower to get better accuracy)
   stepper.setAcceleration(100.0);  // Set Acceleration of Stepper
   initial_homing=1;

   //homing switch initially triggered NO (normally open)
   while (!digitalRead(home_switch)) { // Make the Stepper move CCW until the switch is deactivated
     stepper.moveTo(initial_homing);  
     stepper.run();    // Start moving the stepper
     initial_homing--; //Decrease by 1 for next move if needed
     delay(5);
   }

   //temporary zero position
   stepper.setCurrentPosition(0); 
   stepper.moveTo(steps_past_home);

   //run motor until the position pas the homing switch is reached
   while(stepper.distanceToGo() != 0){
     stepper.run();
   }
  
  //final zero position, calibration done
   stepper.setCurrentPosition(0);
   calibration_done = true;
}
