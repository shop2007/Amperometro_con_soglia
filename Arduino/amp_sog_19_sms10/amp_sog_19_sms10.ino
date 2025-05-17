String Versione = "amp_sog_19_sms10";
/*
SMS partono solo con pc connesso alla USB, ritardo partenza Serial.Software*/
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial sim800(9, 11); // RX, TX
// Imposta l'indirizzo I2C del modulo (generalmente 0x27 o 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

//********************************************************************************************
// PIN di accesso via SMS
#define PinUtente1 "User1"
#define PinUtente2 "User2"
#define PinDati    "Statoattuale"
#define PinReset   "Resetsystem"
#define DeleteUtente2 "eleteuser2"

//********************************************************************************************
// EEPROM indirizzi Numeri telefonici UTENTE1 UTENTE2 in EEPROM
#define EEPROM_ADDR_NUM1 0
#define EEPROM_ADDR_NUM2 20
// Indirizzo EEPROM per il contatore SMS
const int addrContaSms = 40; // Scegli un indirizzo che non si sovrapponga agli altri
bool PrimoSmsDaTrasmettere = true; // appena viene trasmesso il primo sms di allarme si mette false
//********************************************************************************************
bool SimulaSensCurrPotenziom = false; //se true usa i valori del potenziometro al posto di quello dei sensori
bool LcdPresent = true; //se presente lcd
bool SimulaTensionePotenziom = false; //se true usa lettura potenziometro al posto della tensione

bool StampaOgniSecondoAbilitata = false;
//********************************************************************************************

// Numeri registrati
String numero1 = "+393333333333";
String numero2 = "+393477777777";

// Variabili per il messaggio
char msgdati[100];
int ContaSMS = 3;
char StrSogliaMilliampere[5] = "1500";
char strContaSecondiCorrenteElevata[5] = "0123";
char strCurrent[6] = "0250";
char strVoltage[5] = "12.5";
unsigned int ContaSecondi = 0;

float floatVoltage;
int intCurrent;
int intSogliaMilliampere;
unsigned int ContaSecondiCorrenteElevata = 0;
// Timer per ContaSecondi
unsigned long ultimoMillis = 0;
//const unsigned long intervalloSecondi = 1000;
#define intervalloSecondi 1000




int buttonState = 0;  // variable for reading the pushbutton status
bool BuzzerIsDisabled = false;  // se vale True il buzzer non suona
//********************************************************************************************
//Un potenziometro connesso all'ADC stabilisce il valore in milliAmpere di soglia
//Taratura rispetto all'Hardware
//const int ValoreAdcSoglia1ampere = 0x03ff;  // Valore Adc Corrente Positiva 1 ampere, da tarare per interpolazione lineare
//const int ValoreAdcSoglia0ampere = 0x0;     // Valore Adc Corrente 0 Ampere, da tarare
#define ValoreAdcSoglia1ampere 0x03ff
  // Valore Adc Corrente Positiva 1 ampere, da tarare per interpolazione lineare
#define ValoreAdcSoglia0ampere 0x0
     // Valore Adc Corrente 0 Ampere, da tarare

unsigned int adcValue ; //  valore dall'ADC


//********************************************************************************************
//Pinout di Arduino
#define Buzzer 2 // cicalino
#define buttonPin 4 //
#define Rele_Buzzer 5
#define Rele_Contatto 6

#define Led_Verde 13
#define Led_Giallo 10
#define Led_Blu 7
#define Led_Bianco 12

#define AdcSensorCurrent A2
#define AdcVoltageBattery A3
#define AdcPotenziometro A7

// Definizione delle frequenze per le note (in Hz)
#define NOTE_DO 523  // Do - 523 Hz
#define NOTE_RE 587  // Re - 587 Hz
#define NOTE_MI 659  // Mi - 659 Hz
#define NOTE_FA 698  // Fa - 698 Hz
#define NOTE_SOL 784 // Sol - 784 Hz
#define NOTE_LA 880 // Sol - 784 Hz
#define NOTE_WAIT 1200 //

int noteDuration = 500; //durata delle note
//********************************************************************************************

//*******************************************************
void WelcomeDisplay(){
    if (LcdPresent){
      lcd.setCursor(0, 0); // Posiziona il cursore sulla seconda riga, prima colonna
      lcd.print("Welcome ");

      lcd.setCursor(0, 1); // Posiziona il cursore sulla seconda riga, prima colonna
      lcd.print(Versione);
    }
}

//************************
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

