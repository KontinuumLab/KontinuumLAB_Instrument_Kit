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



//----------------- LOAD ALL VARIABLES FROM MEMORY ------------------ //

int address;
uint16_t tempValue;

void loadSettings(){
  int i;
  address = 0;
  
//  Serial.println("Analog sensors:");
  // Analog pin minimum readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.get(address, aPinMin[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinMin[i]);
    address += 2;
  }
//  Serial.println();
  // Analog pin center readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.get(address, aPinCent[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinCent[i]);
    address += 2;
  }
//  Serial.println();
  // Analog pin maximum readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.get(address, aPinMax[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinMax[i]);
    address += 2;
  }
//  Serial.println();
//  Serial.println();
}



//----------------- SAVE ALL VARIABLES TO MEMORY ------------------ //

void saveSettings(){
  int i;
  address = 0;
  
//  Serial.println("Analog sensors:");
  // Analog pin minimum readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.put(address, aPinMin[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
  // Analog pin center readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.put(address, aPinCent[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
  // Analog pin maximum readings:
  for (i = 0; i < aPinArrayLength; i++) {
    EEPROM.put(address, aPinMax[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
//  Serial.println();

  // Blink LED in response:
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
  delay(200);
  digitalWrite(ledPin, HIGH);
  delay(200);
  digitalWrite(ledPin, LOW);
}



