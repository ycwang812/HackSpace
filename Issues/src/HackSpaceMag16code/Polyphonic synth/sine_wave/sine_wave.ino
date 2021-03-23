// include various libraries required for Teensy audio
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

AudioSynthWaveformSine oscillator; // declare an oscillator
AudioOutputI2S mainOutput; // declare the main output

// declare stereo connections from the oscillator to the output
AudioConnection audioConnection1(oscillator, 0, mainOutput, 0);
AudioConnection audioConnection2(oscillator, 0, mainOutput, 1);

AudioControlSGTL5000 teensyAudioBoard; // declare the Teensy audio board

void setup() {
  AudioMemory(50); // allocate some memory to be used for audio functions
  teensyAudioBoard.enable(); // enable the audio board
  teensyAudioBoard.volume(0.5); // set the audio board volume
  oscillator.amplitude(0.3); // set the oscillator volume
  oscillator.frequency(220); // set the frequency (pitch) of the oscillator
}

void loop() {
  
}
