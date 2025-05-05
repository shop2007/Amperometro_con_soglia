#include <SoftwareSerial.h>

// RX, TX (dal punto di vista di Arduino)
SoftwareSerial sim800(10, 11); // RX, TX

void setup() {
  Serial.begin(115200);      // Porta USB con PC
  sim800.begin(57600);      // Porta con SIM800

  Serial.print("Nome file: ");
  Serial.println(__FILE__);

  Serial.println("Invia AT commands dal Serial Monitor");
}

void loop() {
  // Da PC a SIM800
  if (Serial.available()) {
    char c = Serial.read();
    sim800.write(c);
  }

  // Da SIM800 a PC
  if (sim800.available()) {
    char c = sim800.read();
    Serial.write(c);
  }
}
