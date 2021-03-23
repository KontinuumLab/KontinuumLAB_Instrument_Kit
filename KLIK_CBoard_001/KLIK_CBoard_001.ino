
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
const int aPin = 25;
const int octDownPin = 20;
const int octUpPin = 21;

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
int aPinVal;
uint16_t aPinRawVal;
int lastAPinVal;
uint16_t lastAPinRawVal;

uint16_t aPinMin;
uint16_t aPinCent;
uint16_t aPinMax;


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


// For wind instruments MIDI output values:
int breath;
int lastBreath;

int currentNote;
int lastNote;

// Other stuff:
int keyboardSize;
int numberOfDrums;
int drumsVolume;
int keyboardAfterTouch = 1;
int volumeCC = 2;

// Pitch bend reading values:
int pitchBend;
int lastPitchBend;

unsigned int ledStart = 0;
unsigned int ledTimer = 0;

int MIDIchannel = 1;




//################################
// cBoard variables

// Currently pressed keys:
int activeKeyboardSensors[33];
// Previously pressed keys (for melodica):
int lastActiveKeyboardSensors[33];

int keyboardSensorVal[33];
int lastKeyboardSensorVal[33];


int sixSensSensors[6] = {4, 5, 6, 7, 8, 9};
int sixSensNotes[6] = {48, 50, 52, 55, 57, 60};


// MIDI note numbers
int transposeOffset = 47;

int keyboardNotes16[17] = {47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62};
int keyboardNotes32[33] = {41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72};

int noteOnArray[33];

int strokeThreshold = 20;
int volumeThreshold = 1;
int padThreshold = 50;
int offThreshold = 5;

int pitchbendTimer = 3;
int breathTimer = 3;





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

  pinMode(octDownPin, INPUT_PULLUP);
  pinMode(octUpPin, INPUT_PULLUP);
  int tempOctave1 = 12 * !digitalRead(octDownPin);
  int tempOctave2 = 12 * !digitalRead(octUpPin);
  int tempOctave = 0 - tempOctave1 + tempOctave2;
  for(int i = 0; i < 16; i++){
    keyboardNotes16[i] = keyboardNotes16[i] + tempOctave;
    keyboardNotes32[i] = keyboardNotes32[i] + tempOctave;
    keyboardNotes32[i+16] = keyboardNotes32[i+16] + tempOctave;
  }
  for(int i = 0; i < 6; i++){
    sixSensNotes[i] = sixSensNotes[i] + tempOctave;
  }


  if(instrumentSetting == 0 || instrumentSetting == 1 || instrumentSetting == 2 || instrumentSetting == 3 || instrumentSetting == 4){
    pinMode(optionPin0, INPUT_PULLUP);
    pinMode(optionPin1, INPUT_PULLUP);
    pinMode(optionPin2, INPUT_PULLUP);
    optionPin0Val = !digitalRead(optionPin0);
    optionPin1Val = !digitalRead(optionPin1);
    optionPin2Val = !digitalRead(optionPin2);
  }
  else if(instrumentSetting == 5){
    pinMode(optionPin0, INPUT_PULLUP);
    pinMode(optionPin1, INPUT_PULLUP);
    pinMode(optionPin2, INPUT_PULLUP);
    optionPin0Val = !digitalRead(optionPin0);
    optionPin1Val = !digitalRead(optionPin1);
    optionPin2Val = !digitalRead(optionPin2);
    
    // Prepare mux sensor pins for digitalRead:
    pinMode(mux1SensorPin, INPUT_PULLUP);
    pinMode(mux2SensorPin, INPUT_PULLUP);
  }
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
  pinMode(24, INPUT_PULLUP);
  if(!digitalRead(24) == 1){
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
  
  if(instrumentSetting == 0){
    cBoard_vel(); // Capacitive "velocity" keyboard
  }
  else if(instrumentSetting == 1){
    cBoard_vol(); // Capacitive "volume" keyboard
  }
  else if(instrumentSetting == 2){
    cBoard_after(); // Capacitive "velocity + aftertouch" keyboard
  }
  else if(instrumentSetting == 3){
    cBoard_mel(); // Capacitive keyboard (melodica)
  }
  else if(instrumentSetting == 4){
    cBoard_sixSens(); // 6 sensor cap instrument
  }     
}






