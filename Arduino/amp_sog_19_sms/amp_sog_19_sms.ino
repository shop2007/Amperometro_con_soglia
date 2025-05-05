String Versione = "amp-sogl-19_sms";
/*
= alla vers. amp-sogl-17 ma con librerie in locale nel progetto
Per entrare nel menu scrivere "Menu" a 115200 baud

Nota: se debug è attivo anzichè i valori reali di corrente e tensione, viene letto il potenziometro

Funzioni principali:
- Legge la corrente da un sensore LEM, nel range +/- 1000 milliAmpere
- Legge una soglia impostata su un potenziometro, nel range da 0 a 1000 milliAmprer
- Legge la tensione di batteria, nel range da 1 a 15 volt
- Una volta al secondo verifica se la corrente circolante è maggiore della soglia impostata, facendo queste cose:
  - Conta i secondi in cui la corrente supera la sogia 
  - Suona il cicalino
  - Chiude un relay

  I 4 valori (Corrente, Soglia, Tensione, Secondi over soglia)  vengono visualizzati su un display 

  Premendo il pulsante è possibile azzerare il conta secondi over soglia.

Al reset se si parte con il pulsante premuto si disablita il buzzer


*/
#include <Wire.h>
#include "LiquidCrystal_I2C.h"
#include <SoftwareSerial.h>
#include <EEPROM.h>

// SIM800L su pin 7 RX, 8 TX (adatta se usi diversi pin)
SoftwareSerial sim800(10, 11);

// Imposta l'indirizzo I2C del modulo (generalmente 0x27 o 0x3F)
LiquidCrystal_I2C lcd(0x27, 16, 2);

bool DebugOn=false; //se true usa i valori del potenziometro al posto di quello dei sensori
int buttonState = 0;  // variable for reading the pushbutton status
bool BuzzerIsDisabled = false;  // se vale True il buzzer non suona
//********************************************************************************************
//Un potenziometro connesso all'ADC stabilisce il valore in milliAmpere di soglia
//Taratura rispetto all'Hardware
const int ValoreAdcSoglia1ampere = 0x03ff;  // Valore Adc Corrente Positiva 1 ampere, da tarare per interpolazione lineare
const int ValoreAdcSoglia0ampere = 0x0;     // Valore Adc Corrente 0 Ampere, da tarare

unsigned int adcValue ; //  valore dall'ADC

float currentSensor;




//--------------------------------------------
//Due banane connettono i due poli della batteria su auto tramite un diodo ed un partitore di tensione 33k/10k

//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
// risultati

//

float floatVoltage;


char strVoltage[5]; // 4 caratteri + 1 per il terminatore di stringa

int intCurrent;
char strCurrent[6]; // 5 caratteri + 1 per il terminatore di stringa

int intSogliaMilliampere; 
char StrSogliaMilliampere[5]; // 4 caratteri + 1 per il terminatore di stringa

unsigned int ContaSecondi = 0;
unsigned int ContaSecondiCorrenteElevata = 0;

char strContaSecondiCorrenteElevata[5];


//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------
//------------------------------------------------------------



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
// Dichiarazione della variabile per tenere traccia dello stato del pin


String inputString = "";      // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
bool ExitFromMenu = false;    // Used for exit from loop 
char Cestino;                 // 
const long interval = 1000;           // interval at which to blink (milliseconds)

bool debug = false;
String inputBuffer = "";       // Buffer per memorizzare i caratteri ricevuti

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;  // will store last time LED was updated

int counter = 1000;
//------------------------------------

//PROTOTIPI
void Azione1(void);
void Azione2(void);
void Azione3(void);
void Azione4(void);
void Azione5(void);
void Azione6(void);
void Azione7(void);
void Azione8(void);
void Azione9(void);
void Azione10(void);
void Azione11(void);
void Azione12(void);
void Azione13(void);
void Azione14(void);
void Azione15(void);
void Azione16(void);
void Azione17(void);
void Azione18(void);
void Azione19(void);
void Azione20(void);
void Azione99(void);
void Menu(void);
void WelcomeDisplay();
void serialEvent(void); 



