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
  for(i = 1; i < aPinArrayLength; i++){
    aPinCent[i] = analogRead(aPin[i]);
  }
  
  // Then reset all other values before calibration:
  for(i = 0; i < aPinArrayLength; i++){
    aPinMin[i] = 0;
    aPinMax[i] = 1023; // CNY70 readings for the KLIK are normally reversed.
  }
  
    // Start calibration loop:
  while(calBtnPressed == 1){
    
    // ANALOG:
    for(i = 0; i < aPinArrayLength; i++){
      lastAPinRawVal[i] = aPinRawVal[i];
      aPinRawVal[i] = analogRead(aPin[i]);
//      aPinRawVal[i] = expFilter(aPinRawVal[i], lastAPinRawVal[i], 0.001);
      if(aPinRawVal[i] > aPinMin[i]){
        aPinMin[i] = aPinRawVal[i];
      }
      else if(aPinRawVal[i] < aPinMax[i]){
        aPinMax[i] = aPinRawVal[i];
      }
    }
    calBtnPressed = debounce(calPin); 
  }

  // Adjust minimum and maximum values with a buffer value:
  for(i = 0; i < aPinArrayLength; i++){
    tempVal = (aPinMin[i] - aPinMax[i]) / 5;
    aPinMax[i] = aPinMax[i] + tempVal;
//    aPinMax[i] = constrain(aPinMax[i], 0, 1023);
    aPinMin[i] = aPinMin[i] - tempVal;
//    aPinMin[i] = constrain(aPinMin[i], 0, 1023);
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