// Keyboard instrument with 16/32 capacitive keys and velocity output per key.
void cBoard_vel(){
  int i;
  int j;
  // Main for loop. "i" is the sensor number:
  for(i = 0; i < 16; i++){
    
     // Read the extra touchRead pins, and map them to pitchBend values, if that option is activated
    if(optionPin1Val == 1 &&  i % pitchbendTimer == 0){
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[4] = touchPinRawVal[4];
      lastTouchPinRawVal[5] = touchPinRawVal[5];
      
      touchPinRawVal[4] = touchRead(touchPin[4]);
      touchPinRawVal[5] = touchRead(touchPin[5]);
      touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
      touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
      touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
      touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
      // Limit values to sane range:
      touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
      touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }
    // Remember the last sensor reading at this position:    
    lastMux1Val[i] = mux1Val[i];
    lastMux1RawVal[i] = mux1RawVal[i];
    mux1RawVal[i] = readSingleCap(1, i);
    mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.01); 
    mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
    mux1Val[i] = constrain(mux1Val[i], 0, 127);
//    Serial.print(mux1Val[i]);
//    Serial.print(" - ");
    if(keyboardSize == 16){
      if(mux1Val[i] >= strokeThreshold && lastMux1Val[i] < strokeThreshold && noteOnArray[i] == 0){
        int velocity = mux1Val[i];
        usbMIDI.sendNoteOn(keyboardNotes16[i], velocity, MIDIchannel);
        noteOnArray[i] = 1;
      }
      else if(mux1Val[i] < offThreshold && lastMux1Val[i] >= offThreshold){
        usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
        noteOnArray[i] = 0;
      }
    }
  
    else if(keyboardSize == 32){
      lastMux2Val[i] = mux2Val[i];
      lastMux2RawVal[i] = mux2RawVal[i];
      mux2RawVal[i] = readSingleCap(2, i);
      mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.01); 
      mux2Val[i] = map(mux2RawVal[i], mux2Min[i], mux2Max[i], 0, 127);
      mux2Val[i] = constrain(mux2Val[i], 0, 127);
//      Serial.print(mux2Val[i]);
//      Serial.print(" - ");
      
      if(mux1Val[i] >= strokeThreshold && lastMux1Val[i] < strokeThreshold && noteOnArray[i] == 0){
        int velocity = mux1Val[i];
        usbMIDI.sendNoteOn(keyboardNotes32[i], velocity, MIDIchannel);
        noteOnArray[i] = 1;
      }
      else if(mux1Val[i] < offThreshold && lastMux1Val[i] >= offThreshold){
        usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
        noteOnArray[i] = 0;
      }
      if(mux2Val[i] >= strokeThreshold && lastMux2Val[i] < strokeThreshold && noteOnArray[i+16] == 0){
        int velocity = mux2Val[i];
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], velocity, MIDIchannel);
        noteOnArray[i+16] = 1;
      }
      else if(mux2Val[i] < offThreshold && lastMux2Val[i] >= offThreshold){
        noteOnArray[i+16] = 0;
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], 0, MIDIchannel);
      }    
    }
//    Serial.println();
    // Read sensor at this position:

    
//    Serial.print(mux1RawVal[i]);
//    Serial.print(" - ");
    // Filter reading for stability and map to MIDI range:

  }
//  Serial.println();
  
}

