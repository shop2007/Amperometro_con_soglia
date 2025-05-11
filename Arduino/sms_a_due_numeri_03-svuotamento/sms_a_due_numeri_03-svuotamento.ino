







void setup() {
  Serial.begin(115200);
  Serial.println("Menu");
  Serial.println("1 manda sms prova utente 1");
  Serial.println("2 manda sms prova utente 2");
  Serial.println("3 forza utente 1 333210xxxx");
  Serial.println("4 forza utente 2 329703yyyy");
  Serial.println("5 Legge i numeri 1 e 2");

}

void loop() {


  // Comandi seriali per invio SMS
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == '1') {
      
    } else if (cmd == '2') {
      
    } else if (cmd == '3') {
    } else if (cmd == '4') {
      processSMS("199272329703yyyy");//inserisce come numero 1 329703yyyy
    } else if (cmd == '5') {

    }
  }
}
