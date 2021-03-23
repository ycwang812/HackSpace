#include <AccelStepper.h>
#include <MultiStepper.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// globals for the turntable
bool tt_mode = 0; // 0 for constant, 1 for stepped
long tt_pos = 0; // 0 to 360000 rotation
long tt_step = 0; // 0 to 36000 rotation 
int tt_rpm = 1; // 0 to 6 rpm
bool runMotor = 0; // in continuous mode, should we start or stop the motor?
long tick; // timing value to govern the refresh rate


int btn_step = 13;
int pot_step = A3;
int pot_rpm = A2;
int temp_step;

const int stepsPerRevolution = 600; // stepper motor conf 
const int tt_divisions = 90; // maximum number of divisions of the turntable
const int motorInterfaceType = 1; // stepper motor conf 
const int stepPin = 3; // stepper motor conf 
const int dirPin = 2; // stepper motor conf 

LiquidCrystal_I2C lcd(0x27,20,4); //setup LCD screen
AccelStepper motor = AccelStepper(motorInterfaceType, stepPin, dirPin);

void display_splash()
{
    // Print a message to the LCD. ROW, COL, MESSAGE

  lcd.setCursor(0,0);
  lcd.print("   SHEDLANDIA.COM   ");  
  lcd.setCursor(0,1);
  lcd.print("  Welding Turntable ");
   lcd.setCursor(0,2);
  lcd.print("   Digital Indexer  ");
   lcd.setCursor(0,3);
  lcd.print("   MonkeyNuts Inc   ");

};

void display_vars(){
  // variables
  lcd.setCursor(6,0);
  if (tt_mode==0){lcd.print("CONSTANT");}
  else if (tt_mode==1){lcd.print("STEPPED");};
  lcd.print("   ");

  lcd.setCursor(6,1);
  lcd.print(tt_rpm/10.0); // divide by 10 to get the decimal display
  lcd.print("   ");

  lcd.setCursor(6,2);
  lcd.print(tt_step);
  lcd.print("   ");

  //lcd.setCursor(6,3);
  //lcd.print(tt_pos);
  //lcd.print("   ");

}

void display_background()
{
  // background
  lcd.setCursor(0,0);
  lcd.print("MODE:               ");  
  lcd.setCursor(0,1);
  lcd.print("RPM:                ");
  lcd.setCursor(0,2);
  lcd.print("DIVS:              ");
  lcd.setCursor(0,3);
  //lcd.print("POS:                ");
  lcd.print("                    ");
  display_vars();
}
void setup()
{
  // add some extra pins for power
  pinMode(6, OUTPUT);
  pinMode(7, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(6, HIGH); //+5
  digitalWrite(7, HIGH); //+5
  digitalWrite(8, HIGH); //+5
  digitalWrite(9, HIGH); //+5
  digitalWrite(10, LOW); //GND
  digitalWrite(11, LOW); //GND
  digitalWrite(12, LOW); //GND

  // connect the buttons
  pinMode(btn_step, INPUT_PULLUP);
  
  // initialize the lcd and display the splash
  lcd.init();               
  lcd.backlight();
  display_splash();
  delay(2000);
  display_background();
  display_vars();

  // set the stepper motor speed
  motor.setMaxSpeed(500);

}


void loop()
{
// get the values from pots and buttons
temp_step = map(analogRead(pot_step),10,1000,0,tt_divisions);
tt_step = constrain(temp_step,0,tt_divisions);
tt_rpm = map(analogRead(pot_rpm),0,1000,-60,60);

// if there's no divs then the mode is constant
if (tt_step < 2) {
  tt_mode = 0;
  } else {
  tt_mode = 1;
  }

if (tt_mode == 0){
  // turntable mode  
  // button toggles start or stop
  if (digitalRead(btn_step) == LOW){
      if (runMotor == 1) {
        runMotor = 0;
      } else {
        runMotor = 1;
      }
      delay(1000); //debounce the button press
    }
  // advance the motor
  if (runMotor == 1){
    motor.setSpeed((tt_rpm/600.0)*stepsPerRevolution);
    motor.runSpeed();
    }
    
} else {
  // indexing mode
  // button toggles step
  runMotor = 0;
  if (digitalRead(btn_step) == LOW){
    motor.setCurrentPosition(0);
    while(abs(motor.currentPosition()) != stepsPerRevolution/tt_step){
      motor.setSpeed((tt_rpm/600.0)*stepsPerRevolution);
      motor.runSpeed();
      }
    delay(1000); // debounce the button press
    }
  }
  
  // do screen updates if tick has tocked
  if (millis() > tick+500 and runMotor == 0){
    tick = millis();
    display_vars();
  }
}