// Funzioni per la gestione del contatore SMS in EEPROM
void writeContaSmsEEPROM(int value) {
  EEPROM.put(addrContaSms, value);
  Serial.print(F("Scritto in EEPROM ContaSmsEEprom indirizzo "));
  Serial.print(addrContaSms);
  Serial.print(F(" Valore "));
  Serial.println(value);

}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
int readContaSmsEEPROM() {
  int value;
  EEPROM.get(addrContaSms, value);

  Serial.print(F("Letto da EEPROM ContaSmsEEprom indirizzo "));
  Serial.print(addrContaSms);
  Serial.print("");
  Serial.print(F(" Valore "));
  Serial.println(value);
  return value;
}

//---------------------------------------------------------
void SirenaFranceseVeloce(){

  noteDuration=150;
  playTone(NOTE_MI, noteDuration);
  playTone(NOTE_LA, noteDuration);
  playTone(NOTE_MI, noteDuration);
  playTone(NOTE_LA, noteDuration);

  noTone(8); //fermo il tono

}

//------------------------------------


// Funzione per suonare una nota
void playTone(int frequency, int duration) {
  if (!BuzzerIsDisabled){
    tone(Buzzer, frequency, duration); // Genera il suono sul pin
    delay(duration);                      // Attendi la durata della nota
  }
  noTone(Buzzer);                    // Ferma il suono
}
//********************************
//---------------------------------------------------------
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//-----------------------------------------------

//---------------------------------------------------------
 void ProcessaPotenziometroSoglia(int valoreAdcLetto){ 
 // Il potenziometro che imposta la soglia viene letto e si calcola una corrispondente soglia in milliAmpere , da 0 a 1000
  // Calcola il valore della soglia in corrente (in milliampere) range da 0 a 1000 mA
  float FloatSogliaMilliampere = 999; // Cambia questo valore per testare
  FloatSogliaMilliampere = (valoreAdcLetto * 1000.0) / (ValoreAdcSoglia1ampere - ValoreAdcSoglia0ampere);

  // Converti il float in un intero
  intSogliaMilliampere = static_cast<int>(FloatSogliaMilliampere);

  // Usa snprintf per formattare la stringa
  snprintf(StrSogliaMilliampere, sizeof(StrSogliaMilliampere), "%4d", intSogliaMilliampere);

  // Stampa i risultati sul monitor seriale
  if (StampaOgniSecondoAbilitata){
    Serial.print(F("ADC letto (hex): 0x"));
    Serial.print(valoreAdcLetto, HEX); // Stampa il valore ADC in esadecimale

    Serial.print(F(" - Float Soglia (mA): "));
    Serial.print(FloatSogliaMilliampere); // Stampa lo scostamento in decimale

    Serial.print(F(" - int Soglia (mA): "));
    Serial.print(intSogliaMilliampere); // Stampa lo scostamento in decimale

    Serial.print(F(" - Strings Soglia (mA): "));
    Serial.println(StrSogliaMilliampere); // Stampa lo scostamento in decimale
  }
 }




//---------------------------------------------------------

void ProcessaSensoreCorrente(unsigned int adcValue){
  float currentSensor;
  // Calcola la tensione corrispondente al valore ADC

  float floatVoltageSensore = adcValue * (5.0 / 1023.0);

  // Calcola lo scostamento in bit dallo zero (2.5V)
  int deltaBit = adcValue - 512; // 512 è il valore ADC corrispondente a 2.5V
  if (StampaOgniSecondoAbilitata){
    Serial.print(F("AdcCurr="));Serial.print(adcValue);
    Serial.print(F(" - floatVoltageSensore="));Serial.print(floatVoltageSensore);   
    Serial.print(F(" - deltaBit="));Serial.print(deltaBit);
  }
  // Interpolazione lineare tra i punti dati:
  // (2.07V, -350mA) e (2.5V, 0mA)
  // (2.5V, 0mA) e (3.06V, +450mA)

  if (floatVoltageSensore <= 2.5) {
    // Calcola la corrente per valori di tensione <= 2.5V
    currentSensor = mapFloat(floatVoltageSensore, 2.07, 2.5, -350.0, 0.0);
  } else {
    // Calcola la corrente per valori di tensione > 2.5V
    currentSensor = mapFloat(floatVoltageSensore, 2.5, 3.06, 0.0, 450.0);
  }  

    // Approssima il float al numero intero più vicino
  intCurrent = static_cast<int>(round(currentSensor));
  
  // Usa snprintf per formattare la stringa
  // '%+5d' assicura che il numero sia allineato a destra con segno
  snprintf(strCurrent, sizeof(strCurrent), "%+5d", intCurrent);

  // Sostituisci il '+' con uno spazio
  /*
  if (strCurrent[0] == '+') {
    strCurrent[0] = ' ';
  }
  */

  // Stampa il risultato
  if (StampaOgniSecondoAbilitata){
    Serial.print(F(" - currentSensor="));Serial.print(currentSensor);
    Serial.print(F(" - intCurrent="));Serial.print(intCurrent);
    Serial.print(F(" - strCurrent='"));  Serial.print(strCurrent); Serial.println(F("'"));
  }



}
//----------------------------------------------
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//----------------------------------------------

