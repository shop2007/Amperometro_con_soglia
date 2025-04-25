const int ValoreAdcCorrentePositiva1ampere = 0x300f;  // Valore Adc Corrente Positiva 1 ampere
const int ValoreAdcCorrente0ampere = 0x1fff;          // Valore Adc Corrente 0 Ampere
const int ValoreAdcCorrenteNegativa1ampere = 0x0fe0;  // Valore Adc Corrente Negativa 1 ampere
#define AdcSensorCurrent A7;                           // Adc connesso al sensore di corrente

void setup() {
  Serial.begin(9600); // Inizializza la comunicazione seriale
}

void loop() {
  int valoreAdc = analogRead(AdcSensorCurrent); // Legge il valore dall'ADC

  // Calcola lo scostamento dal valore di zero in bit
  int scostamento = valoreAdc - ValoreAdcCorrente0ampere;

  // Calcola il valore di corrente in milliampere
  float correnteMilliampere = 0;
  if (scostamento > 0) {
    // Corrente positiva
    correnteMilliampere = (scostamento * 1000.0) / (ValoreAdcCorrentePositiva1ampere - ValoreAdcCorrente0ampere);
  } else {
    // Corrente negativa
    correnteMilliampere = (scostamento * 1000.0) / (ValoreAdcCorrente0ampere - ValoreAdcCorrenteNegativa1ampere);
  }

  // Stampa i risultati sul monitor seriale
  Serial.print("Valore ADC (hex): 0x");
  Serial.println(valoreAdc, HEX); // Stampa il valore ADC in esadecimale

  Serial.print("Scostamento dal valore zero (decimale): ");
  Serial.println(scostamento); // Stampa lo scostamento in decimale

  Serial.print("Corrente (mA): ");
  Serial.println(correnteMilliampere); // Stampa la corrente in milliampere

  delay(1000); // Attende un secondo prima di ripetere la lettura
}