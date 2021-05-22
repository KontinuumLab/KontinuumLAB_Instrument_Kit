

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




