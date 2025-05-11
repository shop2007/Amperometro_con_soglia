#include <SoftwareSerial.h>
#include <EEPROM.h>
/*
Per cambiare i numeri da sms mandare il seguente sms
pin + n.utente (1 o 2) + telefono senza prefisso
esempio con pin 99887
99887 1 3331234567    senza spazi mette utente 1 = 3331234567
99887 2 3339876543    senza spazi mette utente 1 = 3339876543

C'è una nuova variabile in EEPRON ContaSmsEEprom 
Ad ogni reset il suo valore viene scritto in ContaSMS 
Ogni volta che si chiama sendSMS() si incrementa il numero di ContaSMS 
Il valore di ContaSMS viene scritto in EEprom 
Tutto con Printf di debug
*/ 


// SIM800L su pin 7 RX, 8 TX (adatta se usi diversi pin)
SoftwareSerial sim800(9, 11); //rx, tx

const String Pin = "19927";
const String PrefissoInt = "+39";
const String Pin2 = "19999"; // "Azzera conta sms"

String NumeroUtente1;
String NumeroUtente2;

int ContaSMS = 1;

String TestoSMS = "questo è il testo sms";

// EEPROM offset (ogni numero max 15 caratteri)
const int addr1 = 0;
const int addr2 = 20;

// Funzioni di EEPROM
void writeStringToEEPROM(int addrOffset, const String &str) {
  for (int i = 0; i < str.length(); i++) {
    EEPROM.write(addrOffset + i, str[i]);
  }
  EEPROM.write(addrOffset + str.length(), '\0');
}

String readStringFromEEPROM(int addrOffset) {
  char data[16];
  int len = 0;
  unsigned char k;
  k = EEPROM.read(addrOffset);
  while (k != '\0' && len < 15) {
    data[len++] = k;
    k = EEPROM.read(addrOffset + len);
  }
  data[len] = '\0';
  return String(data);
}

void setup() {
  Serial.begin(115200);
  sim800.begin(57600);
  delay(500);
  Serial.print("Nome file: ");
  Serial.println(__FILE__);
  delay(1500);  
  // Legge i numeri da EEPROM
  NumeroUtente1 = readStringFromEEPROM(addr1);
  NumeroUtente2 = readStringFromEEPROM(addr2);

  Serial.println("NumeroUtente1 da EEPROM: " + NumeroUtente1);
  Serial.println("NumeroUtente2 da EEPROM: " + NumeroUtente2);

  sim800.println("AT");
  delay(1000);
  sim800.println("AT+CMGF=1");  // Modalità testo
  delay(1000);
  sim800.println("AT+CNMI=1,2,0,0,0");  // Ricezione immediata SMS
  delay(1000);
  Serial.println("Menu");
  Serial.println("1 manda sms prova utente 1");
  Serial.println("2 manda sms prova utente 2");
  Serial.println("3 forza utente 1 333210xxxx");
  Serial.println("4 forza utente 2 329703yyyy");
  Serial.println("5 Legge i numeri 1 e 2");
  
}

void loop() {
  // Controllo SMS in arrivo
  if (sim800.available()) {
    String sms = sim800.readString();
    Serial.println("SMS ricevuto: " + sms);
    processSMS(sms);
  }

  // Comandi seriali per invio SMS
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') {
      sendSMS(PrefissoInt + NumeroUtente1, "Testo sms utente 1");
    } else if (cmd == '2') {
      sendSMS(PrefissoInt + NumeroUtente2, "Testo SMS utente 2");
    } else if (cmd == '3') {
      processSMS("199271333210xxxx");//inserisce come numero 1 333210xxxx
    } else if (cmd == '4') {
      processSMS("199272329703yyyy");//inserisce come numero 1 329703yyyy
    } else if (cmd == '5') {
      Serial.println("NumeroUtente1 da EEPROM: " + NumeroUtente1);
      Serial.println("NumeroUtente2 da EEPROM: " + NumeroUtente2);
    }




  }
}

void processSMS(String sms) {
  // Cerca il PIN all'inizio
  int pinIndex = sms.indexOf(Pin);
  if (pinIndex != -1) {
    int idx = pinIndex + Pin.length();
    char userIndex = sms[idx];
    String newNumber = sms.substring(idx + 1, idx + 11);  // 10 cifre

    Serial.println("Tentativo cambio numero per utente: " + String(userIndex));
    Serial.println("Nuovo numero proposto: " + newNumber);

    if (userIndex == '1' && newNumber.length() == 10) {
      NumeroUtente1 = newNumber;
      writeStringToEEPROM(addr1, newNumber);
      Serial.println("NumeroUtente1 aggiornato: " + NumeroUtente1);
      sendSMS(PrefissoInt + NumeroUtente1, "Utente 1 connesso allarm3 corrente");
    } else if (userIndex == '2' && newNumber.length() == 10) {
      NumeroUtente2 = newNumber;
      writeStringToEEPROM(addr2, newNumber);
      Serial.println("NumeroUtente2 aggiornato: " + NumeroUtente2);
      sendSMS(PrefissoInt + NumeroUtente2, "Utente 2 connesso allarme corrente");
    } else {
      Serial.println("Formato SMS non valido");
    }
  }
}

void sendSMS(String numero, String messaggio) {
  Serial.println("Invio SMS a: " + numero);
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z per inviare
  Serial.println("SMS inviato.");
  delay(5000);
}
