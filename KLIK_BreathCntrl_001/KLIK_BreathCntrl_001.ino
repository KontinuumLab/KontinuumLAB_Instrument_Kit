
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



// For wind instruments MIDI output values:
int breath;
int lastBreath;

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
  pinMode(aPin[9], INPUT_PULLUP);
  if(!digitalRead(aPin[9]) == 1){
    volumeCC = 7;
  }
  
//  Serial.begin(9600);


  

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
  breathCntrl();
  
}

void breathCntrl(){
  int i;
  
  // Save previous breath sensor raw value, then read and filter new value:
  lastAPinVal[10]= aPinVal[10];
  aPinVal[10] = analogRead(aPin[10]);
  aPinVal[10] = expFilter(aPinVal[10], lastAPinVal[10], 0.01);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreath = breath;
  breath = map(aPinVal[10], aPinMin[10], aPinMax[10], 0, 127);
//  Serial.println(breathOut);

  // Limit output to MIDI range:
  breath = constrain(breath, 0, 127);

//  Serial.println(breathOut);
  // If breath sensor output value has changed: 
  if(breath != lastBreath){
//    Serial.println(breathOut);
    // Send CC2 volume control:
    usbMIDI.sendControlChange(volumeCC, breath, MIDIchannel);

  }  
}