//----------------------------------------------



//----------------------------------------------

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
//verifica se devo entrare nel menu
void CheckIfMenu(){
  // Controlla i dati ricevuti sulla seriale
  if (Serial.available() > 0) {
    char receivedChar = Serial.read(); // Legge un carattere
    inputBuffer += receivedChar;       // Aggiunge il carattere al buffer

    // Verifica se il buffer contiene la parola "Menu"
    if (inputBuffer.endsWith("Menu")) {
      Serial.println("Comando 'Menu' rilevato! Entrando nella funzione Menu().");
      Menu(); // Chiama la funzione Menu()
      inputBuffer = ""; // Resetta il buffer
    }

    // Evita che il buffer diventi troppo lungo
    if (inputBuffer.length() > 20) {
      inputBuffer = ""; // Resetta il buffer in caso di overflow
    }
  }

}


//-------------------------------------------------------------------
void loop(){
  WelcomeDisplay();
  
  //testa tutti gli output
  Led_giallo_on();
  Led_blu_on();
  Led_bianco_on();
  Led_verde_on();

  SirenaFrancese();
  delay(500);
  
  Led_giallo_off();
  Led_blu_off();
  Led_bianco_off();
  Led_verde_off();

  while (true) {


    // Controllo SMS in arrivo
    if (sim800.available()) {
      String sms = sim800.readString();
      Serial.println("SMS ricevuto: " + sms);
      processSMS(sms);
    }

    // Esegui la funzione desiderata
    CheckIfMenu();//  verifica se devo entrare nel menu
    delay(50);
    CheckIfMenu();//  verifica se devo entrare nel menu
    delay(50);
    CheckIfMenu();//  verifica se devo entrare nel menu
    delay(50);
    CheckIfMenu();//  verifica se devo entrare nel menu
    delay(50);
    CheckIfMenu();//  verifica se devo entrare nel menu
    delay(50);
    Azione4(); //soglia potenziometro
    Azione6(); //legge corrente sensore
    Azione8(); //legge tensione batteria
    Azione9(); //legge button, se premuto azzera ContaSecondiCorrenteElevata

    //ogni secondo
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ContaSecondi++;
      Azione10(); //compara corrente sensore con soglia potenziometro
      Serial.print("ContaSecondi= ");Serial.print(ContaSecondi);
      Serial.print(" - ContaSecondiCorrenteElevata= ");Serial.print(ContaSecondiCorrenteElevata);
      
    } 
  }

  Menu();
  //  Azione15();
}

//------------------------------------
/*
Nei sistemi che utilizzano numeri a virgola mobile, confrontare due numeri float direttamente usando l'operatore di uguaglianza == può 
 essere problematico a causa delle imprecisioni legate alla rappresentazione in virgola mobile.
Per confrontare due numeri float, è meglio verificare se la differenza assoluta tra i due numeri è inferiore a una soglia (detta epsilon). 
Questo metodo consente di considerare due numeri "uguali" se sono sufficientemente vicini tra loro.
*/
bool compareFloats(float a, float b, float epsilon = 0.0001) {
  return fabs(a - b) < epsilon;
}
//------------------------------------
/*
void LeggeAdcGrezzo(int QualeAdc){
  Serial.print("Lettura ADC ");Serial.println(QualeAdc);
  ClearSerialBuffer();
  while (true) {
    // Controlla se c'è un dato disponibile sulla seriale
    if (Serial.available() > 0) {
      char receivedChar = Serial.read(); // Legge il carattere inviato via seriale
      if (receivedChar == 'Q' || receivedChar == 'q') {
        Serial.println("Interruzione della lettura.");
        break; // Esce dal ciclo e dalla funzione
      }
    }
    Serial.println(analogRead(QualeAdc));
  }
}
*/
//------------------------------------






