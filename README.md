#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <ESP32Time.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <WebServer.h>
#include <Preferences.h> 
#include "LEDBuzzerManager.h"

// WIFI CONFIG
//const char* wifiSSID = "colecheck";
//const char* wifiPassword = "coleadmin";
// ACCESPOINT
const char* apSSID = "ColeConfig";
const char* apPassword = "12345678";
bool configalt = false;
Preferences preferences;

WebServer server(80);


// HTML para la página web
const char* htmlPage = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ColeAdmin</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="font-awesome/css/all.min.css">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      background-color: #f7f7f7;
      color: #333;
    }
    h1 {
      color: #0056b3;
    }
    .btn {
      background-color: #0056b3;
      color: white;
      border: none;
      padding: 10px 20px;
      font-size: 16px;
      cursor: pointer;
      border-radius: 5px;
      margin: 10px 0;
      width: 80%;
      max-width: 300px;
    }
    .btn.success {
      background-color: #28a745;
    }
    .btn:hover {
      background-color: #003d80;
    }
    input[type="text"], input[type="password"] {
      padding: 10px;
      width: 80%;
      max-width: 300px;
      margin: 10px auto; /* Alineación centrada */
      border: 1px solid #ccc;
      border-radius: 5px;
      font-size: 16px;
    }
    .icon {
      margin-right: 8px;
    }
    .card {
      background-color: #fff;
      border: 1px solid #ccc;
      border-radius: 10px;
      box-shadow: 0 4px 8px 0 rgba(0, 0, 0, 0.2);
      margin: 20px;
      padding: 20px;
    }
    .hidden {
      display: none;
    }
    #status, #response, #currentConfig {
      margin: 20px 0;
    }
    .avatar-icon {
      font-size: 40px;
      color: #0056b3;
      margin-bottom: 10px;
    }
    .response-box {
      margin-top: 20px;
      text-align: left;
    }
    .response-content {
      padding: 10px;
      background-color: #f0f0f0;
      border-radius: 5px;
    }
    .success-alert {
      background-color: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
      padding: 10px;
      border-radius: 5px;
      margin-top: 10px;
    }
  </style>
  <script>
    function toggleSection(id) {
      var sections = ['configSection', 'dataSection'];
      sections.forEach(function(sectionId) {
        if (sectionId === id) {
          document.getElementById(sectionId).style.display = 'block';
        } else {
          document.getElementById(sectionId).style.display = 'none';
        }
      });
    }

    function syncESP32() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/sync", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4) {
          if (xhr.status == 200) {
            document.getElementById("status").innerText = "Sincronización exitosa";
            showNotification("Sincronización exitosa", "success");
            document.getElementById("syncButton").classList.add("success");
            document.getElementById("syncButton").innerHTML = '<i class="fas fa-sync icon"></i>Sincronizado';
            setTimeout(function() {
              document.getElementById("syncButton").classList.remove("success");
              document.getElementById("syncButton").innerHTML = '<i class="fas fa-sync icon"></i>Sincronizar';
            }, 3000); // Restaurar el botón después de 3 segundos
          } else {
            document.getElementById("status").innerText = "Sincronización fallida";
            showNotification("Sincronización fallida", "error");
          }
          document.getElementById("dataForm").style.display = "block";
        }
      };
      xhr.send();
    }

    function sendData(event) {
      event.preventDefault();
      var data = document.getElementById("dataInput").value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/send?data=" + data, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          document.getElementById("responseContent").innerText = xhr.responseText;
          document.getElementById("response").style.display = "block";
          showNotification("Respuesta enviada", "success");
          document.getElementById("dataInput").value = ""; // Limpiar el campo de DNI después del envío
        }
      };
      xhr.send();
    }

    function saveConfig(event) {
      event.preventDefault();
      var ssid = document.getElementById("ssid").value;
      var password = document.getElementById("password").value;
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/save?ssid=" + ssid + "&password=" + password, true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          document.getElementById("currentConfig").innerText = xhr.responseText;
          showNotification("Configuración guardada", "success");
        }
      };
      xhr.send();
    }

    function testDuplicate() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/testDuplicate", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var response = xhr.responseText;
          // Mostrar la respuesta del ESP32
          document.getElementById("duplicateResponse").innerText = response;
          document.getElementById("duplicateResponse").style.display = "block";
          showNotification("Revisión de duplicado completada", "success");
        }
      };
      xhr.send();
    }

    function showNotification(message, type) {
      var notification = document.createElement('div');
      notification.textContent = message;
      notification.className = 'success-alert';
      document.body.appendChild(notification);
      setTimeout(function() {
        notification.remove();
      }, 3000); // Remover la notificación después de 3 segundos
    }
  </script>
