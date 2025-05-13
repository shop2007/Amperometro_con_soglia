
void sendSMS(String numero, String messaggio) {
  messaggio = ("#" + String(ContaSMS) + " " + messaggio);
  Serial.print(F("Invio SMS a: "));
  Serial.print(numero);
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z per inviare
  Serial.println(F("SMS inviato."));

  // Incrementa il contatore SMS e lo scrive in EEPROM
  ContaSMS++;
  writeContaSmsEEPROM(ContaSMS);
  delay(300);
  Serial.print(F("ContaSMS incrementato a:"));
  Serial.println(ContaSMS);
  delay(5000);
}