//-------------------------------------------------------------------
/*

*/

// funzioni
//---------------------------------------------------------
void SirenaFrancese(){

  noteDuration=400;
  playTone(NOTE_MI, noteDuration);
  playTone(NOTE_LA, noteDuration);
  playTone(NOTE_MI, noteDuration);
  playTone(NOTE_LA, noteDuration);
  noTone(8); //fermo il tono

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

//---------------------------------------------------------
//lampeggia ed attendein loop il tasto Q per continuare
void BlinkWaitQ(void){
  while (1) {
    char inChar = (char)Serial.read();

    if (inChar == 'q'){
      break;
    }

    delay(500);                       // wait for a second

    if (inChar == 'Q'){
      break;
    }
    
    delay(500); 
  
  }
}
//---------------------------------------------------------



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
  Serial.print("ADC letto (hex): 0x");
  Serial.print(valoreAdcLetto, HEX); // Stampa il valore ADC in esadecimale

  Serial.print(" - Float Soglia (mA): ");
  Serial.print(FloatSogliaMilliampere); // Stampa lo scostamento in decimale

  Serial.print(" - int Soglia (mA): ");
  Serial.print(intSogliaMilliampere); // Stampa lo scostamento in decimale

  Serial.print(" - Strings Soglia (mA): ");
  Serial.println(StrSogliaMilliampere); // Stampa lo scostamento in decimale
 }




//---------------------------------------------------------

void ProcessaSensoreCorrente(unsigned int adcValue){

  // Calcola la tensione corrispondente al valore ADC
  Serial.print("AdcCurr=");Serial.print(adcValue);
  
  float floatVoltageSensore = adcValue * (5.0 / 1023.0);
  Serial.print(" - floatVoltageSensore=");Serial.print(floatVoltageSensore);

  // Calcola lo scostamento in bit dallo zero (2.5V)
  int deltaBit = adcValue - 512; // 512 è il valore ADC corrispondente a 2.5V
  Serial.print(" - deltaBit=");Serial.print(deltaBit);

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
  Serial.print(" - currentSensor=");Serial.print(currentSensor);

    // Approssima il float al numero intero più vicino
  intCurrent = static_cast<int>(round(currentSensor));
  Serial.print(" - intCurrent=");Serial.print(intCurrent);

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
  Serial.print(" - strCurrent='");  Serial.print(strCurrent); Serial.println("'");




}
//----------------------------------------------
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

//----------------------------------------------