</head>
<body>
  <h1>ColeAdmin</h1>
  <button id="syncButton" class="btn" onclick="syncESP32()"><i class="fas fa-sync icon"></i>Sincronizar</button>
  <button class="btn" onclick="toggleSection('configSection')"><i class="fas fa-wifi icon"></i>Configurar WiFi</button>
  <button class="btn" onclick="toggleSection('dataSection')"><i class="fas fa-clone"></i>Duplicado</button>

  <div id="configSection" class="hidden card">
    <h2>Configuración de WiFi</h2>
    <form id="wifiForm" onsubmit="saveConfig(event)">
      <label for="ssid">WiFi:</label><br>
      <input type="text" id="ssid" name="ssid" required><br><br>
      <label for="password">Contraseña:</label><br>
      <input type="password" id="password" name="password" required><br><br>
      <input type="submit" value="Guardar">
    </form>
    <div class="card">
      <p id="currentConfig"><strong>WiFi:</strong> {current_ssid}<br><strong>Contraseña:</strong> {current_password}</p>
    </div>
  </div>

  <div id="dataSection" class="hidden card">
    <h2>Sincronizar Datos</h2>
    <div class="card">
      <p id="status">Esperando sincronización...</p>
      <form id="dataForm" onsubmit="sendData(event)">
        <div style="display: flex; align-items: center; flex-direction: column;">
          <i class="fas fa-user avatar-icon"></i>
          <label for="dataInput">DNI:</label><br>
          <input type="text" id="dataInput" name="dataInput" style="width: calc(100% - 20px); padding: 10px; max-width: 300px;"><br><br>
          <input type="submit" value="Enviar" class="btn" style="width: 100%;">
        </div>
      </form>
      <div id="response" class="response-box hidden">
        <p><strong>Respuesta:</strong></p>
        <div id="responseContent" class="response-content"></div>
      </div>
    </div>
  </div>

  <div id="duplicateSection" class="hidden card">
    <h2>Revisar Duplicado</h2>
    <button class="btn" onclick="testDuplicate()">Probar</button>
    <div class="card">
      <p id="duplicateResponse" class="hidden"><strong>Respuesta ESP32:</strong></p>
    </div>
  </div>
</body>
</html>
)rawliteral";

// Función para configurar el ESP32 como Access Point
void setupAccessPoint() {
  bool apStarted = WiFi.softAP(apSSID, apPassword);
  if (apStarted) {
    Serial.println("Access Point iniciado correctamente");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    Serial.print("MODO ACCESPOINT");
  } else {
    Serial.println("Error al iniciar el Access Point");
  }
}



// Función de manejo de datos recibidos
void handleSendData() {
  String data = server.arg("data");
  Serial.println("Datos recibidos: " + data);
  server.send(200, "text/plain", "Datos recibidos: " + data);
}

// Función de manejo de sincronización
void handleSync() {
  server.send(200, "text/plain", "OK");
}

// Función para configurar las rutas del servidor web
void setupServerRoutes() {
  server.on("/", HTTP_GET, []() {
    // Leer la configuración actual de WiFi
    String currentSSID = preferences.getString("wifiSSID", "colecheck");
    String currentPassword = preferences.getString("wifiPassword", "coleadmin");
    String page = String(htmlPage);
    page.replace("{current_ssid}", currentSSID);
    page.replace("{current_password}", currentPassword);
    server.send(200, "text/html", page);
  });

  server.on("/save", HTTP_GET, []() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");
    preferences.putString("wifiSSID", ssid);
    preferences.putString("wifiPassword", password);
    String response = "Configuración guardada. SSID actual: " + ssid + "<br>Contraseña actual: " + password;
    server.send(200, "text/plain", response);
  });

  server.on("/send", HTTP_GET, handleSendData);
  server.on("/sync", HTTP_GET, handleSync);

  server.begin();
  Serial.println("Servidor web iniciado");
}


// API CONFIG
const char* api_url = "https://colecheck.com/api/register_assistance";
const char* auth_token = "fc1ae7ce0f06127361d47cffae3df7b3ea1cb7fc";

// OLED CONFIG
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // OLED reset pin -1 for Adafruit_SSD1306

// NFC CONFIG
#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

// NTP CONFIG
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600;  // GMT-5 for Lima, Peru
const int daylightOffset_sec = 0;      // No daylight saving time in Lima

ESP32Time rtc;



// BUTTON PINS
int buttonPinEntrada = 26;
int buttonPinSalida = 27;
bool manualOverride = false;
char manualAttendanceType[8] = "entrada";

// NFC DATA
char dniData[64] = ""; // DNI data storage

// Struct for attendance data
struct AttendanceData {
  char dni[16];
  char type[8];
};

QueueHandle_t attendanceQueue; // Cola para almacenar temporalmente los DNIs y tipo de asistencia

bool isAlphanumeric(char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '=');
}

void ReadAndDecodeNFC(char* text, size_t maxLen) {
  byte buffer[18];
  byte size = sizeof(buffer);
  size_t textIndex = 0;

  for (byte page = 4; page < 16; page++) {
    if (mfrc522.MIFARE_Read(page, buffer, &size) == MFRC522::STATUS_OK) {
      for (byte i = 0; i < 4 && textIndex < maxLen - 1; i++) {
        if (isAlphanumeric(buffer[i])) {
          text[textIndex++] = (char)buffer[i];
        }
      }
    }
  } 
  text[textIndex] = '\0'; // Null-terminate the string
}



