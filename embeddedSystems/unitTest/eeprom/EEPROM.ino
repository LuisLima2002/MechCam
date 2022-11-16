#include <EEPROM.h>

#define EEPROM_SIZE 12

void setup() {
  Serial.begin(115200);
  Serial.println(F("Initialize System"));
  EEPROM.begin(EEPROM_SIZE);


  //Read data from eeprom
  int address = 0;
  String readId;
  readId = EEPROM.readString(address); //EEPROM.get(address,readId);
  Serial.print("Read Id = ");
  Serial.println(readId);

  //Write data into eeprom
  EEPROM.writeString(address,"teste");
  EEPROM.commit();
}

void loop(){

}