void ControllaSogliaSuperata() {

  intCurrent = abs(intCurrent); // toglie il segno
  Serial.println("intCurrent="); Serial.print(intCurrent);
  Serial.println(" - Soglia="); Serial.print(intSogliaMilliampere);

  // Controllo della corrente rispetto alla soglia
  if (intCurrent < intSogliaMilliampere) {
      Serial.println(" - Corrente inferiore soglia");
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
      Serial.println(" - Corrente SUPERIORE soglia");
      // Se la corrente è maggiore o uguale alla soglia
      digitalWrite(Rele_Contatto, HIGH); // Alimenta il relay
      digitalWrite(Rele_Buzzer, HIGH); // Alimenta il relay
      ContaSecondiCorrenteElevata++;

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

  Serial.print("adcValue="); Serial.print(adcValue);

  floatVoltage = mapFloat2(adcValue, 211, 535, 5.05, 12.0);
  Serial.print(" - floatVoltage="); Serial.print(floatVoltage);




  // Approssima il valore a una cifra decimale
  float roundedVoltage = round(floatVoltage * 10.0) / 10.0;
  Serial.print(" - roundedVoltage="); Serial.print(roundedVoltage);

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
  Serial.print(" - strVoltage="); Serial.print(strVoltage);

  Serial.println();

}


//------------------------------------------------------------------------------

float mapFloat2(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



//----------------------
void ClearSerialBuffer() {
  while (Serial.available() > 0) {
    Serial.read(); // Legge e scarta i dati nel buffer
  }
}
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
const char PrimaCifraValida = '3';  //numeri cellulari in italia inziano con 3
const String Pin = "19927";
const String PrefissoInt = "+39";

String NumeroUtente1;
String NumeroUtente2;

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


//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

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

//erial.println( F ("Attivato"));    
   //start serial connection
  Serial.begin(115200); // terminale

  sim800.begin(57600);
  delay(500);
  Serial.print("Nome file: ");
  Serial.println(__FILE__);
  delay(1500);  


  Serial.println();
  Serial.println( F ("|******   SISTEMA RESETTATO  ******|"));
  Serial.println( F (" Sketch build xxxxx "));
  Serial.print( F ("Versione "));Serial.println(Versione);
  Serial.println();

  lcd.begin();          // Inizializza il display
  lcd.backlight();     // Accende la retroilluminazione

  WelcomeDisplay();
  
  buttonState = digitalRead(buttonPin);
  if (buttonState == LOW){
    BuzzerIsDisabled=true;
    Serial.println("Buzzer disabilitato");
    SirenaFranceseVeloce();
  }

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


}

//--******************************************************************************************

void(* resetFunc) (void) = 0; //declare reset function @ address 0

//-------------------------------------------



//*******************************************************
/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) {
    // get the new byte:
    char inChar = (char)Serial.read();
    // add it to the inputString:
    inputString += inChar;
    // if the incoming character is a newline, set a flag so the main loop can
    // do something about it:
    if (inChar == '\n') {
      stringComplete = true;
    }
  }
}

//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
void TypeMenuList(void){
    Serial.println();
    Serial.println( F ("|*******************************************"));
    Serial.println( F ("|             ʘ‿ʘ   Menù   (◡_◡)          "));
      Serial.print( F ("|  Ver. "));Serial.println(Versione);
    Serial.println( F ("|*******************************************"));
    Serial.println( F ("  0 Reset"));
    Serial.println( F ("  1 Test display 2x16 12385678 ABCDEFGH.."));
    Serial.println( F ("  2 Test buzzer"));
    Serial.println( F ("  3 Test Funzione Trimmer Soglia"));
    Serial.println( F ("  4 .Usa Funzione Trimmer Soglia"));
    Serial.println( F ("  5 Test Funzione Sensore Corrente"));
    Serial.println( F ("  6 .Usa Funzione Sensore Corrente"));
    Serial.println( F ("  7 Test Funzione Voltmetro"));
    Serial.println( F ("  8 .Usa Funzione Voltmetro"));
    Serial.println( F ("  9 Test button"));
    Serial.println( F (" 10 .Usa Funzione Soglia Superata"));
      Serial.print( F (" 11 Debug "));
      if(DebugOn){
        Serial.println("OFF = ADC reali");
        DebugOn = false;
      } else {
        Serial.println("ON = ADC su potenz.");
        DebugOn = true;
      }
    Serial.println( F (" 12 "));
    Serial.println( F (" 13 Disabilita Buzzer"));
    Serial.println( F (" 14 Abilita Buzzer"));
    Serial.println( F (" 15 Ripete ciclo LOOP fino a Q"));
    Serial.println( F (" 16 "));
      Serial.print( F (" 17 Debug is "));
      if(debug){
        Serial.println(" ON");
      }else{
        Serial.println(" OFF");
      }
   Serial.println( F (" 18 Legge tutti gli ADC"));
   Serial.println( F (" 19 Legge solo ADC usati"));
   Serial.println( F (" 20 Test tutti gli output di nano, da D2 a d13"));
   Serial.println( F (" 21 Manda SMS ai due numeri"));
   Serial.println( F (" 99 Torna al loop senza reset"));
  
}
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
//xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx

