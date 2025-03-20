#include "Utils.hpp"

Utils::Utils(const char* ssid, const char* password) {
    this->ssid = ssid;
    this->password = password;
    this->ntpServer = "pool.ntp.org";
}

void Utils::setLeds() {
    pinMode(REDLED, OUTPUT);
    pinMode(BLUELED, OUTPUT);
    pinMode(GREENLED, OUTPUT);
}

void Utils::redLedBlink() {
    digitalWrite(REDLED, HIGH);
    delay(100);
    digitalWrite(REDLED, LOW);
    delay(100);
}

void Utils::blueLedBlink() {
    digitalWrite(BLUELED, HIGH);
    delay(100);
    digitalWrite(BLUELED, LOW);
    delay(100);
}

void Utils::greenLedBlink() {
    digitalWrite(GREENLED, HIGH);
    delay(100);
    digitalWrite(GREENLED, LOW);
    delay(100);
}

void Utils::ScanWifi() {
    greenLedBlink();
    blueLedBlink();
    redLedBlink();
}

void Utils::offLeds() {
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);
    digitalWrite(GREENLED, LOW);
}

void Utils::onRedLed() {
    digitalWrite(REDLED, HIGH);
}

void Utils::onBlueLed() {
    digitalWrite(BLUELED, HIGH);
}

void Utils::onGreenLed() {
    digitalWrite(GREENLED, HIGH);
}

void Utils::lightsAfternoon(){
    digitalWrite(BLUELED, LOW);   
    digitalWrite(GREENLED, HIGH); 
    digitalWrite(REDLED, HIGH);

}

void Utils::lightsTomorrow(){
    digitalWrite(BLUELED, HIGH);  
    digitalWrite(GREENLED, HIGH); 
    digitalWrite(REDLED, LOW);
}

bool Utils::connecToWifi() {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < max_retries) 
    {
        ScanWifi();
        retries++;
        ScanWifi();
    }

    if(WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        digitalWrite(GREENLED, HIGH);
        return true;
    } else {
        Serial.println("Could not connect to WiFi");
        digitalWrite(REDLED, HIGH);
        return false;
    }
}

bool Utils::isUpper(char c) {
    return c >= 'A' && c <= 'Z';
}

bool Utils::isAlpha(char c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool Utils::isDigit(char c) {
    return c >= '0' && c <= '9';
}

String Utils::cesarCipherDecode(String text, int shift) 
{
    String result = "";
    shift = shift % 26;
  
    // Lista de caracteres especiales que no serán modificados
    String specialChars = "áÁéÉíÍóÓúÚñÑ";
  
    for (int i = 0; i < text.length(); i++) 
    {
      char c = text[i];
      
      // Verifica si el carácter es especial, y si lo es, lo agrega sin cambios
      if (specialChars.indexOf(c) >= 0) {
        result += c;
      } 
      // Aplica el descifrado César solo en letras alfabéticas
      else if (isAlpha(c)) {
        char base = isUpper(c) ? 'A' : 'a';
        c = (c - base - shift + 26) % 26 + base;
        result += c;
      }
      // Aplica el descifrado a dígitos numéricos
      else if (isDigit(c)) {
        c = (c - '0' - shift + 10) % 10 + '0';
        result += c;
      } 
      // Agrega otros caracteres sin cambios
      else {
        result += c;
      }
    }
  
    result.replace('$', ' ');
  
    // Buscar el antepenúltimo grupo de caracteres (DNI)
    int lastSpaceIndex = result.lastIndexOf(" ");
    int dniEndIndex = result.lastIndexOf(" ", lastSpaceIndex - 1);
    int dniStartIndex = result.lastIndexOf(" ", dniEndIndex - 1) + 1;
  
    String dni = result.substring(dniStartIndex, dniEndIndex);
  
    return dni;
  }