void ControllaSogliaSuperata() {

  intCurrent = abs(intCurrent); // toglie il segno
  if (StampaOgniSecondoAbilitata){
    Serial.print(F("intCurrent=")); Serial.println(intCurrent);
    Serial.print(F(" - Soglia=")); Serial.println(intSogliaMilliampere);
  }

  // Controllo della corrente rispetto alla soglia
  if (intCurrent < intSogliaMilliampere) {
      if (StampaOgniSecondoAbilitata){
        Serial.println(F(" - Corrente inferiore soglia"));
      }
      // Se la corrente è inferiore alla soglia
      digitalWrite(Buzzer, LOW); // Spegne il Tone
      digitalWrite(Rele_Contatto, LOW); // Stacca il relay
      digitalWrite(Rele_Buzzer, LOW); // Stacca il relay
      noTone(Buzzer); // Spegne il Tone
      Led_giallo_off();
      Led_blu_off();
      Led_bianco_off();
      Led_verde_off();
      
  } else {
      if (StampaOgniSecondoAbilitata){
        Serial.println(F(" - Corrente SUPERIORE soglia"));
      }
      // Se la corrente è maggiore o uguale alla soglia
      digitalWrite(Rele_Contatto, HIGH); // Alimenta il relay
      digitalWrite(Rele_Buzzer, HIGH); // Alimenta il relay
      ContaSecondiCorrenteElevata++;

      //Si trasmette sms solo una volta
      if (PrimoSmsDaTrasmettere){
        PrimoSmsDaTrasmettere = false; // non se ne manderà un altro
        Led_bianco_on();
        Serial.println(F("Trasmissione primo sms in corso"));
          CostruisciMsgDati();
          sendSMS(numero1, String(msgdati));
          sendSMS(numero2, String(msgdati));
      }
      


      if (ContaSecondi % 2 == 0) {
        playTone(NOTE_MI, 500);
        //tone(Buzzer, 1000); // Suona Tone (frequenza di esempio)
        Led_giallo_on();
        Led_blu_on();
        Led_bianco_on();
        Led_verde_off();
      } else {
        playTone(NOTE_LA, 500);
        //noTone(Buzzer); // Spegne il Tone
        Led_giallo_off();
        Led_blu_off();
        Led_bianco_off();
        Led_verde_on();
      }





  } 
  





}

//---------------------------------------------------------
void ProcessaAdcTensione(unsigned int adcValue) {
  // Interpolazione lineare tra i punti dati:
  // (20 bit, 2V) e (500 bit, 12V)  floatVoltage = mapFloat2(adcValue, 20, 500, 2.0, 12.0);
  // nuovo partitore 5,05v 211bit e 12.0v 535bit
  floatVoltage = mapFloat2(adcValue, 211, 535, 5.05, 12.0);

  // Approssima il valore a una cifra decimale
  float roundedVoltage = round(floatVoltage * 10.0) / 10.0;

  // Converte il valore approssimato in una stringa temporanea
  char tempStr[6]; // Buffer temporaneo leggermente più grande per la conversione
  dtostrf(roundedVoltage, 4, 1, tempStr); // Usa 4 caratteri di larghezza, 1 cifra decimale

  // Assicurati che ci sia sempre una cifra decimale e un punto
  int len = strlen(tempStr);
  if (tempStr[len - 2] == '.') {
      // Se c'è un punto decimale seguito da uno zero, nulla da fare
      // Altrimenti, aggiungi uno zero
      if (tempStr[len - 1] == ' ') {
          tempStr[len - 1] = '0';
      }
  }

  // Copia la stringa nel buffer finale allineata a destra
  snprintf(strVoltage, 5, "%4s", tempStr);

  if (StampaOgniSecondoAbilitata){
    Serial.print(F("adcValue=")); Serial.print(adcValue);
    Serial.print(F(" - floatVoltage=")); Serial.print(floatVoltage);
    Serial.print(F(" - roundedVoltage=")); Serial.print(roundedVoltage);
    Serial.print(F(" - strVoltage=")); Serial.print(strVoltage);
    Serial.println();
  }

  

}