// "Continuous" and touch sensitive type keyboard instrument with 16/32 capacitive keys. "Volume" output based on the last pressed key.
void cBoard_vol(){
  int i;
  int j;
  int k;
  
////////////////////////////////////////////
// IF SMALL KEYBOARD:::

  if(keyboardSize == 16){
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

// PITCHBEND SECTION:
    if(optionPin1Val == 1 && i % pitchbendTimer == 0){
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[4] = touchPinRawVal[4];
      lastTouchPinRawVal[5] = touchPinRawVal[5];
      
      touchPinRawVal[4] = touchRead(touchPin[4]);
      touchPinRawVal[5] = touchRead(touchPin[5]);
      touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
      touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
      touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
      touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
      // Limit values to sane range:
      touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
      touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }

      // Remember the last sensor reading at this position:
      lastMux1Val[i] = mux1Val[i];
      lastMux1RawVal[i] = mux1RawVal[i];
      // Read sensor at this position:
      mux1RawVal[i] = readSingleCap(1, i);
  //    Serial.print(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.005); 
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
      
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux1Val[i] >= volumeThreshold && lastMux1Val[i] < volumeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes16[i], 127, MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 15; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i;    
      }
      
      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux1Val[i] < volumeThreshold && lastMux1Val[i] >= volumeThreshold){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 15; j++){
          if(activeKeyboardSensors[j] == i){
            for(int k = j; k < 15; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([15]) to 0:
            activeKeyboardSensors[15] = 0;
          }
        }
      }
      // Print the current sensor reading values to Serial:
    }
    // Send volume control CC using the reading from the last activated sensor:
    if(mux1Val[activeKeyboardSensors[0]] != lastMux1Val[activeKeyboardSensors[0]]){
      usbMIDI.sendControlChange(volumeCC, mux1Val[activeKeyboardSensors[0]], MIDIchannel);
    }
  }

  
////////////////////////////////////////////
// IF LARGE KEYBOARD:::

  else if(keyboardSize == 32){
    lastKeyboardSensorVal[i] = keyboardSensorVal[i];
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

// PITCHBEND SECTION:
    if(optionPin1Val == 1 && i % pitchbendTimer == 0){
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[4] = touchPinRawVal[4];
      lastTouchPinRawVal[5] = touchPinRawVal[5];
      
      touchPinRawVal[4] = touchRead(touchPin[4]);
      touchPinRawVal[5] = touchRead(touchPin[5]);
      touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
      touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
      touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
      touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
      // Limit values to sane range:
      touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
      touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }
      
      // Remember the last sensor reading at this position:
      lastMux1Val[i] = mux1Val[i];
      lastMux1RawVal[i] = mux1RawVal[i];
      // Read sensor at this position:
      mux1RawVal[i] = readSingleCap(1, i);
  //    Serial.println(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.005); 
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
      keyboardSensorVal[i] = mux1Val[i];

      // other mux:
      // Remember the last sensor reading at this position:
      lastMux2Val[i] = mux2Val[i];
      lastMux2RawVal[i] = mux2RawVal[i];
      // Read sensor at this position:
      mux2RawVal[i] = readSingleCap(2, i);
  //    Serial.println(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.005); 
      mux2Val[i] = map(mux2RawVal[i], mux2Min[i], mux2Max[i], 0, 127);
      mux2Val[i] = constrain(mux2Val[i], 0, 127);
      keyboardSensorVal[i+16] = mux2Val[i];



///////////////////////////
      //1st MUX:
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux1Val[i] >= volumeThreshold && lastMux1Val[i] < volumeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i], 127, MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 31; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i;    
      }

      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux1Val[i] < volumeThreshold && lastMux1Val[i] >= volumeThreshold){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 31; j++){
          if(activeKeyboardSensors[j] == i){
            for(int k = j; k < 31; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([15]) to 0:
            activeKeyboardSensors[31] = 0;
          }
        }
      }

