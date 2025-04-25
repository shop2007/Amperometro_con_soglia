
/*L'ADC di arduino nano ha il fondo scala 1023 bit = 5v ( 0x03ff )
Da un sensore di corrente LEM ho rilevato i seguenti valori:
- con corrente 0 mA ha una uscita di 2.5v
- con una corrente di -350mA ha una uscita di 2.07v
- con una corrente di +450mA ha una uscita di 3.06v

Scrivi una funzione e sketch che fa una interpolazione lineare per le pendenze indicate

input funzione: 
 - unsigned int AdcValue ; //bit dell'ADC ( range da 0 a 1023 )
output funzione: 
 - unsigned int AdcValue ; Valore dell'ADC in hex (quello in input)
 - int DeltaBit ;//Scostamento in bit dallo zero 
 - float CurrentSensor ; // valore di corrente in mA interpolato
 - String StrCurrentSensor ; // valore di StrCurrentSensor, 4 digit + segno,  sostituendo agli zeri non significativi uno space e al segno + uno space  
 - stampa su monitor seriale dei 4 valori in output

*/
const int sensorPin = A0; // Pin analogico a cui è collegato il sensore

void setup() {
  Serial.begin(9600); // Inizializza la comunicazione seriale
}

void loop() {
  unsigned int adcValue = analogRead(sensorPin); // Legge il valore dall'ADC
  int deltaBit;
  float currentSensor;
  String strCurrentSensor;

  calculateCurrent(adcValue, deltaBit, currentSensor, strCurrentSensor);

  // Stampa i risultati sul monitor seriale
  Serial.print("ADC Value (Hex): 0x");
  Serial.print(adcValue, HEX);
  Serial.print(" | Offset from Zero: ");
  Serial.print(deltaBit);
  Serial.print(" | Current: ");
  Serial.print(currentSensor, 1); // Stampa la corrente con una cifra decimale
  Serial.print(" mA | Formatted Current: ");
  Serial.println(strCurrentSensor);

  delay(1000); // Attende un secondo prima di leggere nuovamente
}

void calculateCurrent(unsigned int adcValue, int &deltaBit, float &currentSensor, String &strCurrentSensor) {
  // Calcola la tensione corrispondente al valore ADC
  float voltage = adcValue * (5.0 / 1023.0);

  // Calcola lo scostamento in bit dallo zero (2.5V)
  deltaBit = adcValue - 512; // 512 è il valore ADC corrispondente a 2.5V

  // Interpolazione lineare tra i punti dati:
  // (2.07V, -350mA) e (2.5V, 0mA)
  // (2.5V, 0mA) e (3.06V, +450mA)

  if (voltage <= 2.5) {
    // Calcola la corrente per valori di tensione <= 2.5V
    currentSensor = mapFloat(voltage, 2.07, 2.5, -350.0, 0.0);
  } else {
    // Calcola la corrente per valori di tensione > 2.5V
    currentSensor = mapFloat(voltage, 2.5, 3.06, 0.0, 450.0);
  }

  // Formatta la stringa del valore di corrente
  strCurrentSensor = formatCurrentString(currentSensor);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

String formatCurrentString(float current) {
  String formatted = String(current, 0); // Converte il valore in stringa senza cifre decimali

  // Aggiunge spazi per sostituire gli zeri non significativi
  while (formatted.length() < 4) {
    formatted = " " + formatted;
  }

  // Gestisce il segno positivo
  if (current >= 0) {
    formatted = " " + formatted;
  }

  return formatted;
}
