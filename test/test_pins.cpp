#include <Arduino.h>
#define RED_LED_PIN 32
#define BLUE_LED_PIN 33
#define YELLOW_LED_PIN 12
#define GREEN_LED_PIN 2
#define BUZZER_PIN 15
void setup()
{
  delay(2000); 
  Serial.begin(115200);  
  Serial2.begin(115200, SERIAL_8N1, 13, 14);  
  Serial.println("Iniciando comunicación serie con el GM75...");
  pinMode(RED_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(YELLOW_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
}

void loop() {   
  if (Serial2.available()) {  
    String data = Serial2.readStringUntil('\n');  
    Serial.println("Datos recibidos del GM75: " + data); 
  } else {  
    Serial.println("Esperando datos ----------");  
  }  
  digitalWrite(RED_LED_PIN, HIGH);
  delay(500);
  digitalWrite(RED_LED_PIN, LOW);
  delay(500);

  // Prueba del LED azul
  digitalWrite(BLUE_LED_PIN, HIGH);
  delay(500);
  digitalWrite(BLUE_LED_PIN, LOW);
  delay(500);

  // Prueba del LED amarillo
  digitalWrite(YELLOW_LED_PIN, HIGH);
  delay(500);
  digitalWrite(YELLOW_LED_PIN, LOW);
  delay(500);

  // Prueba del LED verde
  digitalWrite(GREEN_LED_PIN, HIGH);
  delay(500);
  digitalWrite(GREEN_LED_PIN, LOW);
  delay(500);

  // Prueba del buzzer
  digitalWrite(BUZZER_PIN, HIGH);
  delay(200);
  digitalWrite(BUZZER_PIN, LOW);
  delay(500);
  delay(1000);  
}