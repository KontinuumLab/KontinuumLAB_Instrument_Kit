/*
    KontinuumLAB Instrument Kit - "Ocarina" individual instrument sketch
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

// Capacitive touch pins:
const int touchPin[11] = {0, 1, 3, 4, 16, 17, 18, 19, 22, 23, 15};
const int touchPinArrayLength = 11;

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


int touchPinVal[11];
uint16_t touchPinRawVal[11];
int lastTouchPinVal[11];
uint16_t lastTouchPinRawVal[11];

uint16_t touchPinMin[11];
uint16_t touchPinMax[11];


// For wind instruments MIDI output values:
int breath;
int lastBreath;

int currentNote;
int lastNote;

int volumeCC = 2;

// Pitch bend reading values:
int pitchBend;
int lastPitchBend;

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

  
  // All instruments output on alternative MIDI channel:
  if(!digitalRead(optionPin2) == 1){
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

  
  Serial.print(modeSetting);
  Serial.print("_");
  Serial.print(instrumentSetting);
}



//--------------------------------------------------------------//
//----------------------MAIN FUNCTION---------------------------//
//--------------------------------------------------------------//

void loop() {
  calibrationCheck();
  instr_Oca();
}



int keyPhase;
int correct;
int newNote;

int octave = 0;



// -----------------------------------------------------------------
// Simple ocarina with 4 capacitive keys and a CNY70 breathSensor:
// -----------------------------------------------------------------

int activeOcarinaKeys[4];
const int ocarinaKeyThreshold = 30;


int ocarinaOctave = 0;

const int ocarinaKeys[5] = {0, 4, 5, 6, 7};

const int ocarinaFingerings[14][8] = {
  {100, 1, 2, 3, 4, 100, 60}, // C
  {4, 100, 1, 2, 3, 100, 59}, // B
  {2, 100, 1, 3, 4, 100, 58}, // Bb
  {1, 100, 2, 3, 4, 100, 57}, // A
  {1, 4, 100, 2, 3, 100, 56}, // Ab
  {1, 3, 100, 2, 4, 100, 55}, // G
  {1, 3, 4, 100, 2, 100, 54}, // F#
  {1, 2, 100, 3, 4, 100, 53}, // F
  {1, 2, 4, 100, 3, 100, 52}, // E
  {2, 3, 100, 1, 4, 100, 51}, // D#
  {1, 2, 3, 100, 4, 100, 50}, // D
  {2, 3, 4, 100, 1, 100, 49}, // C#
  {1, 2, 3, 4, 100, 100, 48}  // C  
};

void instr_Oca(){
  int i;
  // Save previous breath sensor raw value, then read and filter new value:
  lastAPinVal[10]= aPinVal[10];
  aPinVal[10] = analogRead(aPin[10]);
  aPinVal[10] = expFilter(aPinVal[10], lastAPinVal[10], 0.01);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreath = breath;
  breath = map(aPinVal[10], aPinMin[10], aPinMax[10], 0, 127);
//  Serial.println(aPinVal[10]);

  // Limit output to MIDI range:
  breath = constrain(breath, 0, 127);
  
  if(breath >= 100){
    octave = 12;
    breath = breath - 27;
  }
  else{
    octave = 0;
  }
//  Serial.println(breathOut);
  // If breath sensor output value has changed: 
  if(breath != lastBreath){
//    Serial.println(breathOut);
    // Send CC2 volume control:
    usbMIDI.sendControlChange(volumeCC, breath, MIDIchannel);
    // If breath sensor recently DEactivated, send note off message:
    if(breath == 0){
      usbMIDI.sendControlChange(123, 0, MIDIchannel);
    }
    // Else if breath sensor recently activated, send note on message:
    else if(breath != 0 && lastBreath == 0){
      usbMIDI.sendNoteOn(currentNote, 127, MIDIchannel);
    }
  }
  
  //------------- READING KEYS-----------------//

  // Key reading for loop is only done if breath sensor output is NOT equal to zero:
  if(breath != 0){ 
    // Main key reading for loop. "i" is sensor number:
    for(i = 1; i < 5; i++){
      // Remember last key reading:
      lastTouchPinRawVal[ocarinaKeys[i]] = touchPinRawVal[ocarinaKeys[i]];
      // Read sensor at current position:
      touchPinRawVal[ocarinaKeys[i]] = touchRead(touchPin[ocarinaKeys[i]]);
      
      // Filter reading for smoothness:
      touchPinRawVal[ocarinaKeys[i]] = expFilter(touchPinRawVal[ocarinaKeys[i]], lastTouchPinRawVal[ocarinaKeys[i]], 0.01);
//      Serial.print(touchPinRawVal[ocarinaKeys[i]]);
//      Serial.print(" - ");
      // Map reading to MIDI range:
      touchPinVal[ocarinaKeys[i]] = map(touchPinRawVal[ocarinaKeys[i]], touchPinMin[ocarinaKeys[i]], touchPinMax[ocarinaKeys[i]], 0, 127);
      // Limit reading to MIDI range:
      touchPinVal[ocarinaKeys[i]] = constrain(touchPinVal[ocarinaKeys[i]], 0, 127);
      Serial.print(touchPinVal[ocarinaKeys[i]]);
      Serial.print(" - ");
      // Activate "activeKeys" at current position, if reading is higher than threshold:
      if(touchPinVal[ocarinaKeys[i]] > ocarinaKeyThreshold){
        activeOcarinaKeys[i] = 1;
      }
      // Else, DEactivate "activeKeys" at current position:
      else{
        activeOcarinaKeys[i] = 0;
      }
//      Serial.print(activeOcarinaKeys[i]);
//      Serial.print(" - ");
    }
    Serial.println();

    //-------------KEY ANALISYS-------------------//
  
    // Variables for the fingering-analysis section:
    correct = 0;
    newNote = 0;
    lastNote = currentNote;

//    // If octave key is activated, add 12 semitones to final result, else add zero:
//    if(activeRecorderKeys[recorderOctaveSensor] == 1){
//      octave = 0;
//    }
//    else{
//      octave = 12;
//    }
    
    // Main fingering-analysis for-loop.
    // "n" = fingering array, "i" = position within the fingering loop:
    int n;
    for(n = 0; n < 13; n++){
      // During first keyPhase, check if the requested sensor is active. 
      keyPhase = 0;
      for(i = 0; i < 7; i++){
        if(ocarinaFingerings[n][i] != 100){
          // If active sensors coincide with "fingerings" array requirements: "correct" = yes.
          // Else: "correct" = no and exit current "fingering" array.
          if(keyPhase == 0){  // Check "closed" keys
            if(activeOcarinaKeys[ocarinaFingerings[n][i]] == 1){
              correct = 1;
            }
            else{
              correct = 0;
              break;
            }
          }
          // During second keyPhase, check 
          else if(keyPhase == 1){ // Check "open" keys
            if(activeOcarinaKeys[ocarinaFingerings[n][i]] == 0){
              correct = 1;
            }
            else{
              correct = 0;
              break;
            }
          }       
        }
        // When a "100" is encountered, add 1 to keyPhase, except if keyPhase is 1, exit loop:
        else if(ocarinaFingerings[n][i] == 100){
          if(keyPhase == 1){
            break;
          }
          else{
            keyPhase ++;
          }
        }
      }
      // if "correct" is still yes after all loops finish, then a "fingerings" array has perfectly coincided with
      // current situation. Set "currentNote to the note defined at the end of that array:
      if(correct == 1){
        currentNote = ocarinaFingerings[n][6];
        break;
      }
    }
    // If correct is 1, then there is a new note to report, so set "newNote" to 1, and set "currentNote" to
    // currentNote plus octave: 
    if(correct == 1){
      currentNote = currentNote + octave;
      if(currentNote != lastNote){
        newNote = 1;
      }
    }
    // If newNote is a yes, then send MIDI messages. lastNote off and currentNote On
    if(newNote == 1){
//      Serial.print("Note: ");
//      Serial.println(currentNote);
//      delay(1000);
      usbMIDI.sendNoteOn(lastNote, 0, MIDIchannel);
      usbMIDI.sendNoteOn(currentNote, 127, MIDIchannel);
    }
  }

}