///////////////////////////
      //2nd MUX:
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux2Val[i] >= volumeThreshold && lastMux2Val[i] < volumeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], 127, MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 31; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i+16;    
      }
      
      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux2Val[i] < volumeThreshold && lastMux2Val[i] >= volumeThreshold){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 31; j++){
          if(activeKeyboardSensors[j] == i+16){
            for(int k = j; k < 31; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([31]) to 0:
            activeKeyboardSensors[31] = 0;
          }
        }
      }


      
      // Print the current sensor reading values to Serial:
    }
    // Send volume control CC using the reading from the last activated sensor:
    if(keyboardSensorVal[activeKeyboardSensors[0]] != lastKeyboardSensorVal[activeKeyboardSensors[0]]){
      usbMIDI.sendControlChange(volumeCC, keyboardSensorVal[activeKeyboardSensors[0]], MIDIchannel);
    }
  }
  
  // Finish main sensor reading loop//
//  Serial.println();

  // Print current "activeSensors" array to Serial: 
//  for(int k = 0; k < 16; k++){
//    Serial.print(activeKeyboardSensors[k]);
//    Serial.print(" - ");
//  }
//  Serial.println();
//  Serial.println(millis());

  // Read the extra touchRead pins, and map them to pitchBend values, if that option is activated
//  if(optionPin0Val == 1){
//    lastPitchBend = pitchBend;
//    lastTouchPinRawVal[4] = touchPinRawVal[4];
//    lastTouchPinRawVal[5] = touchPinRawVal[5];
//    
//    touchPinRawVal[4] = touchRead(touchPin[4]);
//    touchPinRawVal[5] = touchRead(touchPin[5]);
//    touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
//    touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
//    touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
//    touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
//    // Limit values to sane range:
//    touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
//    touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
//
//    // Calculate final pitchBend value, as "0" plus bend up minus bend down:
//    pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
//    if(pitchBend != lastPitchBend){
//      usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
//    }
//    //  
//  }
//  delay(5);
}
// End main loop//



// Keyboard instrument with 16/32 capacitive keys. Velocity output + aftertouch.
void cBoard_after(){
  int i;
  int j;
  int k;
  
////////////////////////////////////////////
// IF SMALL KEYBOARD:::

  if(keyboardSize == 16){
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

// PITCHBEND SECTION:
    if(optionPin1Val == 1 && i % pitchbendTimer == 0){
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[4] = touchPinRawVal[4];
      lastTouchPinRawVal[5] = touchPinRawVal[5];
      
      touchPinRawVal[4] = touchRead(touchPin[4]);
      touchPinRawVal[5] = touchRead(touchPin[5]);
      touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
      touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
      touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
      touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
      // Limit values to sane range:
      touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
      touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }

      // Remember the last sensor reading at this position:
      lastMux1Val[i] = mux1Val[i];
      lastMux1RawVal[i] = mux1RawVal[i];
      // Read sensor at this position:
      mux1RawVal[i] = readSingleCap(1, i);
  //    Serial.println(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.005); 
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
      
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux1Val[i] >= strokeThreshold && lastMux1Val[i] < strokeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes16[i], mux1Val[i], MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 15; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i;    
      }
      
      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux1Val[i] == 0 && lastMux1Val[i] != 0){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 15; j++){
          if(activeKeyboardSensors[j] == i){
            for(int k = j; k < 15; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([15]) to 0:
            activeKeyboardSensors[15] = 0;
          }
        }
      }
      // Print the current sensor reading values to Serial:
    }
    // Send volume control CC using the reading from the last activated sensor:
    if(mux1Val[activeKeyboardSensors[0]] != lastMux1Val[activeKeyboardSensors[0]]){
      if(keyboardAfterTouch == 1){
        usbMIDI.sendAfterTouch(mux1Val[activeKeyboardSensors[0]], MIDIchannel);
      }
      else{
        usbMIDI.sendControlChange(volumeCC, mux1Val[activeKeyboardSensors[0]], MIDIchannel);
      }
    }
  }

  