//------------------------------------------------------------------------------


//-----------------------------------------------






//-----------------------------------------------
void Azione7(void){

  //prova la funzione di conversione della tensione 
  for (int i =0 ; i<1024;i++){
    i=i+16;
    //ProcessaSensore(i);
    adcValue = i;
    ProcessaAdcTensione(adcValue);
  } 
}
//-----------------------------------------------

void Azione8(void){

}
//-----------------------------------------------


void Azione9(void){


}
//-----------------------------------------------

void Azione10(void){
  //legge l'AD connesso al ptenziometro


}
//-----------------------------------------------

//----------------------
void Buzzer_on(){
  digitalWrite(Buzzer, HIGH); // 4
}  

void Buzzer_off(){
  digitalWrite(Buzzer, LOW); // 4
}  
//---------------
void Led_giallo_on(){
  digitalWrite(Led_Giallo, HIGH); // 4
}  

void Led_giallo_off(){
  digitalWrite(Led_Giallo, LOW); // 4
}  
//---------------
void Led_verde_on(){
  digitalWrite(Led_Verde, HIGH); // 
}  

void Led_verde_off(){
  digitalWrite(Led_Verde, LOW); // 
}  
//---------------
void Led_blu_on(){
  digitalWrite(Led_Blu, HIGH); // 
}  

void Led_blu_off(){
  digitalWrite(Led_Blu, LOW); // 
}  
//---------------
//---------------
void Led_bianco_on(){
  digitalWrite(Led_Bianco, HIGH); // 
}  

void Led_bianco_off(){
  digitalWrite(Led_Bianco, LOW); // 
}  
//---------------
//------------------------------------------------------------------------------

float mapFloat2(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//-------------------------------------------------------------------------

// Pulsante su pin 2
//const int pulsantePin = 2;
//bool statoPrecedente = HIGH;

void setup() {



  //pinMode(Buzzer, OUTPUT); // 3 // cicalino
  pinMode(Led_Giallo, OUTPUT); // 4
  pinMode(Led_Verde, OUTPUT); // 5
  pinMode(Led_Blu, OUTPUT); // 5
  pinMode(Led_Bianco, OUTPUT); // 5
  pinMode(Rele_Contatto, OUTPUT); // 7
  pinMode(Rele_Buzzer, OUTPUT); // 7
  pinMode(buttonPin, INPUT_PULLUP);

  digitalWrite(Buzzer, LOW); // 3 // cicalino
  digitalWrite(Led_Giallo, LOW); // 4
  digitalWrite(Led_Verde, LOW); // 5
  digitalWrite(Led_Blu, LOW); // 5
  digitalWrite(Led_Bianco, LOW); // 5
  digitalWrite(Rele_Contatto, LOW); // 6
  digitalWrite(Rele_Buzzer, LOW); // 6

  // set the digital pin as output:
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
 
  Serial.println(F("RESET"));
  Serial.print(F("Nome file: "));
  Serial.println(__FILE__);

  Serial.println();
  
  if(SimulaSensCurrPotenziom){
    Serial.println(F("OFF = ADC reali"));
  } else {
    Serial.println(F("ON = ADC su potenz."));
  }

  if (LcdPresent){
    Serial.println(F("lcd present"));

    lcd.begin();          // Inizializza il display
    lcd.backlight();     // Accende la retroilluminazione

    WelcomeDisplay();
  } else
  {
    Serial.println(F("NO lcd"));
  }
  // Recupera numeri telefonici da EEPROM
  numero1 = readStringFromEEPROM(EEPROM_ADDR_NUM1);
  numero2 = readStringFromEEPROM(EEPROM_ADDR_NUM2);

  Serial.print(F("Numero Utente1: ")); Serial.println(numero1);
  Serial.print(F("Numero Utente2: ")); Serial.println(numero2);

  // Legge il contatore SMS da EEPROM e lo scrive in ContaSMS
  ContaSMS = readContaSmsEEPROM();
  Serial.print(F("ContaSMS dopo reset:"));
  Serial.println(ContaSMS);
  delay(1000);
  sim800.begin(9600);
  //ritardo perchè la sim senza usb connessa parte dopo
  for (int i = 1; i < 8; i++) {
    Led_bianco_on();
    delay (100);
    Led_bianco_off();
    delay(900);
  }
  //delay(7000);



  //inizializza la sim
  sim800.println("AT+CMGF=1");        // Modalità testo
  delay(1000);
  sim800.println("AT+CNMI=1,2,0,0,0"); // Ricezione immediata SMS
  delay(1000);

  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW){
    BuzzerIsDisabled=true;
    Serial.println(F("Buzzer disabilitato"));
    SirenaFranceseVeloce();
  }

}

