/*
    KontinuumLAB Instrument Kit - "KLIKsophonea" individual instrument sketch
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
const int aPin[2] = {24, 25};
const int aPinArrayLength = 2;

// Capacitive touch pins:
const int touchPin[11] = {0, 1, 3, 4, 16, 17, 18, 19, 22, 23, 15};
const int touchPinArrayLength = 11;

// Multiplexer pins:
const int mux1SensorPin = 0;

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
int aPinVal[2];
uint16_t aPinRawVal[2];
int lastAPinVal[2];
uint16_t lastAPinRawVal[2];

uint16_t aPinMin[2];
uint16_t aPinCent[2];
uint16_t aPinMax[2];


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


// For wind instruments MIDI output values:
int breath;
int lastBreath;

int currentNote;
int lastNote;

// Other stuff:
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


// Function for accessing individual multiplexer sensors. Input: mux number, sensor number: 
int readSingleCap(int number){
  digitalWrite(muxPin1, bitRead(number, 0)); 
  digitalWrite(muxPin2, bitRead(number, 1));
  digitalWrite(muxPin3, bitRead(number, 2));
  digitalWrite(muxPin4, bitRead(number, 3));
  uint16_t value = touchRead(mux1SensorPin);
  return value;
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

  if(modeSetting == 0){
    pinMode(optionPin0, INPUT_PULLUP);
    pinMode(optionPin1, INPUT_PULLUP);
    pinMode(optionPin2, INPUT_PULLUP);
    optionPin0Val = !digitalRead(optionPin0);
    optionPin1Val = !digitalRead(optionPin1);
    optionPin2Val = !digitalRead(optionPin2);
    
    if(instrumentSetting == 0){
      // do nothing....
    }
  }
  

  /////////For all modes:://///////
  
  // All instruments output on alternative MIDI channel:
  if(optionPin2Val == 1){
    MIDIchannel = 2;
  }
//  All volume control outputs CC#7 instead of CC#2:
  pinMode(aPin[0], INPUT_PULLUP);
  if(!digitalRead(aPin[0]) == 1){
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
  
  KLIKsophone();
}


int keyPhase;
int correct;
int newNote;

int octave = 0;

int firstLoop = 1;

const int KLIKsophoneOctaveSensors[3] = {13, 14, 15};

int activeKLIKsophoneKeys[16];

const int KLIKsophoneKeyThreshold = 30;



int looperSensors[6] = {4, 5, 6, 7};
int looperNotes[6] = {48, 50, 52, 55};
int strokeThreshold = 20;
int offThreshold = 5;

int noteOnArray[4];

// Fingering arrays, used to determine which note is currently being played:
// Each array represents a specific note, as stated at the last position of each array. 
// Numbers 0 - 12 represent a specific key, "100" means go to next stage.
// Far left values (until first "100") are keys that HAVE to be pressed to produce the note in question.
// Next series of keys (until next "100") are keys that must NOT be pressed for the note to be valid.

const int KLIKsophoneFingerings[20][14] = {
  {0, 7, 100, 100, 0, 0, 0, 0, 0, 0, 0, 0, 63}, // Eb
  {0, 100, 1, 100, 0, 0, 0, 0, 0, 0, 0, 0, 62}, // D
  {100, 1, 3, 100, 0, 0, 0, 0, 0, 0, 0, 0, 61}, // C#
  {3, 100, 1, 100, 0, 0, 0, 0, 0, 0, 0, 0, 60}, // C
  {1, 100, 2, 3, 100, 0, 0, 0, 0, 0, 0, 0, 59}, // B
  {1, 2, 100, 3, 100, 0, 0, 0, 0, 0, 0, 0, 58}, // Bb
  {1, 3, 7, 100, 4, 100, 0, 0, 0, 0, 0, 0, 58}, // Bb
  {1, 3, 100, 4, 7, 100, 0, 0, 0, 0, 0, 0, 57}, // A
  {1, 3, 4, 5, 100, 8, 9, 100, 0, 0, 0, 0, 56}, // Ab
  {1, 3, 4, 100, 5, 8, 9, 100, 0, 0, 0, 0, 55}, // G
  {1, 3, 4, 9, 100, 8, 100, 0, 0, 0, 0, 0, 54}, // F#
  {1, 3, 4, 8, 100, 9, 100, 0, 0, 0, 0, 0, 53}, // F
  {1, 3, 4, 8, 9, 100, 10, 100, 0, 0, 0, 0, 52}, // E
  {1, 3, 4, 8, 9, 10, 11, 100, 12, 100, 0, 0, 51}, // Eb
  {1, 3, 4, 8, 9, 10, 100, 6, 11, 12, 100, 0, 50}, // D
  {1, 3, 4, 5, 8, 9, 10, 12, 100, 6, 100, 0, 49}, // C#
  {1, 3, 4, 8, 9, 10, 12, 100, 5, 6, 100, 0, 48}, // C
  {1, 3, 4, 6, 8, 9, 10, 100, 12, 100, 0, 0, 47}, // B 
  {1, 3, 4, 6, 8, 9, 10, 12, 100, 100, 0, 0, 46}, // Bb
};




void KLIKsophone(){
  int i;
  int looperPins;
  
  // Save previous breath sensor raw value, then read and filter new value:
  lastAPinVal[1]= aPinVal[1];
  aPinVal[1] = analogRead(aPin[1]);
  aPinVal[1] = expFilter(aPinVal[1], lastAPinVal[1], 0.01);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreath = breath;
  breath = map(aPinVal[1], aPinMin[1], aPinMax[1], 0, 127);
//  Serial.println(breath);

  // Limit output to MIDI range:
  breath = constrain(breath, 0, 127);

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

  looperPins = 4;
    
// Looper controller section
  if(optionPin0Val == 1){
    for(i = 0; i < looperPins; i++){
      lastTouchPinRawVal[looperSensors[i]] = touchPinRawVal[looperSensors[i]];
      lastTouchPinVal[looperSensors[i]] = touchPinVal[looperSensors[i]];
      touchPinRawVal[looperSensors[i]] = touchRead(touchPin[looperSensors[i]]);
      touchPinRawVal[looperSensors[i]] = expFilter(touchPinRawVal[looperSensors[i]], lastTouchPinRawVal[looperSensors[i]], 0.005);
      touchPinVal[looperSensors[i]] = map(touchPinRawVal[looperSensors[i]], touchPinMin[looperSensors[i]], touchPinMax[looperSensors[i]], 0, 127);
      touchPinVal[looperSensors[i]] = constrain(touchPinVal[looperSensors[i]], 0, 127);

      int MIDIchannelExtra = MIDIchannel + 1;
      if(touchPinVal[looperSensors[i]] >= strokeThreshold && lastTouchPinVal[looperSensors[i]] < strokeThreshold && noteOnArray[i] == 0){
        int velocity = touchPinVal[looperSensors[i]];
        usbMIDI.sendNoteOn(looperNotes[i], velocity, MIDIchannelExtra);
        noteOnArray[i] = 1;
      }
      else if(touchPinVal[looperSensors[i]] < offThreshold && lastTouchPinVal[looperSensors[i]] >= offThreshold){
        usbMIDI.sendNoteOn(looperNotes[i], 0, MIDIchannelExtra);
        noteOnArray[i] = 0;
      }
        
//      Serial.print(touchPinVal[looperSensors[i]]);
//      Serial.print(" - ");
    }
//    Serial.println();
  }


    //------------- READING KEYS-----------------//

  // Key reading for loop is only done if breath sensor output is NOT equal to zero:
  if(breath != 0){ 


    // Pitch bend section:
    if(optionPin1Val == 1){
      
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[8] = touchPinRawVal[8];
      lastTouchPinRawVal[9] = touchPinRawVal[9];
      
      touchPinRawVal[8] = touchRead(touchPin[8]);
      touchPinRawVal[9] = touchRead(touchPin[9]);
      touchPinRawVal[8] = expFilter(touchPinRawVal[8], lastTouchPinRawVal[8], 0.01);
      touchPinRawVal[9] = expFilter(touchPinRawVal[9], lastTouchPinRawVal[9], 0.01);
      touchPinVal[8] = map(touchPinRawVal[8], touchPinMin[8], touchPinMax[8], 0, 64);
      touchPinVal[9] = map(touchPinRawVal[9], touchPinMin[9], touchPinMax[9], 0, 63);
      // Limit values to sane range:
      touchPinVal[8] = constrain(touchPinVal[8], 0, 64);
      touchPinVal[9] = constrain(touchPinVal[9], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[9] + touchPinVal[8];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }

    // Main key reading for loop. "i" is sensor number:
    for(i = 0; i < 16; i++){
      // Remember last key reading:
      lastMux1RawVal[i] = mux1RawVal[i];
      // Read sensor at current position:
      mux1RawVal[i] = readSingleCap(i);
      
      // Filter reading for smoothness:
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.01);
//      Serial.print(touchPinRawVal[recorderKeys[i]]);
//      Serial.print(" - ");
      // Map reading to MIDI range:
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      // Limit reading to MIDI range:
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
//      Serial.print(mux1Val[i]);
//      Serial.print(" - ");
      // Activate "activeKeys" at current position, if reading is higher than threshold:
      if(mux1Val[i] > KLIKsophoneKeyThreshold){
        activeKLIKsophoneKeys[i] = 1;
      }
      // Else, DEactivate "activeKeys" at current position:
      else{
        activeKLIKsophoneKeys[i] = 0;
      }
//      Serial.print(activeKLIKsophoneKeys[i]);
//      Serial.print(" - ");
    }
    Serial.println();

    
    //-------------KEY ANALISYS-------------------//
  
    // Variables for the fingering-analysis section:
    correct = 0;
    newNote = 0;
    lastNote = currentNote;

    // If octave key is activated, add 12 semitones to final result, else add zero:
    if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 0){
      octave = 36;
    }
    else if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 1 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 0){
      octave = 24;
    }
    else if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 1 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 1 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 0){
      octave = 12;
    }
    else if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 1 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 0){
      octave = 0;
    }
    else if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 1 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 1){
      octave = -12;
    }
    else if(activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[0]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[1]] == 0 && activeKLIKsophoneKeys[KLIKsophoneOctaveSensors[2]] == 1){
      octave = -24;
    }
    
    // Main fingering-analysis for-loop.
    // "n" = fingering array, "i" = position within the fingering loop:
    int n;
    for(n = 0; n < 19; n++){
      // During first keyPhase, check if the requested sensor is active. 
      keyPhase = 0;
      for(i = 0; i < 16; i++){
        if(KLIKsophoneFingerings[n][i] != 100){
          // If active sensors coincide with "fingerings" array requirements: "correct" = yes.
          // Else: "correct" = no and exit current "fingering" array.
          if(keyPhase == 0){  // Check "closed" keys
            if(activeKLIKsophoneKeys[KLIKsophoneFingerings[n][i]] == 1){
              correct = 1;
            }
            else{
              correct = 0;
              break;
            }
          }
          // During second keyPhase, check 
          else if(keyPhase == 1){ // Check "open" keys
            if(activeKLIKsophoneKeys[KLIKsophoneFingerings[n][i]] == 0){
              correct = 1;
            }
            else{
              correct = 0;
              break;
            }
          }       
        }
        // When a "100" is encountered, add 1 to keyPhase, except if keyPhase is 1, exit loop:
        else if(KLIKsophoneFingerings[n][i] == 100){
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
        currentNote = KLIKsophoneFingerings[n][12];
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

