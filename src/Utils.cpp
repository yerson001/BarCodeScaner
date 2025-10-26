#include "Utils.hpp"

/**
 * @file Utils.cpp
 * @brief Implementación de la clase Utils y funciones del sistema de scanner
 * @author Colecheck Team
 * @version 2.0
 * @date 2025
 */

// =========================================
// ========== ESTRUCTURA DE DATOS ==========
// =========================================

/**
 * @brief Estructura para datos de asistencia en la cola
 * @details Almacena información de cada escaneo para procesamiento asíncrono
 */
struct AttendanceData
{
    char dni[20];  // DNI del usuario escaneado
    char type[10]; // Tipo de asistencia ("entrance" o "exit")
    // unsigned long timestamp; // Timestamp del escaneo (millis())
    char ID_dispositivo[20] = "001"; // ID del dispositivo scanner
};

// ======================================================
// ========== DEFINICIÓN DE VARIABLES GLOBALES ==========
// ======================================================

/**
 * @brief Variables de estado de asistencia
 * @details Controlan el comportamiento del sistema de asistencia
 */

const String WIFI_SSID = "Pixel_7";
const String WIFI_PASSWORD = "12244668";
const char *DEVICE_ID = "001";

char manualAttendanceType[10] = "entrance"; ///< Tipo actual de asistencia
char dniData[20] = "";                      ///< DNI en procesamiento
bool manualOverride = false;                ///< Modo manual activo
bool attendanceToggle = true;               ///< Estado del toggle
const char *prevAttendanceType = "";        ///< Tipo anterior de asistencia

/**
 * @brief Variables de estado del dispositivo
 * @details Monitorean el funcionamiento y estadísticas del scanner
 */
int scanCount = 0;         // Contador total de escaneos
bool deviceWorking = true; // Estado de funcionamiento

/**
 * @brief Configuración de servidor y API
 * @details URLs y tokens para comunicación con servicios externos
 */
const char api_url[] = "https://colecheck.com/api/register_assistance";
const char auth_token[] = "token 2e8c37e4a9023b3b20c952d85addc06b1d2ae559"; // prueba

// ====================================================
// ========== FUNCIONES DE GESTIÓN DE ESTADO ==========
// ====================================================

/**
 * @brief Inicializa el estado del sistema con valores por defecto
 * @details Configura todas las variables globales con sus valores iniciales
 */
void initializeSystemState()
{
    strcpy(manualAttendanceType, "entrance");
    strcpy(dniData, "");
    manualOverride = false;
    attendanceToggle = true;
    prevAttendanceType = "";
    scanCount = 0;
    deviceWorking = true;

    Serial.println("[STATE] Sistema inicializado con valores por defecto");
}

int incrementScanCount()
{
    scanCount++;
    return scanCount;
}

void resetScanCount()
{
    scanCount = 0;
}

void setAttendanceType(const char *type)
{
    prevAttendanceType = manualAttendanceType;
    strncpy(manualAttendanceType, type, sizeof(manualAttendanceType) - 1);
    manualAttendanceType[sizeof(manualAttendanceType) - 1] = '\0';
    Serial.printf("[STATE] Tipo de asistencia cambiado a: %s\n", type);
}

void setDniData(const char *dni)
{
    strncpy(dniData, dni, sizeof(dniData) - 1);
    dniData[sizeof(dniData) - 1] = '\0';
}

void setDeviceWorking(bool working)
{
    deviceWorking = working;
}

// ======================================================
// ========== IMPLEMENTACIÓN DE LA CLASE UTILS ==========
// ======================================================

Utils::Utils() : retries(0)
{
    // Configurar servidor NTP
    ntpServer = "pool.ntp.org";

    attendanceQueue = xQueueCreate(1000, sizeof(AttendanceData));

    if (attendanceQueue == NULL)
    {
        Serial.println("[ERROR] No se pudo crear la cola de asistencia");
    }
}

// ========== FUNCIONES DE TIEMPO ==========

