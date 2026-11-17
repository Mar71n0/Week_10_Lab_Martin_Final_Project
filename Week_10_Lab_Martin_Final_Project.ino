#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_NeoPixel.h>

// Audio connections: sine waveform to both headphone channels.
AudioSynthWaveform waveform1;
AudioOutputI2S i2s1;
AudioConnection patchCord1(waveform1, 0, i2s1, 0);
AudioConnection patchCord2(waveform1, 0, i2s1, 1);
AudioControlSGTL5000 sgtl5000_1;

int tempoPot = A15;
int powerSwitch = 37;
int arpSwitch = 26;

int numPixels = 8;
int pixelPin = 33;
Adafruit_NeoPixel neopixel = Adafruit_NeoPixel(numPixels, pixelPin, NEO_GRB);

// C4, D4, E4, F4, G4, A4, B4, C5
float noteFrequency[8] = {
  261.63, 293.66, 329.63, 349.23,
  392.00, 440.00, 493.88, 523.25
};

int arpNotes[4] = {0, 4, 7, 11};

unsigned long lastStepTime = 0;
int stepLength = 1000;
int currentStep = 0;
int totalSteps = 8;
int arpStep = 0;

bool switchState = LOW;
bool arpState = LOW;
bool lastArpState = LOW;
bool lastSwitchState = LOW;

void setup() {
  Serial.begin(9600);
  AudioMemory(50);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.9);

  waveform1.begin(WAVEFORM_SINE);
  waveform1.frequency(noteFrequency[0]);
  waveform1.amplitude(0);

  pinMode(powerSwitch, INPUT);
  pinMode(arpSwitch, INPUT);

  neopixel.begin();
  neopixel.clear();
  neopixel.show();

  checkLeds();
  Serial.println(currentStep);
}

void loop() {
  lastSwitchState = switchState;
  switchState = digitalRead(powerSwitch);

  if (lastSwitchState != switchState) {
    Serial.print("Power: ");
    Serial.println(switchState);
  }

  lastArpState = arpState;
  arpState = digitalRead(arpSwitch);

  if (lastArpState != arpState) {
    arpStep = 0;
    lastStepTime = millis();
  }

  if (lastSwitchState == LOW and switchState == HIGH) {
    lastStepTime = millis();
  }

  if (switchState == HIGH) {
    sequence();
    playNote();
  }

  if (switchState == LOW) {
    waveform1.amplitude(0);
  }
}

void sequence() {
  int potValue = analogRead(tempoPot);
  stepLength = map(potValue, 0, 1023, 100, 1000);

  if (millis() >= lastStepTime + stepLength) {
    lastStepTime = millis();

    if (arpState == HIGH) {
      arpStep = arpStep + 1;

      if (arpStep >= 4) {
        arpStep = 0;
        nextStep();
      }
    } else {
      nextStep();
    }
  }
}

void nextStep() {
  currentStep = currentStep + 1;

  if (currentStep >= totalSteps) {
    currentStep = 0;
  }

  checkLeds();
  Serial.println(currentStep);
}

void playNote() {
  float frequency = noteFrequency[currentStep];

  if (arpState == HIGH) {
    frequency = frequency * pow(2, arpNotes[arpStep] / 12.0);
  }

  waveform1.frequency(frequency);
  waveform1.amplitude(0.2);
}

void checkLeds() {
  for (int i = 0; i < numPixels; i++) {
    if (i == currentStep) {
      neopixel.setPixelColor(i, 0, 0, 100);
    } else {
      neopixel.setPixelColor(i, 0, 0, 0);
    }
  }

  neopixel.show();
}