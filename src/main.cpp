#include <Arduino.h>

// Función para verificar si un carácter es mayúscula
bool isUpper(char c) {
  return (c >= 'A' && c <= 'Z');
}

// Función para verificar si un carácter es alfabético
bool isAlpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Función para decodificar el cifrado César
String caesarCipherDecode(String text, int shift) {
  String result = "";
  shift = shift % 26;

  for (int i = 0; i < text.length(); i++) {
    char c = text[i];
    if (isAlpha(c)) {
      char base = isUpper(c) ? 'A' : 'a';
      c = (c - base - shift + 26) % 26 + base; // Ajusta el desplazamiento para la decodificación
    }
    result += c;
  }

  result.replace('$', ' ');
  return result;
}

void setup() {
  Serial.begin(115200); // Inicializa el puerto serial USB (UART0) para el monitor serial
  Serial2.begin(9600, SERIAL_8N1, 13, 14); // Inicializa Serial2 en RX (GPIO 13) y TX (GPIO 14)
  delay(1000); // Espera para inicialización
  Serial.println("Iniciando...");
}

void loop() {
  // Verifica si hay datos disponibles en el segundo puerto UART (Serial2)
  if (Serial2.available()) {
    String datos = Serial2.readStringUntil('\n'); // Lee del UART2
    datos = caesarCipherDecode(datos, 3); // Decodifica los datos
    Serial.println("Datos: " + datos); // Imprime los datos decodificados en el monitor serial
  }

  delay(1000); // Espera entre lecturas
}