////////////////////////////////////////////
// IF LARGE KEYBOARD:::

  else if(keyboardSize == 32){
    lastKeyboardSensorVal[i] = keyboardSensorVal[i];
    // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){

// PITCHBEND SECTION:
    if(optionPin1Val == 1 && i % pitchbendTimer == 0){
      lastPitchBend = pitchBend;
      lastTouchPinRawVal[4] = touchPinRawVal[4];
      lastTouchPinRawVal[5] = touchPinRawVal[5];
      
      touchPinRawVal[4] = touchRead(touchPin[4]);
      touchPinRawVal[5] = touchRead(touchPin[5]);
      touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.01);
      touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.01);
      touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
      touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
      // Limit values to sane range:
      touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
      touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
  
      // Calculate final pitchBend value, as "0" plus bend up minus bend down:
      pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
      if(pitchBend != lastPitchBend){
        usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
      }
      //  
    }
      
      // Remember the last sensor reading at this position:
      lastMux1Val[i] = mux1Val[i];
      lastMux1RawVal[i] = mux1RawVal[i];
      // Read sensor at this position:
      mux1RawVal[i] = readSingleCap(1, i);
  //    Serial.println(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.005); 
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
      keyboardSensorVal[i] = mux1Val[i];

      // other mux:
      // Remember the last sensor reading at this position:
      lastMux2Val[i] = mux2Val[i];
      lastMux2RawVal[i] = mux2RawVal[i];
      // Read sensor at this position:
      mux2RawVal[i] = readSingleCap(2, i);
  //    Serial.println(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
      mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.005); 
      mux2Val[i] = map(mux2RawVal[i], mux2Min[i], mux2Max[i], 0, 127);
      mux2Val[i] = constrain(mux2Val[i], 0, 127);
      keyboardSensorVal[i+16] = mux2Val[i];



///////////////////////////
      //1st MUX:
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux1Val[i] >= strokeThreshold && lastMux1Val[i] < strokeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i], keyboardSensorVal[i], MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 31; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i;    
      }

      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux1Val[i] == 0 && lastMux1Val[i] != 0){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 31; j++){
          if(activeKeyboardSensors[j] == i){
            for(int k = j; k < 31; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([15]) to 0:
            activeKeyboardSensors[31] = 0;
          }
        }
      }

///////////////////////////
      //2nd MUX:
      // If sensor at this position has just been pressed, adjust the "activeSensors" array:
      if(mux2Val[i] >= strokeThreshold && lastMux2Val[i] < strokeThreshold){
        // Send note on MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], keyboardSensorVal[i+16], MIDIchannel);  
        // Move all values in the array one position to the right:
        for(j = 31; j > 0; j--){
          activeKeyboardSensors[j] = activeKeyboardSensors[j - 1]; 
        }
        // Save current sensor number at the leftmost position ([0]):
        activeKeyboardSensors[0] = i+16;    
      }
      
      // If sensor at current position has just been released, adjust the "activeSensor" array:
      else if(mux2Val[i] == 0 && lastMux2Val[i] != 0){
        // Send note off MIDI message for the note at the current position:
        usbMIDI.sendNoteOn(keyboardNotes32[i+16], 0, MIDIchannel);
        // Move all values right of the current position in the array, one position to the left:
        for(j = 0; j < 31; j++){
          if(activeKeyboardSensors[j] == i+16){
            for(int k = j; k < 31; k++){
              activeKeyboardSensors[k] = activeKeyboardSensors[k + 1]; 
            }
            // Set the value at the rightmost position ([31]) to 0:
            activeKeyboardSensors[31] = 0;
          }
        }
      }


      
      // Print the current sensor reading values to Serial:
    }
    // Send afterTouch using the reading from the last activated sensor:
    if(keyboardSensorVal[activeKeyboardSensors[0]] != lastKeyboardSensorVal[activeKeyboardSensors[0]]){
      usbMIDI.sendAfterTouch(keyboardSensorVal[activeKeyboardSensors[0]], MIDIchannel);
    }
  }

}



