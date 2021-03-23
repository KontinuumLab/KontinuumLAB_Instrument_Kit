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




//----------------- LOAD ALL VARIABLES FROM MEMORY ------------------ //

int address;
uint16_t tempValue;

void loadSettings(){
  int i;
  address = 0;
  
//  Serial.println("Analog sensors:");
  // Analog pin minimum readings:

    EEPROM.get(address, aPinMin);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinMin[
    address += 2;

//  Serial.println();
  // Analog pin center readings:
    EEPROM.get(address, aPinCent);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinCent);
    address += 2;

//  Serial.println();
  // Analog pin maximum readings:
    EEPROM.get(address, aPinMax);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(aPinMax);

    address += 2;
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
    EEPROM.put(address, aPinMin);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;

//  Serial.println();
  // Analog pin center readings:
    EEPROM.put(address, aPinCent);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;

//  Serial.println();
  // Analog pin maximum readings:
    EEPROM.put(address, aPinMax);
    EEPROM.get(address, tempValue);
//    Serial.print(address);
//    Serial.print(": ");
//    Serial.println(tempValue);
    address += 2;

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

