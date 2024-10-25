#include <WiFi.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <MFRC522.h>
#include <ESP32Time.h>
#include <ArduinoJson.h>
#include <BluetoothSerial.h>

// Constantes movidas a la memoria flash
const char key[] PROGMEM = "colecheck";
const char ssid[] PROGMEM = "colecheck";
const char password[] PROGMEM = "coleadmin";
const char api_url[] PROGMEM = "https://colecheck.com/api/register_assistance";
const char auth_token[] PROGMEM = "fc1ae7ce0f06127361d47cffae3df7b3ea1cb7fc";

const int max_retries = 30;

Adafruit_SSD1306 display(128, 64, &Wire, -1);
#define SCREEN_ADDRESS 0x3C

#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);

const char ntpServer[] PROGMEM = "pool.ntp.org";
const long gmtOffset_sec = -5 * 3600; // GMT-5 for Lima, Peru
const int daylightOffset_sec = 0;     // No daylight saving time in Lima

ESP32Time rtc;

#define GREEN_LED_PIN 12
#define BLUE_LED_PIN 13
#define RED_LED_PIN 32

const int buzzerPin = 14; // D14

int buttonPinEntrada = 26;
int buttonPinSalida = 27;
bool manualOverride = false;
char manualAttendanceType[10] = "entrada";

char dniData[20];

struct AttendanceData {
    char dni[20];
    char type[10];
};

QueueHandle_t attendanceQueue;

enum Mode { SCANNER, WRITER };
Mode currentMode = SCANNER;

BluetoothSerial* pSerialBT = nullptr; // Puntero a BluetoothSerial

void checkSound() {
    tone(buzzerPin, 1000);
    delay(200);
    noTone(buzzerPin);
}

bool isAlphanumericOrEqual(char c) {
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '=');
}

void setupLeds() {
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(BLUE_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);
}

void RedLed() {
    digitalWrite(RED_LED_PIN, HIGH);
    delay(100);
    digitalWrite(RED_LED_PIN, LOW);
    delay(100);
}

void BlueLed() {
    digitalWrite(BLUE_LED_PIN, HIGH);
    delay(100);
    digitalWrite(BLUE_LED_PIN, LOW);
    delay(100);
}

void GreenLed() {
    digitalWrite(GREEN_LED_PIN, HIGH);
    delay(100);
    digitalWrite(GREEN_LED_PIN, LOW);
    delay(100);
}

void scanWifi() {
    GreenLed();
    BlueLed();
    RedLed();
}

char* readAndDecode() {
    byte buffer[18];
    byte size = sizeof(buffer);
    static char text[16];
    memset(text, 0, sizeof(text));

    for (byte page = 4; page < 16; page++) {
        if (mfrc522.MIFARE_Read(page, buffer, &size) == MFRC522::STATUS_OK) {
            for (byte i = 0; i < 4; i++) {
                if (isAlphanumericOrEqual(buffer[i])) {
                    strncat(text, (char*) &buffer[i], 1);
                }
            }
        } else {
            Serial.print("Error reading page ");
            Serial.println(page);
        }
    }

    Serial.print("DECODE: ");
    Serial.println(text);

    return text;
}

  void write(String data) {
    int dataSize = data.length();
    Serial.print("Data size: ");
    Serial.println(dataSize);

    int numPages = (dataSize + 3) / 4;
    Serial.print("Number of pages: ");
    Serial.println(numPages);

    for (int pageIndex = 0; pageIndex < numPages; pageIndex++) {
      byte buffer[16] = {0};
      int startIdx = pageIndex * 4;
      int charsToCopy = min(4, dataSize - startIdx);
      String chunk = data.substring(startIdx, startIdx + charsToCopy);
      chunk.getBytes(buffer, sizeof(buffer));

      byte page = 4 + pageIndex;
      if (mfrc522.MIFARE_Write(page, buffer, 16) != MFRC522::STATUS_OK) {
        Serial.print("Error writing to page ");
        Serial.println(page);
      } else {
        Serial.print("Successfully wrote to page ");
        Serial.println(page);
      }
    }
  }


