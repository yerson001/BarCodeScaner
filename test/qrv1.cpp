#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <bits/stdc++.h>
#include <HTTPClient.h>
#include <vector>
#include <algorithm>
#include <WebServer.h>
#include <Preferences.h>
//#include "utils.hpp"


const char* apSSID = "QRSCANNER";  // SSID del AP
const char* apPassword = "12345678";  // Contraseña del AP

WebServer server;
Preferences preferences;


#define REDLED 32
#define BLUELED 33
#define GREENLED 12
#define BUTTON_PIN 2



std::vector<String> processedDnis; 
const int MAX_PROCESSED_SIZE = 450;

const char ntpServer[] PROGMEM = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;
const int daylightOffset_sec = 0;
const int max_retries = 30;

bool isUpper(char c) {
  return (c >= 'A' && c <= 'Z');
}

bool isAlpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

bool isDigit(char c) {
  return (c >= '0' && c <= '9');
}

String caesarCipherDecode(String text, int shift) {
  String result = "";
  shift = shift % 26;

  // Lista de caracteres especiales que no serán modificados
  String specialChars = "áÁéÉíÍóÓúÚñÑ";

  for (int i = 0; i < text.length(); i++) {
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


// Constantes movidas a la memoria flash

const char ssid[] PROGMEM = "yerson";
const char password[] PROGMEM = "char5524";
const char api_url[] PROGMEM = "https://colecheck.com/api/register_assistance";
const char auth_token[] PROGMEM = "013c329a22103485187ca39546f567eeb012515a";

void RedLed()
{
  digitalWrite(REDLED, HIGH);
  delay(100);
  digitalWrite(REDLED, LOW);
  delay(100);
}

void BlueLed()
{
  digitalWrite(BLUELED, HIGH);
  delay(100);
  digitalWrite(BLUELED, LOW);
  delay(100);
}

void GreenLed()
{
  digitalWrite(GREENLED, HIGH);
  delay(100);
  digitalWrite(GREENLED, LOW);
  delay(100);
}

void scanWifi()
{
  GreenLed();
  BlueLed();
  RedLed();
}

//**************** TASK **************** */
char manualAttendanceType[10] = "entrance";
char dniData[20];

struct AttendanceData
{
  char dni[20];
  char type[10];
};

QueueHandle_t attendanceQueue;



void SendDataToServer(void *pvParameters)
{
  while (1)
  {
    if (uxQueueMessagesWaiting(attendanceQueue) > 0)
    {
      AttendanceData dataToSend;
      xQueueReceive(attendanceQueue, &dataToSend, portMAX_DELAY);
      delay(2000);
      Serial.print("SEND: ");
      Serial.print(dataToSend.dni);
      Serial.print(" TYPE: ");
      Serial.println(dataToSend.type);
      RedLed();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}




/*
void SendDataToServer(void *pvParameters) {
  AttendanceData dataToSend;

  while (1) {
    if (uxQueueMessagesWaiting(attendanceQueue) > 0) {
      xQueueReceive(attendanceQueue, &dataToSend, portMAX_DELAY);

      DynamicJsonDocument jsonDoc(128); // JSON document size
      dataToSend.dni[strcspn(dataToSend.dni, "+")] = 0;

      jsonDoc["dni"] = dataToSend.dni;
      jsonDoc["type_assistance"] = dataToSend.type;

      String jsonString;
      serializeJson(jsonDoc, jsonString);

      HTTPClient http;
      http.begin(api_url);
      http.addHeader("Content-Type", "application/json");
      http.addHeader("Authorization", String("token ") + auth_token);

      int httpResponseCode = http.POST(jsonString);

      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        Serial.print("Response: ");
        Serial.println(response);
      } else {
        Serial.println("Error in sending POST request");
      }

      http.end();

      Serial.print("SEND: ");
      Serial.print(dataToSend.dni);
      Serial.print(" TYPE: ");
      Serial.println(dataToSend.type);
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
*/


/**
 * 
 * ACCESPOINT READY
 * 
 */



/**
 * 
 * END ACCESPOINT READY
 * 
 */





void addToQueueIfUnique(String dni, String attendanceType) {
  // Verificar si el DNI ya ha sido procesado
  if (std::find(processedDnis.begin(), processedDnis.end(), dni) != processedDnis.end()) {
    return;  // No agregar si el DNI ya está en el historial
  }

  // Agregar el DNI a la cola y actualizar la lista de DNIs procesados
  AttendanceData data;
  strncpy(data.dni, dni.c_str(), sizeof(data.dni) - 1);
  data.dni[sizeof(data.dni) - 1] = '\0';
  strncpy(data.type, attendanceType.c_str(), sizeof(data.type) - 1);
  data.type[sizeof(data.type) - 1] = '\0';
  
  // Enviar a la cola de asistencia
  xQueueSend(attendanceQueue, &data, portMAX_DELAY);

  // Agregar el DNI a la lista de procesados
  processedDnis.push_back(dni);
  if (processedDnis.size() > MAX_PROCESSED_SIZE) {
    processedDnis.erase(processedDnis.begin());  // Elimina el DNI más antiguo si el tamaño supera el límite
  }
}



//*************** WIFI ***************** */

bool connectToWiFi()
{
  WiFi.begin(ssid, password);
  Serial.println("Conectando a WiFi...");

  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED && retry_count < max_retries)
  {
    scanWifi();
    retry_count++;
    scanWifi();
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("Conectado a la red WiFi");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    digitalWrite(GREENLED, HIGH);
    return true;
  }
  else
  {
    Serial.println("No se pudo conectar a la red WiFi");
    digitalWrite(REDLED, HIGH);
    return false;
  }
}



// Declaración de variables
bool attendanceToggle = true; // True para "entrance", False para "exit"
bool manualOverride = false;
int lastButtonState = LOW;
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;

const char* prevAttendanceType = "";  

//sysutils wf("Yerson");

void setup()
{
  Serial.begin(11500);
  Serial2.begin(9600, SERIAL_8N1, 13, 14);

  pinMode(REDLED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(GREENLED, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  if (connectToWiFi())
  {
    Serial.println("Conectado a la red WiFi");
  }

  attendanceQueue = xQueueCreate(100, sizeof(AttendanceData));
  xTaskCreate(SendDataToServer, "SendDataToServer", 10000, NULL, 1, NULL);
}


void loop()
{
  // Verificar conexión WiFi y LEDs
  if (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(GREENLED, LOW);
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);

    if (!connectToWiFi())
    {
      digitalWrite(REDLED, HIGH);
    }
  }
  else
  {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
      Serial.println("Failed to obtain time");
      delay(1000);
      return;
    }

    // Determinar el tipo de asistencia automáticamente basado en la hora del día
    const char *attendanceType;
    if (manualOverride)
    {
      attendanceType = attendanceToggle ? "entrance" : "exit";
    }
    else
    {
      attendanceType = (timeinfo.tm_hour < 12) ? "entrance" : "exit";
    }

    // Limpia el vector si el tipo de asistencia cambia automáticamente
    if (!manualOverride && attendanceType != prevAttendanceType)
    {
      Serial.println("Cambio automático en tipo de asistencia, limpiando vector");
      processedDnis.clear();  
      prevAttendanceType = attendanceType;  
    }

    // Control de LEDs según la hora del día, solo si no está en modo manual
    if (!manualOverride)
    {
      if (timeinfo.tm_hour < 12) // Mañana
      {
        digitalWrite(BLUELED, HIGH);  // Encender LED azul
        digitalWrite(GREENLED, HIGH); // Encender LED verde
        digitalWrite(REDLED, LOW);    // Apagar LED rojo
      }
      else // Tarde
      {
        digitalWrite(BLUELED, LOW);   // Apagar LED azul
        digitalWrite(GREENLED, HIGH); // Encender LED verde
        digitalWrite(REDLED, HIGH);   // Encender LED rojo
      }
    }

    // Verificación del botón para alternar entre "entrance" y "exit"
    if (digitalRead(BUTTON_PIN) == HIGH)
    {
      attendanceToggle = !attendanceToggle; // Cambia entre "entrance" y "exit"
      manualOverride = true; // Activa el modo manual

      // Cambia el LED según el nuevo estado
      if (attendanceToggle)
      {
        Serial.println("Manual override: entrance");
        digitalWrite(BLUELED, HIGH); // Encender LED azul
        digitalWrite(REDLED, LOW);   // Apagar LED rojo
      }
      else
      {
        Serial.println("Manual override: exit");
        digitalWrite(BLUELED, LOW);  // Apagar LED azul
        digitalWrite(REDLED, HIGH);  // Encender LED rojo
      }

      processedDnis.clear();  // Limpia el vector al alternar manualmente
      delay(500); // Anti-rebote básico
    }

    // Recepción de datos del puerto Serial2
    if (Serial2.available())
    {
      String dni = Serial2.readStringUntil('\n');
      dni = caesarCipherDecode(dni, 3);

      if (dni.length() > 0)
      {
        if (attendanceType == "entrance")
        {
          BlueLed();
          digitalWrite(BLUELED, HIGH);
        }
        else
        {
          RedLed();
          digitalWrite(REDLED, HIGH);
        }

        addToQueueIfUnique(dni, attendanceType);
      }
    }
  }
}
