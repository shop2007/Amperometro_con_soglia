float SogliaCorrenteMilliampere = 0; // Valore di soglia impostata su un trimmer
unsigned int SogliaInt; // SogliaCorrenteMilliampere convertito in intero
String SogliaString; // SogliaInt convertito in stringa

float ValoreSensoreCorrenteMilliampere = 0; // Valore letto su un sensore di corrente
int SensorInt; // ValoreSensoreCorrenteMilliampere convertito in intero
String SensorString; // SensorInt convertito in stringa

unsigned long MilliSecondiSogliaSuperata = 0;
unsigned long SecondiSogliaSuperata = 0;
String StringSecondiSogliaSuperata; // SecondiSogliaSuperata convertito in stringa

unsigned long previousMillis = 0;
const long interval = 1000; // Intervallo di un secondo

void setup() {
  Serial.begin(9600); // Inizializza la comunicazione seriale
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    ValoreSensoreCorrenteMilliampere = ReadSensoreCorrenteMilliampere(); // Legge il valore del sensore
    SogliaCorrenteMilliampere = ReadSogliaCorrenteMilliampere(); // Legge il valore della soglia

    SensorInt = static_cast<int>(ValoreSensoreCorrenteMilliampere);
    SogliaInt = static_cast<unsigned int>(SogliaCorrenteMilliampere);

    // Converte i valori in stringhe
    SensorString = FormatStringWithSign(SensorInt, 5);
    SogliaString = FormatString(SogliaInt, 4);
    StringSecondiSogliaSuperata = FormatString(SecondiSogliaSuperata, 5);

    // Verifica se la corrente supera la soglia
    if (abs(SensorInt) > SogliaInt) {
      MilliSecondiSogliaSuperata += interval;
      SecondiSogliaSuperata = MilliSecondiSogliaSuperata / 1000;
    }

    // Stampa i risultati sul monitor seriale
    Serial.print("Soglia: ");
    Serial.println(SogliaString);

    Serial.print("Sensore: ");
    Serial.println(SensorString);

    Serial.print("Secondi Soglia Superata: ");
    Serial.println(StringSecondiSogliaSuperata);
  }
}

float ReadSensoreCorrenteMilliampere() {
  // Simulazione della lettura del sensore
  // Dovresti sostituire questo con il codice reale per leggere la corrente
  return random(-2000, 2000);
}

float ReadSogliaCorrenteMilliampere() {
  // Simulazione della lettura della soglia
  // Dovresti sostituire questo con il codice reale per leggere il trimmer
  return random(0, 2000);
}

String FormatString(int value, int width) {
  String formatted = String(value);
  while (formatted.length() < width) {
    formatted = " " + formatted;
  }
  return formatted;
}

String FormatStringWithSign(int value, int width) {
  String formatted = (value >= 0) ? "+" + String(value) : String(value);
  while (formatted.length() < width) {
    formatted = " " + formatted;
  }
  return formatted;
}




