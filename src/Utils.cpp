#include "Utils.hpp"

struct AttendanceData
{
    char dni[20];
    char type[10];
    //time_t hour;
};

Utils::Utils()
{
    this->ntpServer = "pool.ntp.org";
    attendanceQueue = xQueueCreate(100, sizeof(AttendanceData));
    xTaskCreate(Utils::sendDataToServer, "sendDataToServer", 10000, this, 1, NULL); // Pass 'this' as parameter
}

time_t Utils::hour()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return 0; // o algún valor de error
    }
    return mktime(&timeinfo); // Retorna la hora en formato UNIX timestamp
}

void Utils::setLeds()
{
    pinMode(REDLED, OUTPUT);
    pinMode(BLUELED, OUTPUT);
    pinMode(YELLOWLED, OUTPUT);
    pinMode(GREENLED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);
}

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

void Utils::yellowLedBlink() {
    digitalWrite(YELLOWLED, HIGH);
    vTaskDelay(pdMS_TO_TICKS(500));
    digitalWrite(YELLOWLED, LOW);
    vTaskDelay(pdMS_TO_TICKS(500));
}

void Utils::ScanWifi()
{
    yellowLedBlink();
    //blueLedBlink();
    //redLedBlink();
    // greenFlagLedBlink();
    //  onBuzzer();
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

void Utils::lightsAfternoon()
{
    digitalWrite(BLUELED, LOW);
    digitalWrite(YELLOWLED, HIGH);
    digitalWrite(REDLED, HIGH);
}

void Utils::lightsTomorrow()
{
    digitalWrite(BLUELED, HIGH);
    digitalWrite(YELLOWLED, HIGH);
    digitalWrite(REDLED, LOW);
}

bool Utils::connecToWifi(const char *ssid, const char *password)
{
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < max_retries)
    {
        ScanWifi();
        retries++;
        //ScanWifi();
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("Connected to WiFi");
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        digitalWrite(YELLOWLED, HIGH);
        return true;
    }
    else
    {
        Serial.println("Could not connect to WiFi");
        //digitalWrite(REDLED, HIGH);
        return false;
    }
}

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
String Utils::cesarCipherDecode(String text, int shift)
{
    String result = "";
    shift = shift % 26;

    for (int i = 0; i < text.length(); i++)
    {
        char c = text[i];

        if (c >= 'A' && c <= 'Z')
        { // Mayúsculas
            c = (c - 'A' - shift + 26) % 26 + 'A';
        }
        else if (c >= 'a' && c <= 'z')
        { // Minúsculas
            c = (c - 'a' - shift + 26) % 26 + 'a';
        }
        else if (c >= '0' && c <= '9')
        { // Números
            c = (c - '0' - shift + 10) % 10 + '0';
        }
        result += c;
    }

    result.replace('$', ' '); // Reemplazar '$' por espacio

    // Buscar los últimos 8 dígitos consecutivos
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
        return result.substring(lastIndex, lastIndex + 8);
    }
    else
    {
        return ""; // No se encontró un DNI válido
    }
}

void Utils::sendDataToServer(void *pvParameters)
{
    Utils *self = static_cast<Utils *>(pvParameters);
    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("No WiFi connection. Retrying in 5 seconds...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue; 
        }
        else
        {
            if (uxQueueMessagesWaiting(self->attendanceQueue) > 0)
            {
                AttendanceData dataToSend;
                xQueueReceive(self->attendanceQueue, &dataToSend, portMAX_DELAY);
                delay(2000);
                Serial.print("\nSEND: ");
                Serial.print(dataToSend.dni);
                Serial.print("\nTYPE: ");
                Serial.println(dataToSend.type);
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}

// Version con hora
/*
void Utils::sendDataToServer(void *pvParameters)
{
    Utils *self = static_cast<Utils *>(pvParameters); // Cast pvParameters to Utils*
    while (1)
    {
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.println("No WiFi connection. Retrying in 5 seconds...");
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue; // Skip sending if no WiFi
        }
        else
        {
            if (uxQueueMessagesWaiting(self->attendanceQueue) > 0)
            {
                AttendanceData dataToSend;
                xQueueReceive(self->attendanceQueue, &dataToSend, portMAX_DELAY);
                delay(2000);
                Serial.print("\nSEND: ");
                Serial.print(dataToSend.dni);
                if (dataToSend.hour < 72000)
                    dataToSend.hour = self->hour() - (millis() / 1000 - dataToSend.hour);

                char buffer[32];
                strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&dataToSend.hour)); // Formato legible

                Serial.print("\nHOUR: ");
                Serial.println(buffer);

                struct tm timeinfo;
                localtime_r(&dataToSend.hour, &timeinfo); // Convierte time_t a struct tm

                if (timeinfo.tm_hour < 12)
                {
                    strncpy(dataToSend.type, "entrance", sizeof(dataToSend.type) - 1);
                    dataToSend.type[sizeof(dataToSend.type) - 1] = '\0';
                }
                else
                {
                    strncpy(dataToSend.type, "exit", sizeof(dataToSend.type) - 1);
                    dataToSend.type[sizeof(dataToSend.type) - 1] = '\0';
                }
                Serial.print("TYPE: ");
                Serial.println(dataToSend.type);

                // self->redLedBlink(); // Use 'self' to call instance-specific methods
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}
*/
//
const char api_url[] = "https://colecheck.com/api/register_assistance";
// const char auth_token[] = "7925427fe1617bf00edf7169572b11614edf8d32"; // circa token
const char auth_token[] = "2e8c37e4a9023b3b20c952d85addc06b1d2ae559"; // prueba token

/*
void Utils::sendDataToServer(void *pvParameters)
{

    AttendanceData dataToSend;
    Utils *self = static_cast<Utils *>(pvParameters);

    while (1)
    {
        if (uxQueueMessagesWaiting(self->attendanceQueue) > 0)
        {
            xQueueReceive(self->attendanceQueue, &dataToSend, portMAX_DELAY);

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

            if (httpResponseCode > 0)
            {
                String response = http.getString();
                Serial.print("HTTP Response code: ");
                Serial.println(httpResponseCode);
                Serial.print("Response: ");
                Serial.println(response);
            }
            else
            {
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
void Utils::addToQueueIfUnique(String dni, String attendanceType)
{
    // Crear el objeto AttendanceData
    AttendanceData data;
    strncpy(data.dni, dni.c_str(), sizeof(data.dni) - 1);
    data.dni[sizeof(data.dni) - 1] = '\0';
    strncpy(data.type, attendanceType.c_str(), sizeof(data.type) - 1);
    data.type[sizeof(data.type) - 1] = '\0';
    //  Si hay WiFi, usa la hora NTP; si no, usa la hora local
    /*if (WiFi.status() == WL_CONNECTED)
    {
        data.hour = hour(); // Usa la función de hora actual (puede ser time(nullptr) o similar)
    }
    else
    {
        data.hour = (millis() / 1000); // Segundos desde la desconexión
    }*/
    // Enviar a la cola de asistencia
    xQueueSend(attendanceQueue, &data, portMAX_DELAY);
}