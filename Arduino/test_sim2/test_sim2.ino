#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial sim800(9, 11); // RX, TX

// PIN di accesso via SMS
const String PinUtente1 = "11111";
const String PinUtente2 = "22222";
const String PinDati    = "12345";
const String PinReset   = "88888";

// EEPROM indirizzi
#define EEPROM_ADDR_NUM1 0
#define EEPROM_ADDR_NUM2 20

// Numeri registrati
String numero1 = "";
String numero2 = "";

// Variabili per il messaggio
char msgdati[100];
int ContaSMS = 3;
char StrSogliaMilliampere[5] = "1500";
char strContaSecondiCorrenteElevata[5] = "0123";
char strCurrent[6] = "0250";
char strVoltage[5] = "12.5";
unsigned int ContaSecondi = 0;

// Timer per ContaSecondi
unsigned long ultimoMillis = 0;
const unsigned long intervalloSecondi = 1000;

// Pulsante su pin 2
const int pulsantePin = 2;
bool statoPrecedente = HIGH;

void setup() {
  Serial.begin(115200);
  sim800.begin(9600);
  delay(1000);
  Serial.println("RESET");

  pinMode(pulsantePin, INPUT_PULLUP);

  // Recupera numeri da EEPROM
  numero1 = readStringFromEEPROM(EEPROM_ADDR_NUM1);
  numero2 = readStringFromEEPROM(EEPROM_ADDR_NUM2);

  sim800.println("AT+CMGF=1");        // Modalità testo
  delay(1000);
  sim800.println("AT+CNMI=1,2,0,0,0"); // Ricezione immediata SMS
  delay(1000);
}

void loop() {
  // Incrementa ContaSecondi ogni secondo
  if (millis() - ultimoMillis >= intervalloSecondi) {
    ultimoMillis = millis();
    ContaSecondi++;
    Serial.print(ContaSecondi);Serial.print( " ");
    if (ContaSecondi % 50 == 0) {
      //va acapo per non sforare con il terminale
      Serial.println();
    }
  }

  // Controllo SMS
  if (sim800.available()) {
    String raw = sim800.readString();
    raw.trim();
    Serial.print("raw: >"); Serial.print(raw);Serial.println("<");

    if (raw.indexOf("+CMT:") != -1) {
      int msgStart = raw.indexOf("\n", raw.indexOf("\n") + 1);
      Serial.print("msgStart: ");Serial.println(msgStart);
      String contenuto = "";
      if (msgStart != -1) {
          contenuto = raw.substring(msgStart);
          contenuto.trim(); // ✅ CORRETTO
          Serial.print("contenuto: "); Serial.println(contenuto);
      }

      int index = raw.indexOf("+39");
      if (index != -1) {
        String numero = raw.substring(index, index + 13);
        Serial.print("numero: "); Serial.println(numero);
        Serial.print("raw2: >"); Serial.print(raw);Serial.println("<");


        if (raw.indexOf(PinUtente1) != -1) {
          Serial.println("Match PinUtente1");
          numero1 = numero;
          writeStringToEEPROM(EEPROM_ADDR_NUM1, numero1);
          sendSMS(numero1, "Utente1 registrato correttamente.");
        }
        else if (raw.indexOf(PinUtente2) != -1) {
          Serial.println("Match PinUtente2");
          numero2 = numero;
          writeStringToEEPROM(EEPROM_ADDR_NUM2, numero2);
          sendSMS(numero2, "Utente2 registrato correttamente.");
        }
        else if (raw.indexOf(PinDati) != -1) {
          Serial.println("Match PinDati");
          CostruisciMsgDati();
          sendSMS(numero, String(msgdati));
        }
        else if (raw.indexOf(PinReset) != -1) {
            Serial.println("Match PinReset");
            sendSMS(numero1, "Reset in corso...");
            delay(1000);
            sim800.println("AT+CFUN=1,1");
            delay(1000);
            resetCPU();
          
        }
      }
    }
  }

  /*
  // Gestione pulsante per invio dati
  bool statoCorrente = digitalRead(pulsantePin);
  if (statoPrecedente == HIGH && statoCorrente == LOW) {
    CostruisciMsgDati();
    if (numero1.length() == 13) sendSMS(numero1, String(msgdati));
    if (numero2.length() == 13) sendSMS(numero2, String(msgdati));
    delay(200); // debounce
  }
  statoPrecedente = statoCorrente;
  */
}

// Crea msgdati con tutte le variabili
void CostruisciMsgDati() {
  snprintf(msgdati, sizeof(msgdati),
           "SMS:%d SOGL:%smA CUR:%smA SEC:%s V:%s TSEC:%u",
           ContaSMS,
           StrSogliaMilliampere,
           strCurrent,
           strContaSecondiCorrenteElevata,
           strVoltage,
           ContaSecondi);
}

// Invia SMS
void sendSMS(String numero, String messaggio) {
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z
  delay(3000);

  // Verifica invio
  unsigned long tempoInizio = millis();
  String risposta = "";
  while (millis() - tempoInizio < 5000) {
    if (sim800.available()) {
      char c = sim800.read();
      risposta += c;
      if (risposta.indexOf("+CMGS:") != -1) {
        Serial.println("SMS inviato con successo a " + numero);
        return;
      }
    }
  }
  Serial.println("ERRORE: invio SMS fallito verso " + numero);
}

// EEPROM - scrittura
void writeStringToEEPROM(int addr, String data) {
  for (int i = 0; i < data.length(); i++) {
    EEPROM.write(addr + i, data[i]);
  }
  EEPROM.write(addr + data.length(), '\0');
}

// EEPROM - lettura
String readStringFromEEPROM(int addr) {
  char data[20];
  int len = 0;
  unsigned char k = EEPROM.read(addr);
  while (k != '\0' && len < 19) {
    data[len++] = k;
    k = EEPROM.read(addr + len);
  }
  data[len] = '\0';
  return String(data);
}

// Reset software della CPU
void resetCPU() {
  void(* Riavvia)(void) = 0;
  Riavvia();
}

