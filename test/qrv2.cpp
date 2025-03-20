#include <Arduino.h>

void setup()
{
  delay(2000); 
  Serial.begin(9600);  
  Serial2.begin(9600, SERIAL_8N1, 13, 14);  
  Serial.println("Iniciando comunicación serie con el GM75...");
}

void loop() {   
  if (Serial2.available()) {  
    String data = Serial2.readStringUntil('\n');  
    Serial.println("Datos recibidos del GM75: " + data); 
  } else {  
    Serial.println("Esperando datos ----------");  
  }  

  delay(1000);  
}