#include <Arduino.h>

#define BUZZER_PIN 15 // Pin D15 donde está conectado el buzzer

void setup() {
    pinMode(BUZZER_PIN, OUTPUT); // Configurar el pin del buzzer como salida
}

void loop() {
    digitalWrite(BUZZER_PIN, HIGH); // Encender el buzzer
    delay(200);                     // Mantenerlo encendido por 200 ms
    digitalWrite(BUZZER_PIN, LOW);  // Apagar el buzzer
    delay(4000);                    // Esperar 3 segundos antes de repetir
}
