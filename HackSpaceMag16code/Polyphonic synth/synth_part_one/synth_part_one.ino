#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   lfo;          //xy=179,562
AudioMixer4              oscillatorModMixer;         //xy=382,483
AudioMixer4              filterInputMixer;         //xy=386,566
AudioMixer4              filterModMixer;         //xy=388,642
AudioFilterStateVariable filter;        //xy=587,606
AudioSynthWaveformModulated sawtoothOscillator;   //xy=598,496
AudioSynthWaveformModulated squareOscillator;   //xy=599,456
AudioMixer4              mainOutputMixer;         //xy=800,560
AudioOutputI2S           mainOutput;           //xy=990,561
AudioConnection          patchCord1(lfo, 0, mainOutputMixer, 2);
AudioConnection          patchCord2(lfo, 0, oscillatorModMixer, 2);
AudioConnection          patchCord3(lfo, 0, filterInputMixer, 2);
AudioConnection          patchCord4(lfo, 0, filterModMixer, 2);
AudioConnection          patchCord5(oscillatorModMixer, 0, squareOscillator, 0);
AudioConnection          patchCord6(oscillatorModMixer, 0, sawtoothOscillator, 0);
AudioConnection          patchCord7(filterInputMixer, 0, filter, 0);
AudioConnection          patchCord8(filterModMixer, 0, filter, 1);
AudioConnection          patchCord9(filter, 0, mainOutputMixer, 3);
AudioConnection          patchCord10(filter, 0, oscillatorModMixer, 3);
AudioConnection          patchCord11(filter, 0, filterInputMixer, 3);
AudioConnection          patchCord12(filter, 0, filterModMixer, 3);
AudioConnection          patchCord13(sawtoothOscillator, 0, mainOutputMixer, 1);
AudioConnection          patchCord14(sawtoothOscillator, 0, oscillatorModMixer, 1);
AudioConnection          patchCord15(sawtoothOscillator, 0, filterInputMixer, 1);
AudioConnection          patchCord16(sawtoothOscillator, 0, filterModMixer, 1);
AudioConnection          patchCord17(squareOscillator, 0, mainOutputMixer, 0);
AudioConnection          patchCord18(squareOscillator, 0, oscillatorModMixer, 0);
AudioConnection          patchCord19(squareOscillator, 0, filterInputMixer, 0);
AudioConnection          patchCord20(squareOscillator, 0, filterModMixer, 0);
AudioConnection          patchCord21(mainOutputMixer, 0, mainOutput, 0);
AudioConnection          patchCord22(mainOutputMixer, 0, mainOutput, 1);
AudioControlSGTL5000     audioBoard;     //xy=859,473
// GUItool: end automatically generated code

AudioMixer4 *inputMixers[4] = {
  &oscillatorModMixer,
  &filterInputMixer,
  &filterModMixer,
  &mainOutputMixer
};

const int ADDRESS_SEND_PINS[] = {2,3,4};
const int ADDRESS_READ_PINS[] = {5,6,7};
const int CONNECTION_SEND_PIN = 10;
const int CONNECTION_READ_PIN = 8;
const int LED_PIN = 14;
const int ANALOG_PIN_1 = 20;
const int ANALOG_PIN_2 = 21;

void setup() {
  for(int i=0;i<3;i++) {
    pinMode(ADDRESS_SEND_PINS[i], OUTPUT);
    pinMode(ADDRESS_READ_PINS[i], OUTPUT);
  }
  pinMode(CONNECTION_SEND_PIN, OUTPUT);
  pinMode(CONNECTION_READ_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  
  digitalWrite(CONNECTION_SEND_PIN, LOW);
  digitalWrite(LED_PIN, LOW);

  Serial.begin(9600);

  AudioMemory(50);
  audioBoard.enable();
  audioBoard.volume(0.4);
  lfo.amplitude(0.2);
  lfo.frequency(1.5);
  squareOscillator.begin(0.25, 110, WAVEFORM_SQUARE);
  sawtoothOscillator.begin(0.25, 110, WAVEFORM_SAWTOOTH);
  filter.resonance(2.5);
  filter.frequency(500);
  filter.octaveControl(8);
}

void loop() {
  boolean anyBadConnections = false;
  
  for(int a=0;a<8;a++) {
    setSendChannel(a);
    for(int b=0;b<8;b++) {
      setReadChannel(b);
      delayMicroseconds(10);
      if(a < b) {
        boolean connectionReading = !digitalRead(CONNECTION_READ_PIN);
        if(connectionReading) {
          if(a <= 3 && b >= 4) {
            inputMixers[b-4]->gain(a, 1.0);
          } else {
            anyBadConnections = true;
          }
        } else {
          if(a <= 3 && b >= 4) {
            inputMixers[b-4]->gain(a, 0.0);
          }
        }
      }
      float oscillatorFrequency = 60.0 + analogRead(ANALOG_PIN_1);
      float lfoFrequency = 0.1 + analogRead(ANALOG_PIN_2)/25.0;
      squareOscillator.frequency(oscillatorFrequency);
      sawtoothOscillator.frequency(oscillatorFrequency);
      lfo.frequency(lfoFrequency);
    }
  }
  digitalWrite(LED_PIN, anyBadConnections);
}

void setSendChannel(int channel) {
  digitalWrite(ADDRESS_SEND_PINS[0], bitRead(channel, 0));
  digitalWrite(ADDRESS_SEND_PINS[1], bitRead(channel, 1));
  digitalWrite(ADDRESS_SEND_PINS[2], bitRead(channel, 2));
}

void setReadChannel(int channel) {
  digitalWrite(ADDRESS_READ_PINS[0], bitRead(channel, 0));
  digitalWrite(ADDRESS_READ_PINS[1], bitRead(channel, 1));
  digitalWrite(ADDRESS_READ_PINS[2], bitRead(channel, 2));
}


