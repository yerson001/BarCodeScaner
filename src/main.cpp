#include <Arduino.h>
#include "Utils.hpp"
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>

Utils *ut = new Utils();

void setup() {
    // Señalización de inicio
    ut->onBuzzer();
    delay(2000);
    ut->onBuzzer();
    
    // Configuración serial
    Serial.begin(115200);
    Serial2.begin(115200, SERIAL_8N1, 13, 14);
    
    Serial.println("=================================");
    Serial.println("   BARCODE SCANNER SYSTEM v2.0   ");
    Serial.println("=================================");

    // Inicializar hardware y estado
    ut->setLeds();
    initializeSystemState(); 
    
    // Configurar tiempo
    configTime(-5 * 3600, 0, "pool.ntp.org", "time.nist.gov");

    // Crear tasks con verificación
    Serial.println("[INIT] Creando tasks...");
    
    if (xTaskCreate(wifiReconnectTask, "WifiReconnectTask", 4096, ut, 2, NULL) != pdPASS) {
        Serial.println("ERROR: No se pudo crear WiFi Reconnect Task");
    }
    
    if (xTaskCreate(scanTask, "ScanTask", 4096, ut, 3, NULL) != pdPASS) {
        Serial.println("ERROR: No se pudo crear Scan Task");
    }
    
    if (xTaskCreate(attendanceTypeTask, "AttendanceTypeTask", 4096, ut, 1, NULL) != pdPASS) {
        Serial.println("ERROR: No se pudo crear Attendance Type Task");
    }
    
    if (xTaskCreate(statusReportTask, "StatusReportTask", 8192, ut, 1, NULL) != pdPASS) {
        Serial.println("RROR: No se pudo crear Status Report Task");
    }
    
    if (xTaskCreate(Utils::sendDataToServer, "SendDataToServer", 8192, ut, 1, NULL) != pdPASS) {
        Serial.println("ERROR: No se pudo crear Send Data To Server Task");
    }

    Serial.printf("[INIT] Heap libre: %d bytes\n", ESP.getFreeHeap());
    Serial.println("******** SISTEMA INICIADO ********");
}

void loop() {
    // Sistema basado en FreeRTOS
    yield(); // Permite que otros tasks se ejecuten
}