// Melodica instrument with 16/32 CAPACITIVE keys no velocity, volume control via breath.
void cBoard_mel(){
  int i;
  int j;
  
  // Save previous breath sensor raw value, then read and filter new value:
  lastAPinVal = aPinVal;
  aPinVal = analogRead(aPin);
  aPinVal = expFilter(aPinVal, lastAPinVal, 0.01);
  // Save previous breath sensor output value, then map new value from raw reading:
  lastBreath = breath;
  breath = map(aPinVal, aPinMin, aPinMax, 0, 127);
//  Serial.println(aPinVal);

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
//      for(i = 0; i < 16; i++){
//        usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
//      }
      usbMIDI.sendControlChange(123, 0, MIDIchannel);
    }
    // Else if breath sensor recently activated, send note on message:
    else if(breath != 0 && lastBreath == 0){
      for(i = 0; i < 32; i++){
        if(activeKeyboardSensors[i] == 1){
          usbMIDI.sendNoteOn(keyboardNotes32[i], 127, MIDIchannel);
        }
      }
    }
  }

  if(breath != 0){ 
  // Main for loop. "i" is the sensor number:
    for(i = 0; i < 16; i++){
      if(i % breathTimer == 0){
        // Save previous breath sensor raw value, then read and filter new value:
        lastAPinVal= aPinVal;
        aPinVal = analogRead(aPin);
        aPinVal = expFilter(aPinVal, lastAPinVal, 0.01);
        // Save previous breath sensor output value, then map new value from raw reading:
        lastBreath = breath;
        breath = map(aPinVal, aPinMin, aPinMax, 0, 127);
      //  Serial.println(aPinVal);
      
        // Limit output to MIDI range:
        breath = constrain(breath, 0, 127);
      
      //  Serial.println(breath);
        // If breath sensor output value has changed: 
        if(breath != lastBreath){
      //    Serial.println(breathOut);
          // Send CC2 volume control:
          usbMIDI.sendControlChange(volumeCC, breath, MIDIchannel);
        }
        if(breath == 0){
          usbMIDI.sendControlChange(123, 0, MIDIchannel);
        }
      }
       // Read the extra touchRead pins, and map them to pitchBend values, if that option is activated
      if(optionPin1Val == 1 &&  i % pitchbendTimer == 0){
        lastPitchBend = pitchBend;
        lastTouchPinRawVal[4] = touchPinRawVal[4];
        lastTouchPinRawVal[5] = touchPinRawVal[5];
        
        touchPinRawVal[4] = touchRead(touchPin[4]);
        touchPinRawVal[5] = touchRead(touchPin[5]);
        touchPinRawVal[4] = expFilter(touchPinRawVal[4], lastTouchPinRawVal[4], 0.005);
        touchPinRawVal[5] = expFilter(touchPinRawVal[5], lastTouchPinRawVal[5], 0.005);
        touchPinVal[4] = map(touchPinRawVal[4], touchPinMin[4], touchPinMax[4], 0, 64);
        touchPinVal[5] = map(touchPinRawVal[5], touchPinMin[5], touchPinMax[5], 0, 63);
        // Limit values to sane range:
        touchPinVal[4] = constrain(touchPinVal[4], 0, 64);
        touchPinVal[5] = constrain(touchPinVal[5], 0, 63);
    
        // Calculate final pitchBend value, as "0" plus bend up minus bend down:
        pitchBend = 0 - touchPinVal[4] + touchPinVal[5];
        if(pitchBend != lastPitchBend){
          usbMIDI.sendPitchBend(pitchBend<<7, MIDIchannel);
        }
        //  
      }
      // Remember the last sensor reading at this position:    
      lastMux1Val[i] = mux1Val[i];
      lastMux1RawVal[i] = mux1RawVal[i];
      mux1RawVal[i] = readSingleCap(1, i);
      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.01); 
      mux1Val[i] = map(mux1RawVal[i], mux1Min[i], mux1Max[i], 0, 127);
      mux1Val[i] = constrain(mux1Val[i], 0, 127);
  //    Serial.print(mux1Val[i]);
  //    Serial.print(" - ");
      if(keyboardSize == 16){
        if(mux1Val[i] >= padThreshold && lastMux1Val[i] < padThreshold){
          activeKeyboardSensors[i] = 1;
          usbMIDI.sendNoteOn(keyboardNotes16[i], 127, MIDIchannel);
        }
        else if(mux1Val[i] == 0 && lastMux1Val[i] != 0){
          activeKeyboardSensors[i] = 0;
          usbMIDI.sendNoteOn(keyboardNotes16[i], 0, MIDIchannel);
        }
//        Serial.print(activeKeyboardSensors[i]);
//        Serial.print(" - ");
      }    
      else if(keyboardSize == 32){
        lastMux2Val[i] = mux2Val[i];
        lastMux2RawVal[i] = mux2RawVal[i];
        mux2RawVal[i] = readSingleCap(2, i);
        mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.01); 
        mux2Val[i] = map(mux2RawVal[i], mux2Min[i], mux2Max[i], 0, 127);
        mux2Val[i] = constrain(mux2Val[i], 0, 127);
  //      Serial.print(mux2Val[i]);
  //      Serial.print(" - ");
        
        if(mux1Val[i] >= strokeThreshold && lastMux1Val[i] < strokeThreshold){
          activeKeyboardSensors[i] = 1;
          usbMIDI.sendNoteOn(keyboardNotes32[i], 127, MIDIchannel);
        }
        else if(mux1Val[i] == 0 && lastMux1Val[i] != 0){
          activeKeyboardSensors[i] = 0;
          usbMIDI.sendNoteOn(keyboardNotes32[i], 0, MIDIchannel);
        }
        if(mux2Val[i] >= strokeThreshold && lastMux2Val[i] < strokeThreshold){
          activeKeyboardSensors[i+16] = 1;
          usbMIDI.sendNoteOn(keyboardNotes32[i+16], 127, MIDIchannel);
        }
        else if(mux2Val[i] == 0 && lastMux2Val[i] != 0){
          activeKeyboardSensors[i+16] = 0;
          usbMIDI.sendNoteOn(keyboardNotes32[i+16], 0, MIDIchannel);
        }    
      }
      // Read sensor at this position:
  
      
  //    Serial.print(mux1RawVal[i]);
  //    Serial.print(" - ");
      // Filter reading for stability and map to MIDI range:
  
    }
  }