void Menu() {
  //resta nel menu fino a che premi 0
  while (!ExitFromMenu) {
    TypeMenuList();

    //svuoto il buffer
    while (Serial.available()) {
      Cestino = Serial.read();
    }

    Serial.println();
    // qui ciclo attesa e Flash fino a che non c'è un carattere sulla seriale
    while (!Serial.available()) {
      
      //--------------------------------------
      unsigned long currentMillis = millis();
      if (currentMillis - previousMillis >= interval/4) {
        previousMillis = currentMillis;   
      }     
    }

    // arrivato un carattere
    int CmdMenu = Serial.parseInt();
    Serial.print( F ("ricevuto CmdMenu "));Serial.println(CmdMenu);

    switch (CmdMenu){
      //-------------------------------------------------
      case 0:

        Serial.println( F ("reset tra 1 sec"));
        delay(1000);               // wait for a second
        resetFunc();  //call reset

        delay(100);
        Serial.println( F ("Il reset non ha funzionato"));
      break;
      //-------------------------------------------------
      case 1:
        //    TEST DISPLAY 2X16");
        Azione1();
      break;
      //-------------------------------------------------
      case 2:
        //    Serial.println("  2 Abilita Watchdog32 senza retriggerare");
        Azione2();
      break;
      //-------------------------------------------------
      case 3:
        //Serial.println( F ("  3 Abilita, triggera 5 secondi e poi ferma il trigger, deve resettare"));
        Azione3();
      break;
      //-------------------------------------------------
      case 4:
        //    Serial.println( F ("  4 Abilita, triggera con tempi crescenti, deve resettare"));      
        Azione4();
      break;
      //-------------------------------------------------
      case 5:
        //    5 Aziona relè bulbo per 3 secondi      
        Azione5();
      break;
      //-------------------------------------------------
      case 6:
        //    6 Aziona relè lettura bulbo per 3 secondi"     
        Azione6();
      break;
      //-------------------------------------------------
      case 7:
        //     7 Accende led giallo per 3 secondi"));  
        Azione7();
      break;
      //-------------------------------------------------
      case 8:
        //    8 Accende led verde per 3 secondi"));      
        Azione8();
      break;
      //-------------------------------------------------
      case 9:
        //    9 Suona il cicalino"));      
        Azione9();
      break;
      //-------------------------------------------------
      case 10:
        //     10 Legge il trimmer"));     
        Azione10();
      break;
      //-------------------------------------------------
      case 11:
        //     11 Legge il bulbo"));
        Azione11();
      break;
      //-------------------------------------------------
      case 12:
        //     11 Legge il bulbo"));
        Azione12();
      break;
      //-------------------------------------------------
      case 13:
        //     11 Legge il bulbo"));
        Azione13();
      break;
      //-------------------------------------------------
      case 14:
        //     11 Legge il bulbo"));
        Azione14();
      break;
      //-------------------------------------------------
      case 15:
        //     11 Legge il bulbo"));
        Azione15();
      break;
      //-------------------------------------------------
      case 16:
        //     11 Legge il bulbo"));
        Azione16();
      break;
      //-------------------------------------------------
      case 17:
        //     swap debug;
        Azione17();
      break;
      //-------------------------------------------------
      case 18:
        //     swap debug;
        Azione18();
      break;
      //-------------------------------------------------
      case 19:
        //     swap debug;
        Azione19();
      break;
      //-------------------------------------------------
      case 20:
        //     swap debug;
        Azione20();
      break;
      //-------------------------------------------------
      //-------------------------------------------------
      case 21:
        //     swap debug;
        Azione21();
      break;
      //-------------------------------------------------


      case 99:
        Azione99();
      break;
      //-------------------------------------------------

      default:
        //Serial.println( F ("Unrecognized command, try again!"));
        Serial.println( F ("Comando errato! Riprova"));
    } //Serial.available
  } // ExitFromMenu
} // Menu


