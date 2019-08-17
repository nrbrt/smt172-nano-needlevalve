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
long last_measured = 0;
long time_now;
float temp = 0;
String str;
int iCount, i;
String sParams[6];
bool calibration_done = false;

// Stepper Travel Variables

long initial_homing=-1;  // Used to Home Stepper at startup


void setup() {
  Serial.begin(115200);
  pinMode(home_switch, INPUT_PULLUP);
 
}


void loop() {
  time_now = millis();

  if((time_now - last_update >= 1000) && (temp != 0)){
        Serial.println(temp);
        temp = 0;
        last_update = millis();
  }

  if(time_now - last_measured >= 500){
    getTemp();
    last_measured = millis();
  }

  if(Serial.available() > 0){
     str = Serial.readStringUntil('\n');
     iCount = StringSplit(str,':',sParams,6);

     //check homeswitch
     if(sParams[0] == "es"){
        CheckHomeswitch();
     }
        
     //calibrate zero position needlevalve
     if(sParams[0] == "cal"){
        CalibrateNeedelValve(sParams[1].toInt());
     }

     //move steps
     if(sParams[0] == "move"){
        MoveTo(sParams[1].toInt());
     }

     //move to absolute position
     if(sParams[0] == "pos"){
        MoveToPosition(sParams[1].toInt());
     }
  }
}


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

void MoveTo(int steps){
  stepper.setMaxSpeed(100.0);
  stepper.setAcceleration(100.0);
  
  if(calibration_done){
    stepper.moveTo(steps);
    while(stepper.distanceToGo() != 0){
      stepper.run();
    }
  }else{
    Serial.println('not calibrated');
  }
  
}

void MoveToPosition(int absolutepos){
  if(calibration_done){
    stepper.runToNewPosition(absolutepos);
  }
}

void CheckHomeswitch(){
  Serial.print("es:");
  Serial.println(digitalRead(home_switch));
}

void CalibrateNeedelValve(int steps){
 
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
   stepper.setCurrentPosition(0); 
   stepper.moveTo(steps);
   while(stepper.distanceToGo() != 0){
     stepper.run();
   }
  
   stepper.setCurrentPosition(0);
   calibration_done = true;
}

