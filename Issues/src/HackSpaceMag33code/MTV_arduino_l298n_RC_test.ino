/*
 * A simple sketch that reads the pulses from a Remote Control Receiver and maps the value to be sent to the speed control EN1
 * on a L298N motor control board.  
 * Arduino pin connections are as follows 
 * Pin 5 -> EN1 on L298n
 * Pin 8 -> IN1 on L298n
 * Pin 7 -> IN2 on L298n
 * Pin 9 -> Signal pin of RC receiver you wish to use to control motor
 * 
 * We also used the regulated 5v from the Arduino to power the RC receiver connecting 5v and GND between them. 
 * Motor and L298n powered by 11.1v 3 cell LiPo which also supplied the Arduino via the regulated jack input.  
 */

int RC2; // Pulse length from RC channel receiver we will measure

int MotorSpeed; // The mapped value we will create to send as the speed control value to EN1

int IN1 = 8; // Pin assignment for IN1 of L298n
int IN2 = 7; // Pin assignment for IN2 of L298n
int EN1 = 5; // Pin assignment for EN1 of L298n


void setup() {

  pinMode(IN1, OUTPUT);  
  pinMode(IN2, OUTPUT);
  pinMode(EN1, OUTPUT); 
  

  pinMode(9, INPUT); // Connect the RC receiver channel pin you want to use to control this motor to this pin

}

void loop() {

  RC2 = pulseIn(9, HIGH, 10000); // Write the pulse length from the RC receiver pin to a the RC2 variable. 
  
  MotorSpeed = map(RC2, 900, 2100, 0, 255); //Reads RC2 which falls between 900 and 2100 microseconds and maps it across 0-255 range, 
                                            // We realised that the start points and end point of the RC pulsewidth are a littel higher and lower than defined

    digitalWrite(IN1, HIGH); //set the L298n pin HIGH to set motor direction
    digitalWrite(IN2, LOW);  //set the L298n pin LOW to set motor direction (reverse these last two pin values to swap motor direction)
    analogWrite(EN1, MotorSpeed); //  Send our mapped MotorSpeed value to the speed control pin

}
