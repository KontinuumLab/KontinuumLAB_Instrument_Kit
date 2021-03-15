/*
    KontinuumLAB Instrument Kit - "4-string" MIDI string instrument individual instrument sketch
    Copyright (C) 2021 Jeppe Rasmussen - KontinuumLAB
    www.kontinuumlab.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <EEPROM.h>

//--------- PINS -----------//

// Settings pins:
const int optionPin0 = 1;
const int optionPin1 = 3;
const int optionPin2 = 4;

const int instrPin0 = 2;
const int instrPin1 = 5;
const int instrPin2 = 6;

const int modePin0 = 7;
const int modePin1 = 8;

const int ledPin = 13;
const int calPin = 26;


// Capacitive touch pins:
const int touchPin[11] = {0, 1, 3, 4, 16, 17, 18, 19, 22, 23, 15};
const int touchPinArrayLength = 11;

// Multiplexer pins:
const int mux1SensorPin = 0;
const int mux2SensorPin = 15;

const int muxPin1 = 12;
const int muxPin2 = 11;
const int muxPin3 = 10;
const int muxPin4 = 9;

const int muxEnablePin = 14;


// String instrument variables:
  
int rhSectionPins[4] = {4, 5, 6, 7};
int currentStringMax[4] = {0, 0, 0, 0};

int fretSensorThreshold = 70;
int stringSensorThreshold = 40;

int stringNoteValues[4];
int orgStringNoteValues[4] = {28, 33, 38, 43};

int ukuleleStringNoteValues[4] = {67, 60, 64, 69};
int bassStringNoteValues[4] = {40, 45, 50, 55};

int activeStringSensors[32];

int firstStringLoop = 1;

int strumTimer = 2;



//-------- SENSOR VARIABLES --------//

// SETTINGS:
int optionPin0Val;
int optionPin1Val;
int optionPin2Val;

int instrPin0Val;
int instrPin1Val;
int instrPin2Val;
int instrumentSetting = 0;

int modePin0Val;
int modePin1Val;
int modeSetting = 0;


// SENSORS VALUES:

int touchPinVal[11];
uint16_t touchPinRawVal[11];
int lastTouchPinVal[11];
uint16_t lastTouchPinRawVal[11];

uint16_t touchPinMin[11];
uint16_t touchPinMax[11];


int mux1Val[16];
uint16_t mux1RawVal[16];
int lastMux1Val[16];
uint16_t lastMux1RawVal[16];

uint16_t mux1Min[16];
uint16_t mux1Max[16];

int mux2Val[16];
uint16_t mux2RawVal[16];
int lastMux2Val[16];
uint16_t lastMux2RawVal[16];

uint16_t mux2Min[16];
uint16_t mux2Max[16];


// Other stuff:

unsigned int ledStart = 0;
unsigned int ledTimer = 0;

int MIDIchannel = 1;

//------------ Generic Functions------------------


// Error value for the exponential filter:
float error;

// Helper function for the exponential filter function:
float snapCurve(float x){
  float y = 1.0 / (x + 1.0);
  y = (1.0 - y) * 2.0;
  if(y > 1.0) {
    return 1.0;
  }
  return y;
}

// Main exponential filter function. Input "snapMult" = speed setting. 0.001 = slow / 0.1 = fast:
int expFilter(int newValue, int lastValue, float snapMult){
  unsigned int diff = abs(newValue - lastValue);
  error += ((newValue - lastValue) - error) * 0.4;
  float snap = snapCurve(diff * snapMult);
  float outputValue = lastValue;
  outputValue  += (newValue - lastValue) * snap;
  return (int)outputValue;
}


// Function for accessing individual multiplexer sensors. Input: mux number, sensor number: 
int readSingleCap(int mux, int number){
  if(mux == 1){
    digitalWrite(muxPin1, bitRead(number, 0)); 
    digitalWrite(muxPin2, bitRead(number, 1));
    digitalWrite(muxPin3, bitRead(number, 2));
    digitalWrite(muxPin4, bitRead(number, 3));
    uint16_t value = touchRead(mux1SensorPin);
    return value;
  }
  else{
    uint16_t value = touchRead(mux2SensorPin);
    return value;
  }
}

// DEBOUNCE FUNCTION FOR BUTTONS:
int debounce(int pin){
  int temp = !digitalRead(pin);
  if(temp == 1){
    delay(2);
    temp = !digitalRead(pin);
    return(temp);
  }
  else{
    return 0;
  }
}


//Include rest of sketch:
#include "KLIK_Memory.h"
#include "KLIK_Calibration.h"

void setup() {
  pinMode(modePin0, INPUT_PULLUP);
  pinMode(modePin1, INPUT_PULLUP);
  
  pinMode(instrPin0, INPUT_PULLUP);
  pinMode(instrPin1, INPUT_PULLUP);
  pinMode(instrPin2, INPUT_PULLUP);

  pinMode(calPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  
  pinMode(muxPin1, OUTPUT);
  pinMode(muxPin2, OUTPUT);
  pinMode(muxPin3, OUTPUT);
  pinMode(muxPin4, OUTPUT);

  pinMode(muxEnablePin, OUTPUT);
  digitalWrite(muxEnablePin, LOW);

  modePin0Val = !digitalRead(modePin0);
  modePin1Val = !digitalRead(modePin1);

  instrPin0Val = !digitalRead(instrPin0);
  instrPin1Val = !digitalRead(instrPin1);
  instrPin2Val = !digitalRead(instrPin2);


  bitWrite(modeSetting, 0, modePin0Val);
  bitWrite(modeSetting, 1, modePin1Val);

  bitWrite(instrumentSetting, 0, instrPin0Val);
  bitWrite(instrumentSetting, 1, instrPin1Val);
  bitWrite(instrumentSetting, 2, instrPin2Val);

  pinMode(optionPin0, INPUT_PULLUP);
  pinMode(optionPin1, INPUT_PULLUP);
  pinMode(optionPin2, INPUT_PULLUP);
  optionPin0Val = !digitalRead(optionPin0);
  optionPin1Val = !digitalRead(optionPin1);
  optionPin2Val = !digitalRead(optionPin2);


  

  /////////For all modes:://///////
  
  // All instruments output on alternative MIDI channel:
  if(optionPin2Val == 1){
    MIDIchannel = 2;
  }
  
  Serial.begin(9600);

  

  // Read saved settings from memory:
  delay(1000);
  loadSettings();

}



//--------------------------------------------------------------//
//----------------------MAIN FUNCTION---------------------------//
//--------------------------------------------------------------//

void loop() {

  calibrationCheck();
  fourString();
}



void fourString() {
    
  int i;
  int j;
  
  if(firstStringLoop == 1){
    if(instrumentSetting == 0){
      for(i = 0; i < 4; i++){
        orgStringNoteValues[i] = bassStringNoteValues[i];
      }
    }
    else if(instrumentSetting == 1){
      for(i = 0; i < 4; i++){
        orgStringNoteValues[i] = ukuleleStringNoteValues[i];
      }
    }
    firstStringLoop = 0;      
  }

  // Main for loop. "i" is the sensor number:
  for(i = 0; i < 16; i++){
    // Remember the last sensor reading at this position:    
    lastMux1Val[i] = mux1Val[i];
    lastMux1RawVal[i] = mux1RawVal[i];
    // Read sensor at this position:
    mux1RawVal[i] = readSingleCap(1, i);
//    Serial.print(mux1RawVal[i]);
//    Serial.print(" - ");
    // Filter reading for stability and map to MIDI range:
//    mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.005); 
    mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
    mux1Val[i] = constrain(mux1Val[i], 0, 127);
    if(mux1Val[i] > fretSensorThreshold){
      activeStringSensors[i] = 1;
    }
    else{
      activeStringSensors[i] = 0;
    }
    if(optionPin0Val == 1){
      lastMux2Val[i] = mux2Val[i];
      lastMux2RawVal[i] = mux2RawVal[i];
      // Read sensor at this position:
      mux2RawVal[i] = readSingleCap(2, i);
  //    Serial.print(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
//      mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.005); 
      mux2Val[i] = map(mux2RawVal[i], mux2Min[i], mux2Max[i], 0, 127);
      mux2Val[i] = constrain(mux2Val[i], 0, 127);
      if(mux2Val[i] > fretSensorThreshold){
        activeStringSensors[i+16] = 1;
      }
      else{
        activeStringSensors[i+16] = 0;
      }
    }

    // Inserted Strum function:
    if(i % strumTimer == 0){
      for(j = 0; j < 4; j++){
        lastTouchPinRawVal[rhSectionPins[j]] = touchPinRawVal[rhSectionPins[j]];
        lastTouchPinVal[rhSectionPins[j]] = touchPinVal[rhSectionPins[j]];
        touchPinRawVal[rhSectionPins[j]] = touchRead(touchPin[rhSectionPins[j]]);
        touchPinRawVal[rhSectionPins[j]] = expFilter(touchPinRawVal[rhSectionPins[j]], lastTouchPinRawVal[rhSectionPins[j]], 0.005);
        touchPinVal[rhSectionPins[j]] = map(touchPinRawVal[rhSectionPins[j]], touchPinMin[rhSectionPins[j]], touchPinMax[rhSectionPins[j]], 0, 127);
        touchPinVal[rhSectionPins[j]] = constrain(touchPinVal[rhSectionPins[j]], 0, 127);
        if(touchPinVal[rhSectionPins[j]] > stringSensorThreshold){
          if(touchPinVal[rhSectionPins[j]] > currentStringMax[j]){
            currentStringMax[j] = touchPinVal[rhSectionPins[j]];
          }
          touchPinVal[rhSectionPins[j]] = 1;
        }
        else{
          touchPinVal[rhSectionPins[j]] = 0;
        }
//        Serial.print(touchPinVal[rhSectionPins[j]]);
//        Serial.print(" - ");
      }
//      Serial.println();
      
      for(j = 0; j < 4; j++){
        if(activeStringSensors[j + 28] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 8;
        }
        else if(activeStringSensors[j + 24] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 7;
        }
        else if(activeStringSensors[j + 20] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 6;
        }
        else if(activeStringSensors[j + 16] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 5;
        }
        else if(activeStringSensors[j + 12] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 4;
        }
        else if(activeStringSensors[j + 8] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 3;
        }
        else if(activeStringSensors[j + 4] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 2;
        }
        else if(activeStringSensors[j] == 1){
          stringNoteValues[j] = orgStringNoteValues[j] + 1;
        }
        else{
          stringNoteValues[j] = orgStringNoteValues[j];
        }
        if(lastTouchPinVal[rhSectionPins[j]] == 1 && touchPinVal[rhSectionPins[j]] == 0){
          usbMIDI.sendNoteOn(stringNoteValues[j], currentStringMax[j], MIDIchannel);
          currentStringMax[j] = 0;
        }
        if(lastTouchPinVal[rhSectionPins[j]] == 0 && touchPinVal[rhSectionPins[j]] == 1){
          usbMIDI.sendNoteOn(stringNoteValues[j], 0, MIDIchannel);
        }
      }    
    }
//    Serial.print(mux2Val[i]);
//    Serial.print(" - ");
  }
//  Serial.println();

//  delay(5);
}
