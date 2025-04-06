#include <Arduino.h>
#include "Utils.hpp"
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESP32-AP-Config.hpp>

Utils *ut = new Utils();

ESP32AP::Config apConfig;  


char manualAttendanceType[10] = "entrance";
char dniData[20];
#define BUTTON_PIN 2

bool manualOverride = false;
bool attendanceToggle = true;
const char *prevAttendanceType = "";
std::vector<String> processedDnis;
const int MAX_PROCESSED_SIZE = 450;






const char api_url[] PROGMEM = "https://colecheck.com/api/register_assistance";
const char auth_token[] PROGMEM = "1ae9465337bf4747def0bdca0a1eb3dac096fd52";

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


void setup()
{
  delay(2000);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 13, 14);
  ut->setLeds();

  if (ut->connecToWifi("yerson","char5524"))
  {
    Serial.println("Conectado a la red WiFi");
  }
  //attendanceQueue = xQueueCreate(100, sizeof(AttendanceData));
  //xTaskCreate(SendDataToServer, "SendDataToServer", 10000, NULL, 1, NULL);
}


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

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    ut->offLeds();
    if (!ut->connecToWifi("yerson","char5524"))
    {
      ut->onRedLed();
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

    const char *attendanceType;

    if (manualOverride)
    {
      attendanceType = attendanceToggle ? "entrance" : "exit";
    }
    else
    {
      attendanceType = (timeinfo.tm_hour < 12) ? "entrance" : "exit";
    }

    if (manualOverride && attendanceType != prevAttendanceType)
    {
      Serial.println("Modo manual activado");
      processedDnis.clear();
      prevAttendanceType = attendanceType;
    }

    if (!manualOverride)
    {
      if (timeinfo.tm_hour < 12)
      {
        ut->lightsTomorrow();
      }
      else
      {
        ut->lightsAfternoon();
      }
    }

    if (digitalRead(BUTTON_PIN) == HIGH)
    {
      attendanceToggle = !attendanceToggle;
      manualOverride = true;

      if (attendanceToggle)
      {
        Serial.println("Manual override: entrance");
        ut->lightsTomorrow();
      }
      else
      {
        Serial.println("Manual override: exit");
        ut->lightsAfternoon();
      }
      processedDnis.clear();
      delay(500);
    }

    if (Serial2.available())
    {
      String dni = Serial2.readStringUntil('\n');
      dni = ut->cesarCipherDecode(dni, 3);

      if (dni.length() > 0)
      {
        if (attendanceType == "entrance")
        {
          ut->blueLedBlink();
          ut->onBlueLed();
        }
        else
        {
          ut->redLedBlink();
          ut->onRedLed();
        }
        addToQueueIfUnique(dni, attendanceType);
      }
    }
  }
}