//  Serial.println();
  
}




void cBoard_sixSens(){ // 6 sensor capacitive instrument, with pentatonic velocity output (FROOT LOOPER)
  int i;
  for(i = 0; i < 6; i++){
    lastTouchPinRawVal[sixSensSensors[i]] = touchPinRawVal[sixSensSensors[i]];
    lastTouchPinVal[sixSensSensors[i]] = touchPinVal[sixSensSensors[i]];
    touchPinRawVal[sixSensSensors[i]] = touchRead(touchPin[sixSensSensors[i]]);
    touchPinRawVal[sixSensSensors[i]] = expFilter(touchPinRawVal[sixSensSensors[i]], lastTouchPinRawVal[sixSensSensors[i]], 0.005);
    touchPinVal[sixSensSensors[i]] = map(touchPinRawVal[sixSensSensors[i]], touchPinMin[sixSensSensors[i]], touchPinMax[sixSensSensors[i]], 0, 127);
    touchPinVal[sixSensSensors[i]] = constrain(touchPinVal[sixSensSensors[i]], 0, 127);

    if(touchPinVal[sixSensSensors[i]] >= strokeThreshold && lastTouchPinVal[sixSensSensors[i]] < strokeThreshold && noteOnArray[i] == 0){
      int velocity = touchPinVal[sixSensSensors[i]];
      usbMIDI.sendNoteOn(sixSensNotes[i], velocity, MIDIchannel);
      noteOnArray[i] = 1;
    }
    else if(touchPinVal[sixSensSensors[i]] < offThreshold && lastTouchPinVal[sixSensSensors[i]] >= offThreshold){
      usbMIDI.sendNoteOn(sixSensNotes[i], 0, MIDIchannel);
      noteOnArray[i] = 0;
    }
      
//    Serial.print(touchPinVal[sixSensSensors[i]]);
//    Serial.print(" - ");
  }
//  Serial.println();
}