time_t Utils::hour()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("[TIME] Error: No se pudo obtener la hora local");
        return 0;
    }
    return mktime(&timeinfo);
}
// ========== FUNCIONES DE CONTROL DE HARDWARE ==========

/**
 * @brief Configura todos los pines de LEDs y buzzer como OUTPUT
 * @details Inicializa los pines definidos en la configuración para control de hardware
 */
void Utils::setLeds()
{
    pinMode(REDLED, OUTPUT);
    pinMode(BLUELED, OUTPUT);
    pinMode(YELLOWLED, OUTPUT);
    pinMode(GREENLED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Inicializar todos los LEDs apagados
    offLeds();
}

void Utils::offLeds()
{
    digitalWrite(REDLED, LOW);
    digitalWrite(BLUELED, LOW);
    digitalWrite(GREENLED, LOW);
    digitalWrite(YELLOWLED, LOW);
}

void Utils::onBuzzer()
{
    digitalWrite(BUZZER_PIN, HIGH);
    delay(100);
    digitalWrite(BUZZER_PIN, LOW);
}

// ========== CONTROL INDIVIDUAL DE LEDS ==========

void Utils::onRedLed()
{
    digitalWrite(REDLED, HIGH);
}

void Utils::onBlueLed()
{
    digitalWrite(BLUELED, HIGH);
}

void Utils::onYellowLed()
{
    digitalWrite(YELLOWLED, HIGH);
}

void Utils::onGreenLed()
{
    digitalWrite(GREENLED, HIGH);
}

// ========== EFECTOS DE PARPADEO ==========

void Utils::redLedBlink()
{
    digitalWrite(REDLED, HIGH);
    delay(100);
    digitalWrite(REDLED, LOW);
    delay(100);
}

void Utils::blueLedBlink()
{
    digitalWrite(BLUELED, HIGH);
    delay(100);
    digitalWrite(BLUELED, LOW);
    delay(100);
}

void Utils::greenLedBlink()
{
    digitalWrite(GREENLED, HIGH);
    delay(100);
    digitalWrite(GREENLED, LOW);
    delay(100);
}

void Utils::yellowLedBlink()
{
    digitalWrite(YELLOWLED, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(YELLOWLED, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
}

// ========== PATRONES DE ILUMINACIÓN ==========

void Utils::lightsAfternoon()
{
    digitalWrite(BLUELED, LOW);
    digitalWrite(YELLOWLED, HIGH);
    digitalWrite(REDLED, HIGH);
    Serial.println("[PATTERN] Patrón vespertino activado");
}

void Utils::lightsTomorrow()
{
    digitalWrite(BLUELED, HIGH);
    digitalWrite(YELLOWLED, HIGH);
    digitalWrite(REDLED, LOW);
    Serial.println("[PATTERN] Patrón matutino activado");
}

// ========== FUNCIONES DE CONECTIVIDAD ==========

void Utils::ScanWifi()
{
    yellowLedBlink();
    Serial.println("[WIFI] Indicador de escaneo activado");
}

/**
 * @brief Conecta a una red WiFi específica
 * @param ssid Nombre de la red WiFi
 * @param password Contraseña de la red WiFi
 * @return true si la conexión fue exitosa, false si falló
 * @details Intenta conectar hasta max_retries veces, configura NTP si es exitoso
 */
bool Utils::connecToWifi(const char *ssid, const char *password)
{
    Serial.printf("[WIFI] Intentando conectar a: %s\n", ssid);

    WiFi.begin(ssid, password);
    retries = 0;

    while (WiFi.status() != WL_CONNECTED && retries < max_retries)
    {
        Serial.printf("[WIFI] Intento %d/%d...\n", retries + 1, max_retries);
        ScanWifi(); // Indicador visual
        retries++;
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("[WIFI] Conectado exitosamente");
        Serial.printf("[WIFI] IP asignada: %s\n", WiFi.localIP().toString().c_str());
        Serial.printf("[WIFI] Gateway: %s\n", WiFi.gatewayIP().toString().c_str());

        // Configurar sincronización de tiempo
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.println("[NTP] Sincronización de tiempo configurada");

        // Indicador visual de conexión exitosa
        digitalWrite(YELLOWLED, HIGH);
        return true;
    }
    else
    {
        Serial.println("[WIFI] Error de conexión después de todos los intentos");
        return false;
    }
}

// ========== FUNCIONES DE VALIDACIÓN ==========

bool Utils::isUpper(char c)
{
    return c >= 'A' && c <= 'Z';
}

bool Utils::isAlpha(char c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == 'Ñ' || c == 'ñ';
}

bool Utils::isDigit(char c)
{
    return c >= '0' && c <= '9';
}

// ========== FUNCIÓN DE DECODIFICACIÓN ==========

/**
 * @brief Decodifica texto usando cifrado César y extrae DNI
 * @param text Texto cifrado a decodificar
 * @param shift Desplazamiento del cifrado César (por defecto 3)
 * @return String con el DNI de 8 dígitos extraído, "" si no se encuentra
 * @details Aplica cifrado César inverso y busca los últimos 8 dígitos consecutivos
 */
String Utils::cesarCipherDecode(String text, int shift)
{
    String result = "";
    shift = shift % 26;

    // Aplicar decodificación César
    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];

        if (c >= 'A' && c <= 'Z')
        {
            c = (c - 'A' - shift + 26) % 26 + 'A';
        }
        else if (c >= 'a' && c <= 'z')
        {
            c = (c - 'a' - shift + 26) % 26 + 'a';
        }
        else if (c >= '0' && c <= '9')
        {
            c = (c - '0' - shift + 10) % 10 + '0';
        }
        result += c;
    }

    // Reemplazar caracteres especiales
    result.replace('$', ' ');

    // Buscar los últimos 8 dígitos consecutivos (DNI)
    int lastIndex = -1;
    for (int i = result.length() - 8; i >= 0; i--)
    {
        bool isDni = true;
        for (int j = 0; j < 8; j++)
        {
            if (!isdigit(result[i + j]))
            {
                isDni = false;
                break;
            }
        }
        if (isDni)
        {
            lastIndex = i;
            break;
        }
    }

    if (lastIndex != -1)
    {
        String dni = result.substring(lastIndex, lastIndex + 8);
        Serial.printf("[DECODE] DNI extraído: %s\n", dni.c_str());
        return dni;
    }
    else
    {
        Serial.println("[DECODE] No se encontró un DNI válido");
        return "";
    }
}

// ========== FUNCIONES DE COMUNICACIÓN CON SERVIDOR ==========

/**
 * @brief Task estático para envío de datos al servidor
 * @param pvParameters Puntero a la instancia de Utils
 * @details Task de FreeRTOS que procesa la cola de asistencias y las envía al servidor
 */
void Utils::sendDataToServer(void *pvParameters)
{
    Utils *self = static_cast<Utils *>(pvParameters);
    Serial.println("[TASK] sendDataToServer iniciado");

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("[SEND] ⚠️ Sin conexión WiFi. Reintentando en 5s...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        AttendanceData dataToSend;
        if (xQueueReceive(self->attendanceQueue, &dataToSend, pdMS_TO_TICKS(1000)) == pdTRUE)
        {
            Serial.printf("[SEND] Procesando: DNI=%s, Tipo=%s\n", dataToSend.dni, dataToSend.type);

            DynamicJsonDocument jsonDoc(512); // aumentar si es necesario
            jsonDoc["dni"] = dataToSend.dni;
            jsonDoc["type_assistance"] = dataToSend.type;

            String jsonString;
            serializeJson(jsonDoc, jsonString);

            HTTPClient http;
            // Si usas HTTPS en api_url, considera usar WiFiClientSecure y http.begin(client, url)
            http.begin(api_url);
            http.addHeader("Content-Type", "application/json");
            http.addHeader("Authorization", auth_token);

            int httpResponseCode = http.POST(jsonString);
            if (httpResponseCode > 0)
            {
                String response = http.getString();
                Serial.printf("[SEND] ✅ Enviado: %s, Respuesta: %s\n", jsonString.c_str(), response.c_str());
                //setDeviceWorking(true);
            }
            else
            {
                Serial.printf("[SEND] ❌ Error al enviar: Código=%d, Detalle=%s\n", httpResponseCode,
                              http.errorToString(httpResponseCode).c_str());
                //setDeviceWorking(false);
                // Reintentar: colocar al frente de la cola para reintento rápido
                if (xQueueSendToFront(self->attendanceQueue, &dataToSend, pdMS_TO_TICKS(0)) != pdTRUE)
                {
                    Serial.println("[SEND] ⚠️ No se pudo reencolar (cola llena)");
                }
            }
            http.end();
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // no bloquear
    }
}

/**
 * @brief Agrega un DNI a la cola de procesamiento
 * @param dni DNI del usuario escaneado
 * @param attendanceType Tipo de asistencia ("entrance" o "exit")
 * @details Crea un elemento AttendanceData y lo agrega a la cola para procesamiento asíncrono
 */
void Utils::addToQueueIfUnique(String dni, String attendanceType)
{
    AttendanceData data;

    // Copiar DNI de forma segura
    strncpy(data.dni, dni.c_str(), sizeof(data.dni) - 1);
    data.dni[sizeof(data.dni) - 1] = '\0';

    // Copiar tipo de asistencia de forma segura
    strncpy(data.type, attendanceType.c_str(), sizeof(data.type) - 1);
    data.type[sizeof(data.type) - 1] = '\0';

    // Agregar timestamp
    // data.timestamp = millis();

    // Intentar agregar a la cola
    if (xQueueSend(attendanceQueue, &data, pdMS_TO_TICKS(100)) == pdTRUE)
    {
        Serial.printf("[QUEUE] DNI agregado: %s (%s)\n", dni.c_str(), attendanceType.c_str());
    }
    else
    {
        Serial.println("[QUEUE] Error: Cola llena, no se pudo agregar");
    }
}

// =============================================
// ========== IMPLEMENTACIÓN DE TASKS ==========
// =============================================

/**
 * @brief Task para escaneo de códigos de barras
 * @param pvParameters Puntero a instancia de Utils
 * @details Monitorea Serial2 para códigos escaneados, los decodifica y procesa
 */
void scanTask(void *pvParameters)
{
    Utils *ut = static_cast<Utils *>(pvParameters);
    Serial.println("[TASK] scanTask iniciado");

    while (1)
    {
        if (Serial2.available())
        {
            // Feedback inmediato de escaneo
            ut->onBuzzer();
            ut->onGreenLed();

            // Leer y decodificar DNI
            String dni = Serial2.readStringUntil('\n');
            dni = ut->cesarCipherDecode(dni, 3);

            if (dni.length() > 0)
            {
                // Incrementar contador y procesar
                incrementScanCount();
                ut->addToQueueIfUnique(dni, manualAttendanceType);

                Serial.printf("[SCAN] DNI procesado: %s (Total: %d)\n",
                              dni.c_str(), scanCount);
            }

            // Apagar LED verde después del procesamiento
            digitalWrite(GREENLED, LOW);
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

/**
 * @brief Task para reporte periódico de estado del dispositivo
 * @param pvParameters Puntero a instancia de Utils
 * @details Envía el estado del dispositivo al servidor cada 5 segundos
 */
void statusReportTask(void *pvParameters)
{
    Utils *ut = static_cast<Utils *>(pvParameters);
    Serial.println("[TASK] statusReportTask iniciado");

    while (1)
    {
        bool currentStatus = (WiFi.status() == WL_CONNECTED);

        if (currentStatus)
        {
            HTTPClient http;

            // Configurar conexión HTTP
            http.begin("http://10.217.191.181:3000/signal");
            http.addHeader("Content-Type", "application/json");
            http.setTimeout(10000);

            // Crear JSON con estado del dispositivo
            DynamicJsonDocument doc(512);
            doc["id"] = DEVICE_ID;
            doc["cantidad"] = scanCount;
            doc["conectado"] = currentStatus;
            // doc["timestamp"] = millis();

            String jsonString;
            serializeJson(doc, jsonString);

            Serial.printf("[STATUS] 📤 Enviando: %s\n", jsonString.c_str());

            // Enviar reporte
            int httpResponseCode = http.POST(jsonString);

            if (httpResponseCode > 0)
            {
                String response = http.getString();
                Serial.printf("[STATUS] ✅ Código: %d, Respuesta: %s\n",
                              httpResponseCode, response.c_str());
                setDeviceWorking(true);
            }
            else
            {
                Serial.printf("[STATUS] ❌ Error: %d - %s\n",
                              httpResponseCode, http.errorToString(httpResponseCode).c_str());
                setDeviceWorking(false);
            }

            http.end();
        }
        else
        {
            Serial.println("[STATUS] 📶 WiFi desconectado - Reporte pausado");
            setDeviceWorking(false);
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Reportar cada 5 segundos
    }
}

/**
 * @brief Task para reconexión automática de WiFi
 * @param pvParameters Puntero a instancia de Utils
 * @details Monitorea la conexión WiFi y reconecta automáticamente si se pierde
 */
void wifiReconnectTask(void *pvParameters)
{
    Utils *ut = static_cast<Utils *>(pvParameters);
    bool wasConnected = false;
    Serial.println("[TASK] wifiReconnectTask iniciado");

    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            if (wasConnected)
            {
                Serial.println("[WIFI] ⚠️ Conexión perdida, reintentando...");
            }

            ut->connecToWifi(WIFI_SSID.c_str(), WIFI_PASSWORD.c_str());
            wasConnected = false;
        }
        else
        {
            if (!wasConnected)
            {
                Serial.println("[WIFI] ✅ Conexión WiFi restaurada");
                ut->offLeds();
                digitalWrite(YELLOWLED, HIGH); // Indicador de conexión activa
                wasConnected = true;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(1000)); // Verificar cada segundo
    }
}

/**
 * @brief Task para determinar tipo de asistencia según la hora
 * @param pvParameters Puntero a instancia de Utils
 * @details Cambia automáticamente entre "entrance" y "exit" según la hora del día
 */
void attendanceTypeTask(void *pvParameters)
{
    Utils *ut = static_cast<Utils *>(pvParameters);
    Serial.println("[TASK] attendanceTypeTask iniciado");

    while (1)
    {
        if (WiFi.status() == WL_CONNECTED)
        {
            struct tm timeinfo;

            if (getLocalTime(&timeinfo))
            {
                // Apagar LEDs de estado anterior
                digitalWrite(REDLED, LOW);
                digitalWrite(BLUELED, LOW);

                if (timeinfo.tm_hour < 12)
                {
                    // Horario matutino - Entrada
                    setAttendanceType("entrance");
                    ut->onBlueLed();
                    Serial.printf("[ATTENDANCE] 🌅 Modo entrada activo (%02d:%02d)\n",
                                  timeinfo.tm_hour, timeinfo.tm_min);
                }
                else
                {
                    // Horario vespertino - Salida
                    setAttendanceType("exit");
                    ut->onRedLed();
                    Serial.printf("[ATTENDANCE] 🌆 Modo salida activo (%02d:%02d)\n",
                                  timeinfo.tm_hour, timeinfo.tm_min);
                }
            }
            else
            {
                Serial.println("[ATTENDANCE] ⚠️ No se pudo obtener la hora local");
            }
        }
        else
        {
            Serial.println("[ATTENDANCE] ⚠️ WiFi desconectado - Usando modo manual");
        }

        vTaskDelay(pdMS_TO_TICKS(5000)); // Verificar cada 5 segundos
    }
}