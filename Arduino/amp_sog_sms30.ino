// Unione sketch SIM800 + controllo corrente con LEM + LCD
#include <SoftwareSerial.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

SoftwareSerial sim800(7, 8); // RX, TX
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Pin
const int relePin = 4;
const int buzzerPin = 5;
const int sogliaPin = A1;
const int lemPin = A0;
const int batteryPin = A2;

// SMS
const String PinUtente1 = "12345";
const String PinUtente2 = "54321";
const String PinDati = "11111";
const String PinReset = "88888";

String numero1 = "";
String numero2 = "";
char numero1eeprom[14];
char numero2eeprom[14];

char msgdati[100];
char StrSogliaMilliampere[5];
char strContaSecondiCorrenteElevata[5];
char strCurrent[6];
char strVoltage[5];

int ContaSMS = 0;
unsigned int ContaSecondi = 0;
bool allarmeInviato = false;

void setup() {
  Serial.begin(9600);
  sim800.begin(9600);

  pinMode(relePin, OUTPUT);
  pinMode(buzzerPin, OUTPUT);
  lcd.init();
  lcd.backlight();

  // Carica numeri da EEPROM
  EEPROM.get(0, numero1eeprom);
  EEPROM.get(20, numero2eeprom);
  numero1 = String(numero1eeprom);
  numero2 = String(numero2eeprom);

  lcd.setCursor(0, 0);
  lcd.print("Sistema avviato");
  delay(2000);
  lcd.clear();
}

void loop() {
  // Letture analogiche
  int valoreLEM = analogRead(lemPin);
  int valoreSoglia = analogRead(sogliaPin);
  int valoreBatt = analogRead(batteryPin);

  int corrente = map(valoreLEM, 0, 1023, 0, 5000); // in mA
  int soglia = map(valoreSoglia, 0, 1023, 0, 5000);
  int tensione = map(valoreBatt, 0, 1023, 0, 250); // 25.0V max

  snprintf(strCurrent, sizeof(strCurrent), "%04d", corrente);
  snprintf(StrSogliaMilliampere, sizeof(StrSogliaMilliampere), "%04d", soglia);
  snprintf(strVoltage, sizeof(strVoltage), "%03d", tensione);

  lcd.setCursor(0, 0);
  lcd.print("C:"); lcd.print(strCurrent); lcd.print(" S:"); lcd.print(StrSogliaMilliampere);
  lcd.setCursor(0, 1);
  lcd.print("V:"); lcd.print(strVoltage); lcd.print(" T:"); lcd.print(ContaSecondi);

  if (corrente > soglia) {
    ContaSecondi++;
    digitalWrite(relePin, HIGH);
    digitalWrite(buzzerPin, HIGH);

    if (!allarmeInviato) {
      CostruisciMsgDati();
      if (numero1.length() == 13) sendSMS(numero1, String(msgdati));
      if (numero2.length() == 13) sendSMS(numero2, String(msgdati));
      allarmeInviato = true;
    }
  } else {
    digitalWrite(relePin, LOW);
    digitalWrite(buzzerPin, LOW);
  }

  riceviSMS();

  delay(1000);
}

void CostruisciMsgDati() {
  snprintf(strContaSecondiCorrenteElevata, sizeof(strContaSecondiCorrenteElevata), "%04d", ContaSecondi);
  snprintf(msgdati, sizeof(msgdati), "SMS:%02d Soglia:%s mA Alti:%s Corr:%s V:%s Tsec:%d",
           ContaSMS++, StrSogliaMilliampere, strContaSecondiCorrenteElevata, strCurrent, strVoltage, ContaSecondi);
}

void riceviSMS() {
  if (sim800.available()) {
    String sms = sim800.readString();
    sms.trim();
    String mittente = "+39xxxxxxxxxxx"; // semplificato per esempio

    if (sms.indexOf(PinUtente1) != -1) {
      numero1 = mittente;
      numero1.toCharArray(numero1eeprom, 14);
      EEPROM.put(0, numero1eeprom);
      sendSMS(mittente, "Numero1 salvato");
    } else if (sms.indexOf(PinUtente2) != -1) {
      numero2 = mittente;
      numero2.toCharArray(numero2eeprom, 14);
      EEPROM.put(20, numero2eeprom);
      sendSMS(mittente, "Numero2 salvato");
    } else if (sms.indexOf(PinDati) != -1) {
      CostruisciMsgDati();
      sendSMS(mittente, String(msgdati));
    } else if (sms.indexOf(PinReset) != -1) {
      sendSMS(numero1, "Reset in corso...");
      sim800.println("AT+CFUN=1,1");
      delay(1000);
      asm volatile ("jmp 0");
    }
  }
}

void sendSMS(String numero, String messaggio) {
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z
  delay(3000);

  unsigned long t0 = millis();
  String risposta = "";
  while (millis() - t0 < 5000) {
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
