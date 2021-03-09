/*
    KontinuumLAB Instrument Kit - "Melodica" individual instrument sketch
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

// Multiplexer pins:
const int mux1SensorPin = 0;
const int mux2SensorPin = 15;

const int muxPin1 = 12;
const int muxPin2 = 11;
const int muxPin3 = 10;
const int muxPin4 = 9;

const int muxEnablePin = 14;

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


// For wind instruments MIDI output values:
int breath;
int lastBreath;

int currentNote;
int lastNote;

// Other stuff:
int keyboardSize;

int volumeCC = 2;

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


// Melodica variables:

// Currently pressed keys:
int activeKeyboardSensors[33];
// Previously pressed keys (for melodica):
int lastActiveKeyboardSensors[33];


// MIDI note numbers
int transposeOffset = 47;

int keyboardNotes16[17] = {47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62};
int keyboardNotes32[33] = {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72};

int noteOnArray[33];


//Include rest of sketch:
#include "KLIK_Memory.h"
#include "KLIK_Calibration.h"



void setup() {
  pinMode(modePin0, INPUT_PULLUP);
  pinMode(modePin1, INPUT_PULLUP);
  
  pinMode(instrPin0, INPUT_PULLUP);
  pinMode(instrPin1, INPUT_PULLUP);
  pinMode(instrPin2, INPUT_PULLUP);

  pinMode(optionPin0, INPUT_PULLUP);
  pinMode(optionPin1, INPUT_PULLUP);
  pinMode(optionPin2, INPUT_PULLUP);
  
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

  optionPin0Val = !digitalRead(optionPin0);
  optionPin1Val = !digitalRead(optionPin1);
  optionPin2Val = !digitalRead(optionPin2);
  

  bitWrite(modeSetting, 0, modePin0Val);
  bitWrite(modeSetting, 1, modePin1Val);

  bitWrite(instrumentSetting, 0, instrPin0Val);
  bitWrite(instrumentSetting, 1, instrPin1Val);
  bitWrite(instrumentSetting, 2, instrPin2Val);

  
  pinMode(aPin[5], INPUT_PULLUP);
  pinMode(aPin[6], INPUT_PULLUP);
  int tempOctave1 = 12 * !digitalRead(aPin[5]);
  int tempOctave2 = 12 * !digitalRead(aPin[6]);
  int tempOctave = 0 - tempOctave1 + tempOctave2;
  for(int i = 0; i < 16; i++){
    keyboardNotes16[i] = keyboardNotes16[i] + tempOctave;
    keyboardNotes32[i] = keyboardNotes32[i] + tempOctave;
    keyboardNotes32[i+16] = keyboardNotes32[i+16] + tempOctave;
  }

  // Prepare mux sensor pins for digitalRead:
  pinMode(mux1SensorPin, INPUT_PULLUP);
  pinMode(mux2SensorPin, INPUT_PULLUP);
  
  if(optionPin0Val == 1){
    keyboardSize = 32;
  }
  else{
    keyboardSize = 16;
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
  
  Serial.begin(9600);
  // Read "Serial" pin. If "on" then start Serial.?

  

  // Read saved settings from memory:
  delay(1000);
  loadSettings();

 
}



//--------------------------------------------------------------//
//----------------------MAIN FUNCTION---------------------------//
//--------------------------------------------------------------//

void loop() {

  calibrationCheck();
  instr_Melo();
}






// melodica type keyboard instrument with 16/32 DIGITAL keys. No velocity, breath sensor volume control.
void instr_Melo(){
  int i;
  // Save previous breath sensor raw value, then read and filter new value:
  lastAPinVal[10]= aPinVal[10];
  aPinVal[10] = analogRead(aPin[10]);
  aPinVal[10] = expFilter(aPinVal[10], lastAPinVal[10], 0.005);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreath = breath;
  breath = map(aPinVal[10], aPinMin[10], aPinMax[10], 0, 127);
//  Serial.println(aPinVal[10]);

  // Limit output to MIDI range:
  breath = constrain(breath, 0, 127);




//  Serial.println(breath);
  // If breath sensor output value has changed: 
  if(breath != lastBreath){
//    Serial.println(breathOut);
    // Send CC2 volume control:
    usbMIDI.sendControlChange(volumeCC, breath, MIDIchannel);
    // If breath sensor recently DEactivated, send note off message:
    if(breath == 0){
//      if(keyboardSize == 16){
//        for(i = 0; i < 16; i++){
//          if(activeKeyboardSensors[i] == 1){
//            usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
//          }
//        }
//      }
//      else if(keyboardSize == 32){
//        for(i = 0; i < 32; i++){
//          if(activeKeyboardSensors[i] == 1){
//            usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
//          }
//        }
//      }
      usbMIDI.sendControlChange(123, 0, MIDIchannel);
    }
    // Else if breath sensor recently activated, send note on message:
    else if(breath != 0 && lastBreath == 0){
      if(keyboardSize == 16){
        for(i = 0; i < 16; i++){
          if(activeKeyboardSensors[i] == 1){
            usbMIDI.sendNoteOn(keyboardNotes16[i], 127, MIDIchannel);
          }
        }
      }
      else if(keyboardSize == 32){
        for(i = 0; i < 32; i++){
          if(activeKeyboardSensors[i] == 1){
            usbMIDI.sendNoteOn(keyboardNotes32[i], 127, MIDIchannel);
          }
        }
      }
    }
  }

  
  if(keyboardSize == 32){
//      digitalWrite(muxPin1, bitRead(0, 0)); 
//      digitalWrite(muxPin2, bitRead(0, 1));
//      digitalWrite(muxPin3, bitRead(0, 2));
//      digitalWrite(muxPin4, bitRead(0, 3));
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

      
      digitalWrite(muxPin1, bitRead(i, 0)); 
      digitalWrite(muxPin2, bitRead(i, 1));
      digitalWrite(muxPin3, bitRead(i, 2));
      digitalWrite(muxPin4, bitRead(i, 3));
      delay(1);
      lastActiveKeyboardSensors[i] = activeKeyboardSensors[i];
      lastActiveKeyboardSensors[i+16] = activeKeyboardSensors[i+16];
      activeKeyboardSensors[i] = !digitalRead(mux1SensorPin);
      activeKeyboardSensors[i+16] = !digitalRead(mux2SensorPin);     

      if(breath != 0){
        if(activeKeyboardSensors[i] == 0 && lastActiveKeyboardSensors[i] == 1){
          usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
        }
        else if(activeKeyboardSensors[i] == 1 && lastActiveKeyboardSensors[i] == 0){
          usbMIDI.sendNoteOn(keyboardNotes32[i], 127, MIDIchannel);
        }
        if(activeKeyboardSensors[i+16] == 0 && lastActiveKeyboardSensors[i+16] == 1){
          usbMIDI.sendNoteOn(keyboardNotes32[i+16], 0, MIDIchannel);
        }
        else if(activeKeyboardSensors[i+16] == 1 && lastActiveKeyboardSensors[i+16] == 0){
          usbMIDI.sendNoteOn(keyboardNotes32[i+16], 127, MIDIchannel);
        }
      }
//      Serial.print(activeKeyboardSensors[i]);
//      Serial.print(" - ");
      
    }
//  Serial.println();
  }
  else if(keyboardSize == 16){
//      digitalWrite(muxPin1, bitRead(0, 0)); 
//      digitalWrite(muxPin2, bitRead(0, 1));
//      digitalWrite(muxPin3, bitRead(0, 2));
//      digitalWrite(muxPin4, bitRead(0, 3));
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

      digitalWrite(muxPin1, bitRead(i, 0)); 
      digitalWrite(muxPin2, bitRead(i, 1));
      digitalWrite(muxPin3, bitRead(i, 2));
      digitalWrite(muxPin4, bitRead(i, 3));
      delay(1);
      lastActiveKeyboardSensors[i] = activeKeyboardSensors[i];
      activeKeyboardSensors[i] = !digitalRead(mux1SensorPin);      
      if(breath != 0){
        if(activeKeyboardSensors[i] == 0 && lastActiveKeyboardSensors[i] == 1){
          usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
        }
        else if(activeKeyboardSensors[i] == 1 && lastActiveKeyboardSensors[i] == 0){
          usbMIDI.sendNoteOn(keyboardNotes16[i], 127, MIDIchannel);
        }
      }
//      Serial.print(activeKeyboardSensors[i]);
//      Serial.print(" - ");
      
    }
//  Serial.println();
  }
 

}

