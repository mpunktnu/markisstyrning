#include <ZUNO_Stepper.h>
//Howto: Press lower limit to initiate calibration

//TODO: Fix variable for speed and rotation direction

CCBYTE motor_pins[] = {5, 6, 7, 8}; // z-uno pins connected to motor, 4-pins mode
const int stepsPerRevolution = 200;  // change this to fit the number of steps per revolution

// initialize the stepper library on pins 5 through 8:
Stepper myStepper(stepsPerRevolution, sizeof(motor_pins), motor_pins);


int LASTMOTORVALUE[1];
long rotations = 0;
bool calibration = 0;

#define button1          9
#define button2         10
#define enableMotor     11

ZUNO_SETUP_SLEEPING_MODE(ZUNO_SLEEPING_MODE_ALWAYS_AWAKE);
//ZUNO_SETUP_CFGPARAMETER_HANDLER(config_parameter_changed);

//make sure sensors are the first channels. Else reports won't work correct.
ZUNO_SETUP_CHANNELS(ZUNO_BLINDS(getMotorValue, setMotorValue));

void calibrate(){
  //run to upper endstop
  Serial.println("Opening blinds");
  if (digitalRead(button1) != LOW){
    do{
      myStepper.step(1); 
    
    } while (digitalRead(button1) != LOW);  
  };

  //run to lower endstop and count number of revolutions
  Serial.println("Closing blinds");
  if (digitalRead(button2) != LOW){
    do{
      myStepper.step(-1); 
      rotations += 1;
    } while (digitalRead(button2) != LOW);  
  };

  Serial.println("Opening blinds");
  //return to upper endstop
  if (digitalRead(button1) != LOW){
    do{
      myStepper.step(1); 
    
    } while (digitalRead(button1) != LOW);  
  };  

  LASTMOTORVALUE[0] = 99;
}

void setup() {
  // initialize the serial port:
  Serial.begin(115200);
  Serial.println("BEGIN");
  
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);  
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(enableMotor, OUTPUT);

  digitalWrite(enableMotor, LOW);

  LASTMOTORVALUE[0] = 99;
  
  // set the speed at 100 rpm:
  myStepper.setSpeed(100);
}

void loop() {
  //start calibration when button 2 is pressed
  if (calibration == 0 && digitalRead(button2) == LOW){
    digitalWrite(enableMotor, HIGH); //enable motor
    calibrate();   
    calibration = 1;
    digitalWrite(enableMotor, LOW); //disable motor
  };
  
  delay(200);
}

// ################### Motor setter/getter functions #####################
byte getMotorValue() {
  return LASTMOTORVALUE[0];
}

void setMotorValue(byte newVal) {
  digitalWrite(enableMotor, HIGH); //enable motor
  if (newVal == 0){
    //close blind
      long howFarHaveWeGone = 0;
      if (digitalRead(button2) != LOW){
        do{
          myStepper.step(-1);
          howFarHaveWeGone += 1; 
        } while (digitalRead(button2) != LOW && howFarHaveWeGone <= (rotations*1.5));  //if switch failes we only run for a maximum of 1.5 times the calucaled distance!
      };
    
    } 
   else if (newVal == 99){
      //open blind
      long howFarHaveWeGone = 0;
      if (digitalRead(button1) != LOW){
        do{
          myStepper.step(1); 
          howFarHaveWeGone += 1;
        } while (digitalRead(button1) != LOW && howFarHaveWeGone <= (rotations*1.5));  //if switch failes we only run for a maximum of 1.5 times the calucaled distance!
      };  
    }
//  else if (newVal == 255){
    //open to last known position
//  } 
  else {
    //open to selected position
    int numberOfSteps = rotations/100;
    int lastPosition = LASTMOTORVALUE[0] * numberOfSteps;

    long newPos = (numberOfSteps * newVal)-lastPosition;

    long howFarHaveWeGone = 0;

    //if we are going down
    if (newPos < 0){
      if (digitalRead(button2) != LOW){
        do{
          myStepper.step(-1); 
          howFarHaveWeGone -= 1;
        } while (digitalRead(button2) != LOW && howFarHaveWeGone != newPos);  
      }; 
      
    }
    //if we are going up
    else {
      if (digitalRead(button1) != LOW){
        do{
          myStepper.step(1); 
          howFarHaveWeGone += 1;
        } while (digitalRead(button1) != LOW && howFarHaveWeGone != newPos);  
      }; 
      
    }

    };
  LASTMOTORVALUE[0] = newVal;
  digitalWrite(enableMotor, LOW); //disable motor
}