//*******************************************************
void WelcomeDisplay(){
    lcd.setCursor(0, 0); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print("Welcome ");

    lcd.setCursor(0, 1); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print(Versione);
}

//************************

void Azione1(void){
    
    
    lcd.setCursor(0, 0); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print("s: ");
    lcd.print(counter++);

    lcd.setCursor(8, 0); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print("A: ");
    lcd.print(counter++);

    lcd.setCursor(0, 1); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print("V: ");
    lcd.print(counter++);

    lcd.setCursor(8, 1); // Posiziona il cursore sulla seconda riga, prima colonna
    lcd.print("E: ");
    lcd.print(counter++);

    delay(500);

}
void Azione2(void){
  //test buzzer
  SirenaFrancese();

  

}
//-----------------------------------------------
void Azione3(void){
  //prova la funzione di conversione del Potenziometro in valore di soglia di corrente
   for (int i =0 ; i<1023;i++){
    i=i+16;
    ProcessaPotenziometroSoglia(i);
  } 
}

void Azione4(void){
  //usa la funzione  ProcessaPotenziometroSoglia

  //legge l'AD connesso al ptenziometro
  int valoreAnalogico = analogRead(AdcPotenziometro);

  // lo converte
  ProcessaPotenziometroSoglia(valoreAnalogico);

  // lo manda al display LCD riga 1 posizione 12
  lcd.setCursor(8, 0); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(" SmA");

  lcd.setCursor(12, 0); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(StrSogliaMilliampere);
}

//-----------------------------------------------
void Azione5(void){
  //prova la funzione di conversione del sensore di corrente in valore di corrente
  //Nano ha un ADC da 10 bit, range 0-1023
  for (int i =0 ; i<1023;i++){
    i=i+16;
    adcValue = i;
    ProcessaSensoreCorrente(adcValue);
  } 
}

//----------------------------------------------------

void Azione6(void){
/*
  #define AdcSensorCurrent A2
  #define AdcVoltageBattery A3
  #define AdcPotenziometro A7
*/
  if(debug){
    //in debug legge l'AD connesso al potenziometro
    adcValue = analogRead(AdcPotenziometro);
  } else {
    //run time legge l'AD connesso al sensore corrente
    adcValue = analogRead(AdcSensorCurrent);    
  }

  ProcessaSensoreCorrente(adcValue);

  // lo manda al display LCD riga 1 posizione 12
  lcd.setCursor(0, 0); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print("ImA=");

  lcd.setCursor(3, 0); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(strCurrent);


}




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

/*
  #define AdcSensorCurrent A2
  #define AdcVoltageBattery A3
  #define AdcPotenziometro A7
*/

  if(debug){
    //in debug legge l'AD connesso al potenziometro
    adcValue = analogRead(AdcPotenziometro);
  } else {
    //run time legge l'AD connesso al sensore tensione
    adcValue = analogRead(AdcVoltageBattery);    
  }

  ProcessaAdcTensione(adcValue);

  // lo manda al display LCD riga 1 posizione 12
  lcd.setCursor(0, 1); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print("Volt");

  lcd.setCursor(4, 1); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(strVoltage);


}
//-----------------------------------------------


void Azione9(void){

  //test button
  buttonState = digitalRead(buttonPin);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (buttonState == LOW) {
    Serial.println("Button premuto");
    ContaSecondiCorrenteElevata=0;
  } else {
    // turn LED off:
    Serial.println("Button rilasciato");
  }
  

}
//-----------------------------------------------

void Azione10(void){
  //legge l'AD connesso al ptenziometro
  
  ControllaSogliaSuperata();
  if(ContaSecondiCorrenteElevata>9999){
    ContaSecondiCorrenteElevata=9999; //satura a 9999
  }
  sprintf(strContaSecondiCorrenteElevata, "%04u", ContaSecondiCorrenteElevata);

  // lo manda al display LCD riga 1 posizione 12
  lcd.setCursor(8, 1); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(" Err");

  lcd.setCursor(12, 1); // Posiziona il cursore sulla prima riga, colonna 1
  lcd.print(strContaSecondiCorrenteElevata);


}
//-----------------------------------------------

