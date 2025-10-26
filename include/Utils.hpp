#ifndef UTILS_HPP
#define UTILS_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>

// ================================================================================================
// ========== CONFIGURACIÓN DE HARDWARE ==========
// ================================================================================================

#define REDLED 32
#define BLUELED 33
#define YELLOWLED 12
#define BUZZER_PIN 15
#define GREENLED 2
#define BUTTON_PIN 23

/**
 * @brief Configuración de red WiFi
 * @details Credenciales para conexión a red inalámbrica
 */
extern const String WIFI_SSID;           // Nombre de la red WiFi
extern const String WIFI_PASSWORD;       // Contraseña de la red WiFi

/**
 * @brief Identificación del dispositivo scanner
 */
// extern device id
extern const char* DEVICE_ID;               // ID único del dispositivo

/**
 * @brief Variables de estado del sistema de asistencia
 * @details Controlan el comportamiento y tipo de asistencia actual
 */
extern char manualAttendanceType[10];          // Tipo de asistencia actual ("entrance"/"exit")
extern char dniData[20];                       // DNI actualmente en procesamiento
extern bool manualOverride;                    // Indica si el modo manual está activo
extern bool attendanceToggle;                  // Estado del toggle de asistencia
extern const char* prevAttendanceType;        // Tipo de asistencia anterior

/**
 * @brief Variables de monitoreo del dispositivo
 * @details Controlan el estado operacional y estadísticas del scanner
 */
extern int scanCount;                          // Contador total de escaneos realizados
extern bool deviceWorking;                     // Estado de funcionamiento del dispositivo

void initializeSystemState();
int incrementScanCount();
void resetScanCount();
void setAttendanceType(const char* type);
void setDniData(const char* dni);
void setDeviceWorking(bool working);

class Utils
{
public:
    Utils();

public:
    // ========== FUNCIONES DE CONTROL DE LEDs ==========
    /**
     * @brief Funciones para control de hardware LED
     * @details Configuración y control individual de LEDs indicadores
     */
    time_t hour();
    void setLeds();              // Configura pines de LEDs como OUTPUT
    void redLedBlink();          // Parpadea LED rojo
    void onRedLed();             // Enciende LED rojo (salida/tarde)
    void onBlueLed();            // Enciende LED azul (entrada/mañana)
    void onYellowLed();          // Enciende LED amarillo (WiFi conectado)
    void onGreenLed();           // Enciende LED verde (escaneo exitoso)
    void lightsTomorrow();       // Patrón para horario matutino
    void lightsAfternoon();      // Patrón para horario vespertino
    void blueLedBlink();         // Parpadea LED azul
    void greenLedBlink();        // Parpadea LED verde
    void yellowLedBlink();       // Parpadea LED amarillo
    void offLeds();              // Apaga todos los LEDs
    void onBuzzer();             // Activa buzzer para feedback sonoro

    // ========== FUNCIONES DE CONECTIVIDAD WiFi ==========
    /**
     * @brief Funciones para manejo de conectividad inalámbrica
     */
    void ScanWifi();                                    // Escanea redes WiFi disponibles
    bool connecToWifi(const char* ssid, const char* password); // Conecta a red WiFi específica
    
private:
    const int max_retries = 5;   // Máximo número de reintentos de conexión
    int retries;                 // Contador actual de reintentos

public:
    // ========== FUNCIONES DE DECODIFICACIÓN ==========
    /**
     * @brief Funciones para procesamiento de datos escaneados
     * @details Decodificación de texto usando cifrado César (shift = 3)
     */
    bool isUpper(char c);                               // Verifica si carácter es mayúscula
    bool isAlpha(char c);                               // Verifica si carácter es alfabético
    bool isDigit(char c);                               // Verifica si carácter es dígito
    String cesarCipherDecode(String text, int shift);   // Decodifica texto con cifrado César

    // ========== CONFIGURACIÓN DE TIEMPO NTP ==========
    /**
     * @brief Configuración de servidor de tiempo
     * @details Parámetros para sincronización con servidor NTP
     */
    const char* ntpServer = "pool.ntp.org";     // Servidor NTP para sincronización
    const long gmtOffset_sec = -5 * 3600;       // Offset GMT en segundos (UTC-5 para Perú)
    const int daylightOffset_sec = 0;           // Offset horario de verano

    // ========== FUNCIONES DE COMUNICACIÓN CON SERVIDOR ==========
    /**
     * @brief Funciones para envío de datos de asistencia al servidor
     * @details Manejo de cola de asistencias y comunicación HTTP
     */
    static void sendDataToServer(void* pvParameters);           // Task para envío de datos
    QueueHandle_t attendanceQueue;                              // Cola de procesamiento de asistencias
    void addToQueueIfUnique(String dni, String attendanceType); // Agrega DNI a cola si es único
};

/**
 * @brief Tasks del sistema FreeRTOS
 * @details Funciones que se ejecutan en paralelo para diferentes funcionalidades
 */
void scanTask(void *pvParameters);           // Task para escaneo de códigos
void wifiReconnectTask(void *pvParameters);  // Task para reconexión WiFi automática
void attendanceTypeTask(void *pvParameters); // Task para determinar tipo de asistencia por horario
void statusReportTask(void *pvParameters);   // Task para reporte periódico de estado

#endif 