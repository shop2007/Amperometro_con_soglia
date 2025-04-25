/*
ARDUINO
float floatVoltage; //range da 0 a 25.999
char strVoltage[5]; // 4 caratteri + 1 per il terminatore di stringa

vorrei una funzione che approssimi floatVoltage , lo riduca ad una sola cifra dopo la virgola e poi venga 
scritta una sringa in strVoltage allineata a destra con eliminazionedegli zeri non significativi

esempio floatVoltage= 1,345 -> strVoltage=" 1.3"
esempio floatVoltage=22,65 ->  strVoltage="22.7"
*/

void convertVoltageToString(float floatVoltage, char* strVoltage) {
    // Approssima il valore a una cifra decimale
    float roundedVoltage = round(floatVoltage * 10.0) / 10.0;

    // Converte il valore approssimato in una stringa temporanea
    char tempStr[6]; // Buffer temporaneo leggermente più grande per la conversione
    dtostrf(roundedVoltage, 4, 1, tempStr); // Usa 4 caratteri di larghezza, 1 cifra decimale

    // Rimuove gli zeri non significativi
    int len = strlen(tempStr);
    if (tempStr[len - 1] == '0') {
        tempStr[len - 2] = '\0'; // Rimuove il punto decimale e lo zero
    }

    // Copia la stringa nel buffer finale allineata a destra
    snprintf(strVoltage, 5, "%4s", tempStr);
}

void setup() {
    Serial.begin(9600);

    float floatVoltage = 1.345;
    convertVoltageToString(floatVoltage, strVoltage);
    Serial.println(strVoltage); // Output: " 1.3"

    floatVoltage = 22.65;
    convertVoltageToString(floatVoltage, strVoltage);
    Serial.println(strVoltage); // Output: "22.7"
}

void loop() {
    // Il loop non è necessario per questo esempio
}