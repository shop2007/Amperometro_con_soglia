void sendSMS(String numero, String messaggio) {
  messaggio = "#" + String(ContaSMS) + " " + messaggio;

  Serial.print(F("Invio SMS a: "));
  Serial.println(numero);

  // Invia il comando AT con numero, ma usando .c_str()
  sim800.print(F("AT+CMGS=\""));
  sim800.print(numero.c_str());  // meglio usare c_str()
  sim800.println(F("\""));
  delay(1000);  // attesa per il prompt '>'

  // Invia il testo del messaggio
  sim800.print(messaggio.c_str());  // usa c_str() per invio affidabile

  // Termina con Ctrl+Z (ASCII 26)
  sim800.write(26);
  Serial.println(F("SMS inviato."));

  // Aggiorna contatore
  ContaSMS++;
  writeContaSmsEEPROM(ContaSMS);
  delay(300);
  Serial.print(F("ContaSMS incrementato a: "));
  Serial.println(ContaSMS);

  delay(5000);  // attesa per stabilità del modulo
}