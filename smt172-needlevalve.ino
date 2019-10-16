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
String str; //string received by serial port
int count; //amount of elements after splitting the received string
String sParams[10];
bool calibration_done = false; //no movement before calibration
bool moving = false; //used to determine if the motor is moving
long initial_homing=-1;  // Used to Home Stepper at startup
float motorspeed = 300;
float motoraccel = 3000;


void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);
  pinMode(home_switch, INPUT_PULLUP); //enable internal pullup
 
}


void loop() {
  time_now = millis();
  
  //check if the motor is running to a position or not
  if(stepper.distanceToGo() != 0){
     moving = true;
   }else{
     moving = false;
   }

  //if 1 second has passed, since last measurement and the motor is not moving, read temperature
  //again and send it 
  if(((time_now - last_update) >= 1000) && !moving){
        getTemp();
        Serial.println(temp);
        last_update = millis();
  }

  if(Serial.available()){
     str = Serial.readStringUntil('\n');
     count = StringSplit(str,':',sParams,6);
       
     
     //check homeswitch. Use=> es Returns 0(triggered) or 1(not triggered)
     if(sParams[0] == "es" && count == 1){
        CheckHomeswitch();
     }
       
     
     //calibrate zero position needlevalve. Use=> cal:steps_beyond_homeswitch:motor_acceleration:motor_speed Returns nothing
     if(sParams[0] == "cal" && count == 4){
        CalibrateNeedleValve(sParams[1].toInt(),sParams[2].toFloat(),sParams[3].toFloat());
     }
     
     
     //move non-blocking to absolute position. Use=> move:target_position:motor_acceleration:motor_speed Returns nothing unless not calibrated
     if(sParams[0] == "move" && count == 2){
        MoveTo(sParams[1].toInt());
     }
     
     
     //move blocking to absolute position. Use=> pos:target_position:motor_acceleration:motor_speed Returns nothing unless not calibrated
     if(sParams[0] == "pos" && count == 2){
        MoveToPosition(sParams[1].toInt());
     }
     

     if(sParams[0] == "spd" && count == 2){
      motorspeed = sParams[1].toFloat();
      stepper.setMaxSpeed(motorspeed);
      stepper.setSpeed(motorspeed);
     }
     

     if(sParams[0] == "acc" && count == 2){
      motoraccel = sParams[1].toFloat();
      stepper.setAcceleration(motoraccel);
     }
     
  }
  stepper.run();
}

//find sensor and get temperature
void getTemp(){
  SMT172::startTemperature(0.001);
  repeat:
  switch (SMT172::getStatus()) {
    case 0: goto repeat; //Edwin Croissant is very sorry for this
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
    }
    return (iParamCount);
}


//non-blocking move to absolute postion
void MoveTo(int steps){
  //only move if calibrated
  if(calibration_done){
    
    //if we are not moving, move immediately, otherwise stop first, then move
    if(moving){
      stepper.stop();
    }
    stepper.moveTo(steps);
    stepper.run();
    
  }else{
    Serial.println("not calibrated");
  }
  
}


//blocking move to absolute position
void MoveToPosition(int absolutepos){
  //only move if calibrated
  if(calibration_done){
    //if the motor is not moving, move immediately, otherwise stop first, then move
    if(!moving){
      stepper.runToNewPosition(absolutepos);
    }else{
      stepper.stop();
      stepper.runToNewPosition(absolutepos);
    }
  }else{
    Serial.println("not calibrated");
  }
}


//check current state homing switch. Normally Open microswitch between gnd and the input pin with internal pull-up enabled, so 1 is not triggered, 0 is triggered
void CheckHomeswitch(){
  Serial.print("es:");
  Serial.println(digitalRead(home_switch));
}


//calibrate zero position, using the homing switch as a fixed point from where to count steps back to the closed
//position of the needle valve and set zero
void CalibrateNeedleValve(int steps_past_home, float calibrationaccel, float calibrationspeed){
   //if the motor is moving for some reason, stop first
   if(moving){
    stepper.stop();
   }
   //set the speed to use during the calibration
   stepper.setMaxSpeed(calibrationspeed);
   stepper.setAcceleration(calibrationaccel);
  
   //homing switch not initially triggered NO (normally open)
   while (digitalRead(home_switch)) {  // Make the Stepper move CW until the switch is activated   
    stepper.moveTo(initial_homing);  // Set the position to move to
    initial_homing++;  // Increase by 1 for next move if needed
    stepper.run();  // Start moving the stepper
    delay(5);
   }

   stepper.setCurrentPosition(0);  // Set the current position as zero for now
   stepper.setMaxSpeed(calibrationspeed);      // Set Max Speed of Stepper (Slower to get better accuracy)
   stepper.setAcceleration(calibrationaccel);  // Set Acceleration of Stepper
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

   //run motor until the position past the homing switch is reached
   while(stepper.distanceToGo() != 0){
     stepper.run();
   }
  
  //final zero position, calibration done
   stepper.setCurrentPosition(0);
   stepper.setMaxSpeed(motorspeed);
   stepper.setSpeed(motorspeed);
   stepper.setAcceleration(motoraccel);
   calibration_done = true;
}
