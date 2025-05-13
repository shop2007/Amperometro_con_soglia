void sendSMS(String numero, String messaggio) {
  // Aggiungi numero SMS e (opzionale) timestamp al testo
  messaggio = "#" + String(ContaSMS) + " " + messaggio;
  
  Serial.print(F("Invio SMS a: "));
  Serial.println(numero);

  // Costruisci il comando AT+CMGS con c_str()
  sim800.print(F("AT+CMGS=\""));
  sim800.print(numero.c_str());
  sim800.println(F("\""));

  delay(1000);  // Attendi il prompt '>'

  sim800.print(messaggio.c_str());
  sim800.write(26);  // Ctrl+Z per invio
  Serial.println(F("Comando di invio SMS inviato, attendo risposta..."));

  // Attesa risposta del modulo
  unsigned long timeout = millis() + 10000; // max 10 secondi
  String risposta = "";

  while (millis() < timeout) {
    if (sim800.available()) {
      char c = sim800.read();
      risposta += c;
    }
    if (risposta.indexOf("OK") >= 0 || risposta.indexOf("+CMGS") >= 0) {
      Serial.println(F("SMS inviato correttamente."));
      break;
    }
    if (risposta.indexOf("ERROR") >= 0) {
      Serial.println(F("Errore durante l'invio dell'SMS!"));
      Serial.println("Risposta modulo: " + risposta);
      return;
    }
  }

  if (risposta.length() == 0) {
    Serial.println(F("Timeout: nessuna risposta dal modulo."));
  }

  // Incrementa contatore e salva
  ContaSMS++;
  writeContaSmsEEPROM(ContaSMS);
  delay(300);
  Serial.print(F("ContaSMS incrementato a: "));
  Serial.println(ContaSMS);
}