void Azione11(void){

}
void Azione12(void){

}
void Azione13(void){
  BuzzerIsDisabled = true;
}
void Azione14(void){
  BuzzerIsDisabled = false;
}
//*****************************************************
void Azione15(void){
 // ripeti Funzione Fin che Non Ricevi Quit
  while (true) {
    // Esegui la funzione desiderata
    delay(250);
    Azione4(); //soglia potenziometro
    Azione6(); //legge corrente sensore
    Azione8(); //legge tensione batteria
    Azione9(); //legge button, se premuto azzera ContaSecondiCorrenteElevata


    //ogni secondo
    unsigned long currentMillis = millis();

    // Ogni secondo
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      ContaSecondi++;
      Azione10(); //compara corrente sensore con soglia potenziometro
      Serial.print("ContaSecondi= ");Serial.print(ContaSecondi);
      Serial.print(" - ContaSecondiCorrenteElevata= ");Serial.print(ContaSecondiCorrenteElevata);
      
    } 

    // Controlla se ci sono dati disponibili sulla seriale
    if (Serial.available() > 0) {
      // Leggi il carattere dalla seriale
      char carattereRicevuto = Serial.read();

      // Controlla se il carattere ricevuto è 'q' o 'Q'
      if (carattereRicevuto == 'q' || carattereRicevuto == 'Q') {
        // Esci dal ciclo
        break;
      }
    }
  }
}
//*********************************************************************

void Azione16(void){
  
}

void Azione17(){
  //swap debug
  if (debug){
    debug=false;
  }else {
    debug=true;
  }
}
//--------------------------------------------------------------------------
//-
//--------------------------------------------------------------------------

void Azione18(){

  uint16_t adcValues[8];
  ClearSerialBuffer();
  while (true) {
    // Controlla se c'è un dato disponibile sulla seriale
    if (Serial.available() > 0) {
      char receivedChar = Serial.read(); // Legge il carattere inviato via seriale
      if (receivedChar == 'Q' || receivedChar == 'q') {
        Serial.println("Interruzione della lettura.");
        break; // Esce dal ciclo e dalla funzione
      }
    }

    // Leggi i valori dai pin analogici A0-A7
    for (int i = 0; i < 8; i++) {
      adcValues[i] = analogRead(i); // Leggi il valore ADC dal pin i
    }
    
    Serial.println();
    Serial.println("  A0    A1    A2    A3    A4    A5    A6    A7");

    // Invia i valori in esadecimale su una riga al terminale seriale
    for (int i = 0; i < 8; i++) {
      Serial.print("0x"); // Aggiungi uno spazio tra i valori
      if (adcValues[i] < 0x100) {
        Serial.print("0"); // Aggiungi uno zero iniziale se il valore è inferiore a 0x10
      }
      if (adcValues[i] < 0x10) {
        Serial.print("0"); // Aggiungi uno zero iniziale se il valore è inferiore a 0x10
      }
      Serial.print(adcValues[i], HEX); // Stampa il valore in esadecimale
      if (i < 7) {
        Serial.print(" "); // Aggiungi uno spazio tra i valori
      }
    }
    Serial.println(); // Vai a capo dopo aver inviato tutti i valori

    delay(250); // Aspetta 500 ms prima della prossima lettura

  }
}

//--------------------------------------------------------------------------

