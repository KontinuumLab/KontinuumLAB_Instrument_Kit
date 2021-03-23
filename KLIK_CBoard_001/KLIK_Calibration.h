/*
    KontinuumLAB Instrument Kit - "C-Board" MIDI capacitive keyboard individual instrument sketch
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



int calBtnPressed = 0;
int calibrating = 0;

int calibrationPhase;

unsigned int calibrationStart;
unsigned int calibrationTimer;


void calibration(){
  int i;
  uint16_t tempVal;
  digitalWrite(ledPin, HIGH);
  delay(500);
  digitalWrite(ledPin, LOW);
  delay(5);
  // First set all analog "center points" to current reading: 
  aPinCent = analogRead(aPin);
  
  // Then reset all other values before calibration:
  aPinMin = 0;
  aPinMax = 1023; // CNY70 readings for the KLIK are normally reversed.
  
  // Touch sensors
  for(i = 0; i < touchPinArrayLength; i++){
    touchPinMin[i] = 10000;
    touchPinMax[i] = 0;
  }

  // Multiplexers:
  for(i = 0; i < 16; i++){
    mux1Min[i] = 10000;
    mux1Max[i] = 0;
  }
  for(i = 0; i < 16; i++){
    mux2Min[i] = 10000;
    mux2Max[i] = 0;
  }
  
    // Start calibration loop:
  while(calBtnPressed == 1){
    
    // ANALOG:
      lastAPinRawVal = aPinRawVal;
      aPinRawVal = analogRead(aPin);
      if(aPinRawVal > aPinMin){
        aPinMin = aPinRawVal;
      }
      else if(aPinRawVal < aPinMax){
        aPinMax = aPinRawVal;
      }
      
    delay(10);
    // TOUCH:
    for(i = 0; i < touchPinArrayLength; i++){
      lastTouchPinRawVal[i] = touchPinRawVal[i];
      touchPinRawVal[i] = touchRead(touchPin[i]);
//      touchPinRawVal[i] = expFilter(touchPinRawVal[i], lastTouchPinRawVal[i], 0.001);
      if(touchPinRawVal[i] < touchPinMin[i]){
        touchPinMin[i] = touchPinRawVal[i];
      }
      else if(touchPinRawVal[i] > touchPinMax[i]){
        touchPinMax[i] = touchPinRawVal[i];
      }
    }
    delay(10);
    // MULTIPLEXER 1:
    for(i = 0; i < 16; i++){
//      lastMux1RawVal[i] = mux1RawVal[i];
      mux1RawVal[i] = readSingleCap(1, i);
//      mux1RawVal[i] = expFilter(mux1RawVal[i], lastMux1RawVal[i], 0.001);
      if(mux1RawVal[i] < mux1Min[i]){
        mux1Min[i] = mux1RawVal[i];
      }
      else if(mux1RawVal[i] > mux1Max[i]){
        mux1Max[i] = mux1RawVal[i];
      }
    }

    // MULTIPLEXER 2:
    for(i = 0; i < 16; i++){
//      lastMux2RawVal[i] = mux2RawVal[i];
      mux2RawVal[i] = readSingleCap(2, i);
//      mux2RawVal[i] = expFilter(mux2RawVal[i], lastMux2RawVal[i], 0.001);
      if(mux2RawVal[i] < mux2Min[i]){
        mux2Min[i] = mux2RawVal[i];
      }
      else if(mux2RawVal[i] > mux2Max[i]){
        mux2Max[i] = mux2RawVal[i];
      }
    }
    
    calBtnPressed = debounce(calPin); 
  }

  // Adjust minimum and maximum values with a buffer value:

    tempVal = (aPinMin - aPinMax) / 5;
    aPinMax = aPinMax + tempVal;
    aPinMin = aPinMin - tempVal;

  for(i = 0; i < touchPinArrayLength; i++){
    tempVal = (touchPinMax[i] - touchPinMin[i]) / 7;
    touchPinMax[i] = touchPinMax[i] - tempVal;
    touchPinMin[i] = touchPinMin[i] + tempVal;
  }

  for(i = 0; i < 16; i++){
    tempVal = (mux1Max[i] - mux1Min[i]) / 7;
    mux1Max[i] = mux1Max[i] - tempVal;
    mux1Min[i] = mux1Min[i] + tempVal;
    tempVal = (mux2Max[i] - mux2Min[i]) / 7;
    mux2Max[i] = mux2Max[i] - tempVal;
    mux2Min[i] = mux2Min[i] + tempVal;
  }
  // Finish calibration
//  digitalWrite(ledPin, LOW);
}

void calibrationCheck(){
  calibrationPhase = 0;
  int i;
  // Read calibrate button. Inverse result for "1" to be "yes":
  calBtnPressed = debounce(calPin);
  
  if(calBtnPressed == 1){ // Check for long press or 3 repeated short presses:
    for(i = 0; i < 3; i++){
      
      // "Btn is pressed" part of the cycle:
      calibrationStart = millis();
      calibrationTimer = millis();
      while(calibrationTimer - 500 < calibrationStart){
        calBtnPressed = debounce(calPin);
        if(calBtnPressed == 0){
          calibrationPhase++;
          break; // Break out of "while" loop, keep checking for presses
        }
        calibrationTimer = millis();
      }
      if(calibrationTimer - 500 >= calibrationStart){
        calibration();
        return;
      }
      // "Btn is NOT pressed" part of the cycle:

      // Go to "saveSettings" function after releasing the 3rd press:
      if(calibrationPhase > 2){
        saveSettings();
        return;
      }
      calibrationStart = millis();
      calibrationTimer = millis();
      while(calibrationTimer - 500 < calibrationStart){
        calBtnPressed = debounce(calPin);
        if(calBtnPressed == 1){
          break; // Break out of "while" loop, keep checking for presses
        }
        calibrationTimer = millis();
      }
      if(calibrationTimer - 500 >= calibrationStart){
        return;
      }
    }
  }
}