// Dichiarazione della variabile per tenere traccia dello stato del pin


void loop() {
  
/*
void LetturaAnalogiche(){
  Azione4(); //soglia potenziometro
  Azione6(); //legge corrente sensore
  Azione8(); //legge tensione batteria
  Azione9(); //legge button, se premuto azzera ContaSecondiCorrenteElevata
  Azione10(); //compara corrente sensore con soglia potenziometro
  Serial.print(" - ContaSecondiCorrenteElevata= ");Serial.println(ContaSecondiCorrenteElevata);
}
*/
  LetturaAnalogiche();
  // Incrementa ContaSecondi ogni secondo
  if (millis() - ultimoMillis >= intervalloSecondi) {
    ultimoMillis = millis();
    ContaSecondi++;
    StampaOgniSecondoAbilitata = true; //abilita le stampe di debug, ma solo ogni secondo
    
    Serial.println();
    Serial.println(ContaSecondi);
    if (ContaSecondi % 50 == 0) {
      //va acapo per non sforare con il terminale
      Serial.println();
    }
  }

  // Controllo SMS
  if (sim800.available()) {
    Led_bianco_on();
    String raw = sim800.readString();
    raw.trim();
    Serial.print(F("raw: >")); Serial.print(raw);Serial.println(F("<"));

    if (raw.indexOf("+CMT:") != -1) {
      int msgStart = raw.indexOf("\n", raw.indexOf("\n") + 1);
      Serial.print(F("msgStart: "));Serial.println(msgStart);
      String contenuto = "";
      if (msgStart != -1) {
          contenuto = raw.substring(msgStart);
          contenuto.trim(); // ✅ CORRETTO
          Serial.print(F("contenuto: ")); Serial.println(contenuto);
      }

      int index = raw.indexOf("+39");
      if (index != -1) {
        String numero = raw.substring(index, index + 13);
        Serial.print(F("numero: ")); Serial.println(numero);
        Serial.print(F("raw2: >")); Serial.print(raw);Serial.println(F("<"));


        String TempString ="xxxxx";

        TempString = PinUtente1;
        if (raw.indexOf(TempString) != -1) {
          Serial.println(F("Match PinUtente1="));
          numero1 = numero;
          writeStringToEEPROM(EEPROM_ADDR_NUM1, numero1);
          sendSMS(numero1, "Utente1 registrato correttamente.");
        }

        TempString = PinUtente2;
        if (raw.indexOf(TempString) != -1) {
          Serial.println(F("Match PinUtente2"));
          numero2 = numero;
          writeStringToEEPROM(EEPROM_ADDR_NUM2, numero2);
          sendSMS(numero2, "Utente2 registrato correttamente.");
        }

        TempString = DeleteUtente2;
        if (raw.indexOf(TempString) != -1) {
          Serial.println(F("Match DeleteUtente2"));
          numero2 = "+39";
          writeStringToEEPROM(EEPROM_ADDR_NUM2, numero2);
          sendSMS(numero1, "Utente2 cancellato correttamente.");
        }

        TempString = PinDati;
        if (raw.indexOf(TempString) != -1) {
          Serial.println(F("Match PinDati"));
          CostruisciMsgDati();
          sendSMS(numero, String(msgdati));
        }

        TempString = PinReset;
        if (raw.indexOf(TempString) != -1) {
            Serial.println(F("Match PinReset"));
            sendSMS(numero1, "Reset in corso...");
            delay(1000);
            sim800.println("AT+CFUN=1,1");
            delay(1000);
            resetCPU();         
        }
      }
    }
  }
  Led_bianco_off();
}

// Crea msgdati con tutte le variabili
void CostruisciMsgDati() {
  snprintf(msgdati, sizeof(msgdati),
           "SMS:%d | Soglia:%smA | Current:%smA | Time.alarm:%s | Volt:%s | Time.tot:%u",
           ContaSMS,
           StrSogliaMilliampere,
           strCurrent,
           strContaSecondiCorrenteElevata,
           strVoltage,
           ContaSecondi);
}

