#include <Arduino.h>

void setup()
{
  delay(2000); 
  Serial.begin(115200);  
  Serial.println("Iniciando comunicación serie con el GM75...");
}

void loop() {   
  if (Serial.available()) {  
    String data = Serial.readStringUntil('\n');  
    Serial.println("Datos recibidos del GM75: " + data); 
  } else {  
    Serial.println("Esperando datos ----------");  
  }  

  delay(1000);  
}