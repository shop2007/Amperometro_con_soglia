#include <SoftwareSerial.h>
#include <EEPROM.h>
#include <stdio.h> // Includi la libreria per sprintf

/*
Per cambiare i numeri da sms mandare il seguente sms
pin + n.utente (1 o 2) + telefono senza prefisso
esempio con pin 99887
99887 1 3331234567     senza spazi mette utente 1 = 3331234567
99887 2 3339876543     senza spazi mette utente 1 = 3339876543

C'è una nuova variabile in EEPRON ContaSmsEEprom
Ad ogni reset il suo valore viene scritto in ContaSMS
Ogni volta che si chiama sendSMS() si incrementa il numero di ContaSMS
Il valore di ContaSMS viene scritto in EEprom
Tutto con Printf di debug
Se ricevo sms con Pin2 azzero contasms ram ed eeprom

inoltre se la sim riceve un sms contenente il Pin2 azzera il contasms sia in ram che in eeprom
*/


// SIM800L su pin 7 RX, 8 TX (adatta se usi diversi pin)
SoftwareSerial sim800(9, 11); //rx, tx

const String Pin = "19927";
const String PrefissoInt = "+39";
const String Pin2 = "19999"; // "Azzera conta sms"

String NumeroUtente1;
String NumeroUtente2;

int ContaSMS = 0; // Inizializzato a 0

// Indirizzo EEPROM per il contatore SMS
const int addrContaSms = 40; // Scegli un indirizzo che non si sovrapponga agli altri

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

// Funzioni per la gestione del contatore SMS in EEPROM
void writeContaSmsEEPROM(int value) {
  EEPROM.put(addrContaSms, value);
  Serial.print("Scritto in EEPROM ContaSmsEEprom indirizzo ");Serial.print(addrContaSms);Serial.print(" Valore ");Serial.println(value);

}

int readContaSmsEEPROM() {
  int value;
  EEPROM.get(addrContaSms, value);
  Serial.print("Letto da EEPROM ContaSmsEEprom indirizzo ");Serial.print(addrContaSms);Serial.print(" Valore ");Serial.println(value);
  return value;
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

  // Legge il contatore SMS da EEPROM e lo scrive in ContaSMS
  ContaSMS = readContaSmsEEPROM();
  Serial.print("ContaSMS dopo reset:"); Serial.println(ContaSMS);

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
    Serial.println("Evento SIM: " + sms);
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
  // Controllo per il PIN di azzeramento
  if (sms.indexOf(Pin2) != -1) {
    // Azzera il contatore in RAM
    ContaSMS = 0;
    Serial.print("Azzerato ContaSmsEEprom indirizzo ");Serial.print(addrContaSms);Serial.print(" Valore ");Serial.println(ContaSMS);

    // Azzera il contatore in EEPROM
    writeContaSmsEEPROM(ContaSMS); // Scrive 0 nell'EEPROM
    Serial.println("DEBUG: ContaSmsEEprom azzerato tramite SMS.");

    // Puoi anche inviare un SMS di conferma (opzionale)
    sendSMS(PrefissoInt + NumeroUtente1, (String(ContaSMS) + " Contatore SMS azzerato.")); // Invia a utente 1 come esempio
    return; // Esci dalla funzione processSMS per evitare ulteriori elaborazioni
  }

  // Cerca il PIN per la modifica dei numeri
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
  messaggio = ("#" + String(ContaSMS) + " " + messaggio);
  Serial.println("Invio SMS a: " + numero);
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z per inviare
  Serial.println("SMS inviato.");

  // Incrementa il contatore SMS e lo scrive in EEPROM
  ContaSMS++;
  writeContaSmsEEPROM(ContaSMS);
  delay(300);
  Serial.print("ContaSMS incrementato a:"); Serial.println(ContaSMS);
  delay(5000);
}