LEDBuzzerManager ledBuzzerManager;

void setupButtons() {
  pinMode(buttonPinEntrada, INPUT);
  pinMode(buttonPinSalida, INPUT);
}

void setupOLED() {
  Wire.begin(21, 33);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop indefinitely
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 10);
  display.println("Connecting to WiFi...");
  display.display();
}

bool connectToWiFi(String wifiSSID,String wifiPassword) {
  WiFi.begin(wifiSSID, wifiPassword);
  Serial.println("Connecting to WiFi...");

  int retry_count = 0;
  const int max_retries = 30;

  while (WiFi.status() != WL_CONNECTED && retry_count < max_retries) {
    ledBuzzerManager.GreenLed();
    ledBuzzerManager.BlueLed();
    ledBuzzerManager.RedLed();
    retry_count++;
  }

  ledBuzzerManager.GreenLed();
  ledBuzzerManager.playSound();
  ledBuzzerManager.playSound();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    return true;
  } else {
    Serial.println("Failed to connect to WiFi");
    return false;
  }
}

void SendDataToServer(void *pvParameters) {
    while (1) {
        if (uxQueueMessagesWaiting(attendanceQueue) > 0) {
            AttendanceData dataToSend;
            xQueueReceive(attendanceQueue, &dataToSend, portMAX_DELAY);
            // Simular un retraso de 2 segundos
            delay(2000);
            // Simular el envío de datos al servidor
            Serial.print("SEND: ");
            Serial.print(dataToSend.dni);
            Serial.print(" TYPE: ");
            Serial.println(dataToSend.type);
 
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // Esperar 100ms antes de verificar nuevamente
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

      ledBuzzerManager.RedLed();
    }
    vTaskDelay(pdMS_TO_TICKS(100));
  }
}
*/
void setup() {
  Serial.begin(115200);
  btStop();
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  preferences.begin("config", false); // Iniciar preferencias


  ledBuzzerManager.setup();
  setupButtons();
  setupOLED();
  String ssid = preferences.getString("wifiSSID", "colecheck");
  String password = preferences.getString("wifiPassword", "coleadmin");

  pinMode(buttonPinEntrada, INPUT);  // Asegúrate de que el pin sea configurado como entrada
  pinMode(buttonPinSalida, INPUT);   // Asegúrate de que el pin sea configurado como entrada
  delay(2000);

  //if (digitalRead(buttonPinEntrada) == HIGH)
  if (true)
   {  // Verifica el estado del botón de entrada
    // Si el botón está presionado al encender, crear Access Point
    setupAccessPoint();
    setupServerRoutes(); // Configurar rutas del servidor solo si se crea Access Point
  configalt = true;
  } else {
    if (!connectToWiFi(ssid.c_str(), password.c_str())) {
      Serial.println("Failed to connect to WiFi");
      while (1);  // Puedes agregar una acción de manejo de error aquí
    } else {
      Serial.println("Connected to WiFi");
      setupServerRoutes();  // Configurar rutas del servidor si se conecta a WiFi
    }
  }

  attendanceQueue = xQueueCreate(10, sizeof(AttendanceData));
  xTaskCreate(SendDataToServer, "SendDataToServer", 10000, NULL, 1, NULL);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED || !configalt) {
     delay(configalt == false ? 100 : 1200000);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, LOW);
    ledBuzzerManager.playSound();
    ledBuzzerManager.playSound();
    
    String ssid = preferences.getString("wifiSSID", "colecheck");
    String password = preferences.getString("wifiPassword", "coleadmin");
    if (!connectToWiFi(ssid,password)) {
      digitalWrite(RED_LED_PIN, HIGH);
    }
  } else {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      delay(1000);
      return;
    }

    if (digitalRead(buttonPinEntrada) == HIGH) {
      manualOverride = true;
      strcpy(manualAttendanceType, "entrada");
      Serial.println("Manual override: entrada");
      delay(500);
    }
    if (digitalRead(buttonPinSalida) == HIGH) {
      manualOverride = true;
      strcpy(manualAttendanceType, "salida");
      Serial.println("Manual override: salida");
      delay(500);
    }

    const char* attendanceType;
    if (manualOverride) {
      attendanceType = manualAttendanceType;
    } else {
      attendanceType = (timeinfo.tm_hour < 12) ? "entrance" : "exit";
    }

    const char* attendanceTypeDisplay = (strcmp(attendanceType, "entrada") == 0) ? "Entrada" : "Salida";

    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("- %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    display.setTextSize(2);
    display.setCursor(0, 30);
    display.printf("- %s", attendanceTypeDisplay);

    display.display();

    if (mfrc522.PICC_IsNewCardPresent()) {
      if (mfrc522.PICC_ReadCardSerial()) {
        char dni[64] = "";
        ReadAndDecodeNFC(dni, sizeof(dni));
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();

        if (strlen(dni) > 0) {
          AttendanceData data;
          strcpy(data.dni, dni);
          strcpy(data.type, attendanceType);
          xQueueSend(attendanceQueue, &data, portMAX_DELAY);
        }
      }
    }
  }
  server.handleClient();
}
