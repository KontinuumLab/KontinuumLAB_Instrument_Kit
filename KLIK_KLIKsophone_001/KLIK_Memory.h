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
//  
//  Serial.println("Touch sensors:");
  // Touch pin minimum readings:
  for (i = 0; i < touchPinArrayLength; i++) {
    EEPROM.get(address, touchPinMin[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(touchPinMin[i]);
    address += 2;
  }
  // (No touch pin center readings)
  
//  Serial.println();
  // Touch pin maximum readings:
  for (i = 0; i < touchPinArrayLength; i++) {
    EEPROM.get(address, touchPinMax[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(touchPinMax[i]);
    address += 2;
  }
//  Serial.println();
//  Serial.println();
//  
//  Serial.println("Multiplexers:");
  
  // Multiplexer 1 minimum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.get(address, mux1Min[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(mux1Min[i]);
    address += 2;
  }
//  Serial.println();
  // Multiplexer 1 maximum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.get(address, mux1Max[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(mux1Max[i]);
    address += 2;
  }
//  Serial.println();
  // Multiplexer 2 minimum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.get(address, mux2Min[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(mux2Min[i]);
    address += 2;
  }
//  Serial.println();
  // Multiplexer 2 maximum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.get(address, mux2Max[i]);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(mux2Max[i]);
    address += 2;
  }
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

//  Serial.println("Touch sensors:");
  // Touch pin minimum readings:
  for (i = 0; i < touchPinArrayLength; i++) {
    EEPROM.put(address, touchPinMin[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
  // (No touch pin center readings)
//  Serial.println();
  // Touch pin maximum readings:
  for (i = 0; i < touchPinArrayLength; i++) {
    EEPROM.put(address, touchPinMax[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
//  Serial.println();

//  Serial.println("Multiplexers:");
  // Multiplexer 1 minimum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.put(address, mux1Min[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
  // Multiplexer 1 maximum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.put(address, mux1Max[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
  Serial.println();
  // Multiplexer 2 minimum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.put(address, mux2Min[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }
//  Serial.println();
  // Multiplexer 2 maximum readings:
  for (i = 0; i < 16; i++) {
    EEPROM.put(address, mux2Max[i]);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;
  }

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

void loadAllDefaults() {

}

void setDefaults() {

}

void loadSensorsPreset() {

}

void loadKeysPreset() {

}

void loadFingeringsPreset() {

}

void loadOctavesPreset() {

}

void loadOutputPreset() {

}



////################### set up EEPROM: ########################



//       For reading:
//  uint8_t read(int address);
//  uint8_t readByte(int address);
//  uint16_t readInt(int address);
//  uint32_t readLong(int address);
//  float readFloat(int address);
//  double readDouble(int address);
//  Where address is the starting position in EEPROM, and the return value the value read from EEPROM.

//        For writing:
//  bool write(int address, uint8_t value);
//  bool writeByte(int address, uint8_t value);
//  bool writeInt(int address, uint16_t value);
//  bool writeLong(int address, uint32_t value);
//  bool writeFloat(int address, float value);
//  bool writeDouble(int address, double value);

