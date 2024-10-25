#Barcode


#define LED1 32   // GPIO para el primer LED
#define LED2 33   // GPIO para el segundo LED
#define LED3 34   // GPIO para el tercer LED

void setup() {
  // Configurar los pines de los LEDs como salida
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
}

void loop() {
  // Encender y apagar los LEDs en secuencia
  digitalWrite(LED1, HIGH);  // Enciende el LED 1
  delay(500);                // Espera 500ms
  digitalWrite(LED1, LOW);   // Apaga el LED 1

  digitalWrite(LED2, HIGH);  // Enciende el LED 2
  delay(500);                // Espera 500ms
  digitalWrite(LED2, LOW);   // Apaga el LED 2

  digitalWrite(LED3, HIGH);  // Enciende el LED 3
  delay(500);                // Espera 500ms
  digitalWrite(LED3, LOW);   // Apaga el LED 3
}
****
