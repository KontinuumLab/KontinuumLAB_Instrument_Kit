/*
    KontinuumLAB Instrument Kit - "Membrane" individual instrument sketch
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

// Analog sensor pins:
const int aPin[11] = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
const int aPinArrayLength = 11;


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
int aPinVal[11];
uint16_t aPinRawVal[11];
int lastAPinVal[11];
uint16_t lastAPinRawVal[11];

uint16_t aPinMin[11];
uint16_t aPinCent[11];
uint16_t aPinMax[11];

// Other stuff:
int numberOfDrums;
int drumsVolume;
int volumeCC = 2;

unsigned int ledStart = 0;
unsigned int ledTimer = 0;

int MIDIchannel = 1;




// Membrane specific variables: 


int drumMIDIchannel[8] = {2, 3, 4, 5, 6, 7, 8, 9};

int drumPins[8] = {1, 2, 3, 4, 5, 6, 7, 8};

int midiNote[8] = {40, 43, 45, 47, 50, 52, 55, 57};
//int velMidiNote[8] = {35, 36, 41, 38, 44, 37, 42, 46};

int preparingVel[8];



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



// Set membrane options:
  pinMode(optionPin0, INPUT_PULLUP);
  pinMode(optionPin1, INPUT_PULLUP);
  pinMode(optionPin2, INPUT_PULLUP);
  optionPin0Val = !digitalRead(optionPin0);
  optionPin1Val = !digitalRead(optionPin1);
  optionPin2Val = !digitalRead(optionPin2);
  if(instrumentSetting == 0){
    drumsVolume = 0;
  }
  else if(instrumentSetting == 1){
    drumsVolume = 1;
  }
  
  if(optionPin0Val == 0 && optionPin1Val == 0){
    numberOfDrums = 1;
  }
  else if(optionPin0Val == 1 && optionPin1Val == 0){
    numberOfDrums = 4;
  }
  else if(optionPin0Val == 0 && optionPin1Val == 1){
    numberOfDrums = 6;
  }
  else if(optionPin0Val == 1 && optionPin1Val == 1){
    numberOfDrums = 8;
  }

  /////////For all modes:://///////
  
  // All instruments output on alternative MIDI channel:
  if(optionPin2Val == 1){
    MIDIchannel = 2;
  }
//  All volume control outputs CC#7 instead of CC#2:
  pinMode(aPin[9], INPUT_PULLUP);
  if(!digitalRead(aPin[9]) == 1){
    volumeCC = 7;
  }
  
//  Serial.begin(9600);
  // Read "Serial" pin. If "on" then start Serial.?

  

  // Read saved settings from memory:
  delay(1000);
  loadSettings();

  
//  Serial.print(modeSetting);
//  Serial.print("_");
//  Serial.print(instrumentSetting);
}



//--------------------------------------------------------------//
//----------------------MAIN FUNCTION---------------------------//
//--------------------------------------------------------------//

void loop() {

  calibrationCheck();
  
  membrane();
}



// Analog CNY70 (membrane) percussion instrument, with selectable number of drums.

void membrane(){
  int i;
  
  // Main for loop. "i" is the sensor number:

  // velocity output section:
  if(drumsVolume == 0){
    for(i = 0; i < numberOfDrums; i++){
      // Remember the last sensor output:
      lastAPinVal[drumPins[i]] = aPinVal[drumPins[i]];
      lastAPinRawVal[drumPins[i]] = aPinRawVal[drumPins[i]];
      
      // read the sensor at the current position and filter for smoothness:
      aPinRawVal[drumPins[i]] = analogRead(aPin[drumPins[i]]);
      aPinRawVal[drumPins[i]] = expFilter(aPinRawVal[drumPins[i]], lastAPinRawVal[drumPins[i]], 0.01);
  
  //    Serial.print(aPinRawVal[drumPins[i]]);
  //    Serial.print(" - ");
      
      // map and limit the reading to MIDI range:
      aPinVal[drumPins[i]] = map(aPinRawVal[drumPins[i]], aPinMin[drumPins[i]], aPinMax[drumPins[i]], 0, 127);
      aPinVal[drumPins[i]] = constrain(aPinVal[drumPins[i]], 0, 127);
//      Serial.print(aPinVal[drumPins[i]]);
//      Serial.print(" - ");
  
      // Compare sensor output to previous value. If new value at current 
      // position, Send CC message for volume control:
      if(aPinVal[drumPins[i]] != lastAPinVal[drumPins[i]] || preparingVel[i] == 1){ 
  //      Serial.println(sensorOut[i]);
  //      usbMIDI.sendControlChange(volumeCC, aPinVal[drumPins[i]], drumMIDIchannel[i]);
        // Check for recently activated sensor. If activated in this loop,
        // activate "preparingVel" at this position, and save current time:
        if(lastAPinVal[drumPins[i]] == 0 && aPinVal[drumPins[i]] != 0){
          preparingVel[i] = 1;
        }
        // If preparingVel was activated at current position but in a 
        // previous loop, calculate time since activation:
        else if(preparingVel[i] == 1){
        // Calculate, map and limit velocity,
        // and then send a "note on" message with the values from current pos:
  //        int vel = analogRead(aPinVal[drumPins[i]]);
  //        int sensorVelocity = aPinVal[drumPins[i]];
  //        if(sensorVelocity[i] < 0){
  //          sensorVelocity[i] = 0;
  //        }
  //        else if(sensorVelocity[i] > 127){
  //          sensorVelocity[i] = 127;
  //        }
  //          Serial.print("noteOn: ");
  //          Serial.print(i);
  //          Serial.print(" - ");
  //          Serial.println(sensorVelocity[i]);
          usbMIDI.sendNoteOn(midiNote[i], aPinVal[drumPins[i]], MIDIchannel);
          preparingVel[i] = 0;
  //          delay(1000);
          }
        // If sensor at current position has been deactivated during current loop,
        // Send "note off" message with values from current position:
        if(aPinVal[drumPins[i]] == 0 && lastAPinVal[drumPins[i]] != 0){
  //        Serial.print("noteOff - ");
  //        Serial.print("Last noteOn: ");
  //        Serial.println(sensorVelocity[i]);
  //        usbMIDI.sendNoteOn(midiNote[i], 0, drumMIDIchannel[0]);
  //        delay(1000);
        }
      }
    }
  }


  // continuous volume output section:
  else if(drumsVolume == 1){
    for(i = 0; i < numberOfDrums; i++){
      // Remember the last sensor output:
      lastAPinVal[drumPins[i]] = aPinVal[drumPins[i]];
      lastAPinRawVal[drumPins[i]] = aPinRawVal[drumPins[i]];
      
      // read the sensor at the current position and filter for smoothness:
      aPinRawVal[drumPins[i]] = analogRead(aPin[drumPins[i]]);
      aPinRawVal[drumPins[i]] = expFilter(aPinRawVal[drumPins[i]], lastAPinRawVal[drumPins[i]], 0.01);
  
  //    Serial.print(aPinRawVal[drumPins[i]]);
  //    Serial.print(" - ");
      
      // map and limit the reading to MIDI range:
      aPinVal[drumPins[i]] = map(aPinRawVal[drumPins[i]], aPinMin[drumPins[i]], aPinMax[drumPins[i]], 0, 127);
      aPinVal[drumPins[i]] = constrain(aPinVal[drumPins[i]], 0, 127);
//      Serial.print(aPinVal[drumPins[i]]);
//      Serial.print(" - ");
  
      // Compare sensor output to previous value. If new value at current 
      // position, Send CC message for volume control:
      if(aPinVal[drumPins[i]] != lastAPinVal[drumPins[i]]){ 
  //      Serial.println(sensorOut[i]);
        usbMIDI.sendControlChange(volumeCC, aPinVal[drumPins[i]], MIDIchannel);
        // Check for recently activated sensor. If activated in this loop,
        // activate "preparingVel" at this position, and save current time:
        if(lastAPinVal[drumPins[i]] == 0 && aPinVal[drumPins[i]] != 0){
          usbMIDI.sendNoteOn(midiNote[i], 127, MIDIchannel);
        }
        // If sensor at current position has been deactivated during current loop,
        // Send "note off" message with values from current position:
        if(aPinVal[drumPins[i]] == 0 && lastAPinVal[drumPins[i]] != 0){
  //        Serial.print("noteOff - ");
  //        Serial.print("Last noteOn: ");
  //        Serial.println(sensorVelocity[i]);
          usbMIDI.sendNoteOn(midiNote[i], 0, MIDIchannel);
  //        delay(1000);
        }
      }
    }
  }
  
//  Serial.println();
  // End main for loop//
  delay(2);
//  Serial.println();
}


