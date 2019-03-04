#include <Adafruit_NeoPixel.h>
#include <math.h>

#define LED_COUNT 60 
//The number of total LEDs in each gate

int currentRing = 0; 
// Tracks the active gate

int numRings = 2; 
// The total number of gates 

int rings_output[] = {5,6}; 
//Arduino pins that send data to the LEDs

int rings_input[] = {A0,A1}; 
//Arduino pins that read the voltage from a gate's solar cell

int sensorState = 0, lastState=0; 
int onlyShowActive = 1;
float Vstart[] = {0.0,0.0}; 
//Saves our baseline voltage recordings

float margin_of_error = .05; 
// The threshold of voltage drop that triggers a 'hit'

int firstTime = 1;

Adafruit_NeoPixel led1 = Adafruit_NeoPixel(LED_COUNT, rings_output[0], NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel led2 = Adafruit_NeoPixel(LED_COUNT, rings_output[1], NEO_GRB + NEO_KHZ800);

Adafruit_NeoPixel gate_leds[2] = {
  led1, 
  led2
};

//Sets a specific gate to all of one color
void setAllToColor(int gate, int r, int g, int b) {
  for (int j=0; j<LED_COUNT; j++)
        {
          gate_leds[gate].setPixelColor(j, gate_leds[gate].Color(r,g,b));
        }
  gate_leds[gate].show();
}

//Turns off all of the LEDs in a specific gate
void clearLEDs(int j)
{
   setAllToColor(j, 0,0,0);
}

void setup() {

  pinMode(5, OUTPUT);
  pinMode(A0, INPUT);
  pinMode(6, OUTPUT);
  pinMode(A1, INPUT);
  for (int i=0; i < 2; i++) {
    gate_leds[i].begin();       
    //Call this to start up the LED strip.
    setAllToColor(i, 0,0,255);    
    //Turns all gates on to make sure they are working
    gate_leds[i].show();
    //The LEDs don't acrtually light up until you show() them
  }
  delay(500);  
  //Pause for LEDs to reach full brightness before taking the baseline reading
}

//Sets the new baseline for a specific gate
void resetVStart(int i) {
    float voltage= analogRead(rings_input[i]) * (5.0 / 1023.0);
    Vstart[i] = voltage;
}
  
void loop() {

 if (firstTime) {
    for (int i=0; i < numRings; i++) {
      resetVStart(i);
    }
    firstTime = 0;
  }
  //if this is the first loop, record the baseline voltage
  
  for (int i=0; i < numRings; i++) {
      float voltage= analogRead(rings_input[i]) * (5.0 / 1023.0); 
      //Read the current voltage input from the sensor
      if (currentRing==i) {

        //If this is the active ring check if the current reading indicates a beam break
        if (voltage < (Vstart[i] - margin_of_error)) {
            
            //The beam is broken, flash green to indicate success
            setAllToColor(i, 0,255,0);
            gate_leds[i].show();
            delay(250);
            setAllToColor(i, 0,0,0);
            gate_leds[i].show();
            delay(250);
            setAllToColor(i, 0,255,0);
            gate_leds[i].show();
            delay(250);
            setAllToColor(i, 0,0,0);
            gate_leds[i].show();
            delay(250);
            setAllToColor(i, 0,255,0);
            gate_leds[i].show();
            delay(250);
            setAllToColor(i, 0,0,0);
            gate_leds[i].show();
            delay(250);
            gate_leds[i].show(); 
            
            currentRing++;   
            //Cycle to next gate in the series 
            
            if (currentRing>=numRings) 
             {
              currentRing = 0;
             }
             //If that was the last gate, start over
            
            setAllToColor(currentRing, 0,0,255);
            //Turn the new gate blue
            
            delay(500); 
            //Pause again before taking the new baseline reading
        } else {
          //If the beam is unbroken, keep the same color
          setAllToColor(i, 0,0,255);
        }
      } else {
        //If the gate is not an active one, keep it OFF
        setAllToColor(i, 0,0,0);
      }  
      gate_leds[i].show();   
      //Show the LEDs
  }
  
} 