// Invia SMS
void sendSMS(String numero, String messaggio) {
  Led_bianco_on();
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z
  delay(3000);
  Led_bianco_off();

  // Verifica invio
  unsigned long tempoInizio = millis();
  String risposta = "";
  while (millis() - tempoInizio < 5000) {
    if (sim800.available()) {
      char c = sim800.read();
      risposta += c;
      if (risposta.indexOf("+CMGS:") != -1) {
        Serial.print(F("SMS inviato con successo a "));
        Serial.println(numero);
        return;
      }
    }
  }
  Serial.print(F("ERRORE: invio SMS fallito verso "));
  Serial.println(numero);

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

//*******************************************************************************
void LetturaAnalogiche(){

//**********
  //Azione4(); //soglia potenziometro
  //usa la funzione  ProcessaPotenziometroSoglia

  //legge l'AD connesso al ptenziometro
  int valoreAnalogico = analogRead(AdcPotenziometro);

  // lo converte
  ProcessaPotenziometroSoglia(valoreAnalogico);

  // lo manda al display LCD riga 1 posizione 12
  if (LcdPresent){
  
    lcd.setCursor(8, 0); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(" SmA");

    lcd.setCursor(12, 0); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(StrSogliaMilliampere);

  }
//**********
  //Azione6(); //legge corrente sensore
/*
  #define AdcSensorCurrent A2
  #define AdcVoltageBattery A3
  #define AdcPotenziometro A7
*/
  if(SimulaSensCurrPotenziom){
    //in debug legge l'AD connesso al potenziometro
    adcValue = analogRead(AdcPotenziometro);
  } else {
    //run time legge l'AD connesso al sensore corrente
    adcValue = analogRead(AdcSensorCurrent);    
  }

  ProcessaSensoreCorrente(adcValue);

  // lo manda al display LCD riga 1 posizione 12
  if (LcdPresent){
    lcd.setCursor(0, 0); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print("ImA=");

    lcd.setCursor(3, 0); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(strCurrent);
  }  
//**********  
  //Azione8(); //legge tensione batteria
/*
  #define AdcSensorCurrent A2
  #define AdcVoltageBattery A3
  #define AdcPotenziometro A7
*/

  if(SimulaTensionePotenziom){
    //in SimulaTensionePotenziom legge l'AD connesso al potenziometro
    adcValue = analogRead(AdcPotenziometro);
  } else {
    //run time legge l'AD connesso al sensore tensione
    adcValue = analogRead(AdcVoltageBattery);    
  }

  ProcessaAdcTensione(adcValue);

  // lo manda al display LCD riga 1 posizione 12
  if (LcdPresent){
    lcd.setCursor(0, 1); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print("Volt");

    lcd.setCursor(4, 1); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(strVoltage);
  }


//**********  
  //Azione9(); //legge button, se premuto azzera ContaSecondiCorrenteElevata
  //test button
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    Serial.println(F("Button premuto"));
    ContaSecondiCorrenteElevata=0;
    if (LcdPresent){
      lcd.setCursor(0, 0); // Posiziona il cursore sulla seconda riga, prima colonna
      lcd.print("RESET IN CORSO  ");

      lcd.setCursor(0, 1); // Posiziona il cursore sulla seconda riga, prima colonna
      lcd.print(Versione);
    }
    delay(1000);
    sim800.println("AT+CFUN=1,1");
    delay(2000);
    resetCPU(); 




  } else {
    // turn LED off:
    if (StampaOgniSecondoAbilitata){
      Serial.println(F("Button rilasciato"));
    }
    
  }
  

//**********  
  //Azione10(); //compara corrente sensore con soglia potenziometro
  
  ControllaSogliaSuperata();
  if(ContaSecondiCorrenteElevata>9999){
    ContaSecondiCorrenteElevata=9999; //satura a 9999
  }
  sprintf(strContaSecondiCorrenteElevata, "%04u", ContaSecondiCorrenteElevata);

  // lo manda al display LCD riga 1 posizione 12
  if (LcdPresent){
    lcd.setCursor(8, 1); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(" Err");

    lcd.setCursor(12, 1); // Posiziona il cursore sulla prima riga, colonna 1
    lcd.print(strContaSecondiCorrenteElevata);
  }

  if (StampaOgniSecondoAbilitata){
    Serial.print(F(" - Conta_Secondi_Corrente_Elevata= "));Serial.println(ContaSecondiCorrenteElevata);
  }
  StampaOgniSecondoAbilitata = false; //questa era l'ultima stampa di debug
//**********
}