bool connectToWiFi() {
    WiFi.begin(ssid, password);
    Serial.println("Conectando a WiFi...");
   
    int retry_count = 0;
    while (WiFi.status() != WL_CONNECTED && retry_count < max_retries) {
        scanWifi();
        retry_count++;
        scanWifi();
    }
    
    digitalWrite(GREEN_LED_PIN, HIGH);
    checkSound();
    checkSound();

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Conectado a la red WiFi");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        return true;
    } else {
        Serial.println("No se pudo conectar a la red WiFi");
        return false;
    }
}

void SendDataToServer(void *pvParameters) {
    while (1) {
        if (uxQueueMessagesWaiting(attendanceQueue) > 0) {
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

void updateDisplay(const char* message) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(message);
    display.display();
}

unsigned long startTime = 0;
const unsigned long activeTime = 300000;

void setup() {
    Serial.begin(9600);
    SPI.begin();
    mfrc522.PCD_Init();
    delay(4);
    mfrc522.PCD_DumpVersionToSerial();

    pinMode(buzzerPin, OUTPUT);
    setupLeds();

    pinMode(buttonPinEntrada, INPUT);
    pinMode(buttonPinSalida, INPUT);
    
    Wire.begin(21, 33);
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 10);
    display.println("Conectando a WiFi...");
    display.display();

    if (connectToWiFi()) {
        Serial.println("Conectado a la red WiFi");
    }

    attendanceQueue = xQueueCreate(3, sizeof(AttendanceData));
    xTaskCreate(SendDataToServer, "SendDataToServer", 10000, NULL, 1, NULL);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) {
        digitalWrite(GREEN_LED_PIN, LOW);
        digitalWrite(RED_LED_PIN, LOW);
        checkSound();
        checkSound();

        if (!connectToWiFi()) {
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
            attendanceType = (timeinfo.tm_hour < 12) ? "entrada" : "salida";
        }
        const char* attendanceTypeDisplay = strcmp(attendanceType, "entrada") == 0 ? "Entrada" : "Salida";

        display.clearDisplay();
        display.setTextSize(2);
        display.setCursor(0, 0);
        display.printf("- %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        display.setTextSize(2);
        display.setCursor(0, 30);
        display.printf("- %s", attendanceTypeDisplay);
        display.display();

          // Control del modo mediante Serial
        if (Serial.available() > 0) {
              char command = Serial.read();
              if (command == 'w') {
                  currentMode = WRITER;
                  if (!pSerialBT) {
                      pSerialBT = new BluetoothSerial();  // Inicializa BluetoothSerial si no está inicializado
                      pSerialBT->begin("ESP32_BT");
                  }
                  // Desactivar RTC y liberar memoria de la cola
                  //rtc.stop(); // Detener el RTC
                  vQueueDelete(attendanceQueue); // Eliminar la cola de asistencia
                  attendanceQueue = NULL; // Establecer el puntero de la cola a NULL
                  Serial.println("Modo WRITER activado");
              } else if (command == 's') {
                  currentMode = SCANNER;
                  if (pSerialBT) {
                      delete pSerialBT;  // Liberar la memoria de BluetoothSerial si está inicializado
                      pSerialBT = nullptr;
                  }
                  // Reactivar RTC y recrear la cola
                  //rtc.start(); // Iniciar el RTC
                  attendanceQueue = xQueueCreate(3, sizeof(AttendanceData)); // Recrear la cola de asistencia
                  Serial.println("Modo SCANNER activado");
              }
          }

        if (currentMode == SCANNER) {
            if (mfrc522.PICC_IsNewCardPresent()) {
                if (mfrc522.PICC_ReadCardSerial()) {
                    char* dni = readAndDecode();
                    mfrc522.PICC_HaltA();
                    mfrc522.PCD_StopCrypto1();
                    if (strlen(dni) > 0) {
                        AttendanceData data;
                        strncpy(data.dni, dni, sizeof(data.dni));
                        strncpy(data.type, attendanceType, sizeof(data.type));
                        xQueueSend(attendanceQueue, &data, portMAX_DELAY);
                    }
                }
            }
        } else if (currentMode == WRITER) {
            // Aquí iría el código para el modo WRITER que utiliza BluetoothSerial
            if (pSerialBT) {
                // Ejemplo: enviar datos a través de Bluetooth
                pSerialBT->println("Datos para enviar por Bluetooth");
            }
        }
    }
}