void Azione19(void){
  /*
  #define AdcSensorCurrent A2
  #define AdcVoltageBattery A3
  #define AdcPotenziometro A7
  */

  while (true) {
    //ogni secondo
    unsigned long currentMillis = millis();

    // Ogni secondo
    //if (currentMillis - previousMillis >= interval) {
    if (currentMillis - previousMillis >= 333) {   //ogni 333msec
      previousMillis = currentMillis;
      
      //esegue le letture
      int potValue = analogRead(AdcPotenziometro);
      int currentValue = analogRead(AdcSensorCurrent); 
      int voltageValue = analogRead(AdcVoltageBattery);

      // Stampa i valori in formato decimale ed esadecimale
      Serial.print("Pot A7 = ");
      Serial.print(potValue);
      Serial.print(" 0x");
      Serial.print(potValue, HEX);  // Stampa il valore in esadecimale

      Serial.print(" | Current A2 = ");
      Serial.print(currentValue);
      Serial.print(" 0x");
      Serial.print(currentValue, HEX);

      Serial.print(" | Voltage A3 = ");
      Serial.print(voltageValue);
      Serial.print(" 0x");
      Serial.println(voltageValue, HEX);  // Usa println per andare a capo

    }

    // Controlla se ci sono dati disponibili sulla seriale
    if (Serial.available() > 0) {
      // Leggi il carattere dalla seriale
      char carattereRicevuto = Serial.read();

      // Controlla se il carattere ricevuto è 'q' o 'Q'
      if (carattereRicevuto == 'q' || carattereRicevuto == 'Q') {
        // Esci dal ciclo
        break;
      }
    }
  }











}

//--------------------------------------------------------------------------

void Azione20(){
  const int startPin = 2;  // Primo pin digitale disponibile
  const int endPin = 13;  // Ultimo pin digitale disponibile

  Serial.println("Testa gli outut a rotazione (solo quelli configuati)... q termina");

  ClearSerialBuffer();
  while (true) {
    //Serial.print(".");
    // Controlla se c'è un dato disponibile sulla seriale
    if (Serial.available() > 0) {
      char receivedChar = Serial.read(); // Legge il carattere inviato via seriale
      if (receivedChar == 'Q' || receivedChar == 'q') {
        Serial.println("Interruzione della lettura.");
        break; // Esce dal ciclo e dalla funzione
      }


    }
    Serial.println();
    for (int pin = startPin; pin <= endPin; pin++) {
        digitalWrite(pin, HIGH); // Accendi il pin corrente
        delay(250);        // Aspetta per 200 ms
        digitalWrite(pin, LOW);  // Spegni il pin corrente
        Serial.print(" D");Serial.print(pin);
    }
  }
}



//--------------------------------------------------------------------------

void Azione21(){
  sendSMS(PrefissoInt + NumeroUtente1, TestoSMS);
  sendSMS(PrefissoInt + NumeroUtente2, TestoSMS);

}

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------



void Azione99(){
  Serial.println( F ("E' stata scelta l'azione n. 99"));
  delay(2000);
  Serial.println( F ("torno al loop'"));
  delay(2000);
  ExitFromMenu = true;

  
}
//--------------------------------------------------------------------------
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
    } else if (userIndex == '2' && newNumber.length() == 10) {
      NumeroUtente2 = newNumber;
      writeStringToEEPROM(addr2, newNumber);
      Serial.println("NumeroUtente2 aggiornato: " + NumeroUtente2);
    } else {
      Serial.println("Formato SMS non valido");
    }
  }
}

void sendSMS(String numero, String messaggio) {
  // Verifica validità numero (senza prefisso)
  String numeroLocale = numero;
  if (numero.startsWith(PrefissoInt)) {
    numeroLocale = numero.substring(PrefissoInt.length());
  }

  if (numeroLocale.length() != 10 || numeroLocale[0] != PrimaCifraValida) {
    Serial.println("Numero non valido, SMS non inviato: " + numero);
    return;
  }

  Serial.println("Invio SMS a: " + numero);
  sim800.println("AT+CMGS=\"" + numero + "\"");
  delay(1000);
  sim800.print(messaggio);
  sim800.write(26); // Ctrl+Z per inviare
  Serial.println("SMS inviato.");
  delay(5000);
}



