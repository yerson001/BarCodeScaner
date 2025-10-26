� ESP32 Barcode Scanner System

Sistema de scanner de códigos de barras para registro automático de asistencia usando ESP32. El dispositivo escanea DNIs cifrados, los decodifica y envía los datos a un servidor para control de asistencia.

💚 Características

✅ Escaneo automático de códigos de barras → Lee DNIs cifrados desde scanner conectado por Serial2
✅ Decodificación César → Decodifica automáticamente DNIs con cifrado César (shift=3)
✅ Control por horario → Cambia automáticamente entre entrada/salida según la hora del día
✅ Conectividad WiFi → Reconexión automática y reporte de estado al servidor
✅ Feedback visual/sonoro → LEDs indicadores y buzzer para confirmación de operaciones
✅ Sistema multitarea → Procesamiento en tiempo real usando FreeRTOS

⚙️ Configuración del Hardware

El sistema utiliza los siguientes componentes conectados al ESP32:

### 📡 Scanner de códigos
```cpp
Serial2.begin(115200, SERIAL_8N1, 13, 14);  // RX=13, TX=14
```

### 💡 LEDs indicadores
```cpp
#define REDLED 32      // LED rojo - Modo salida/tarde
#define BLUELED 33     // LED azul - Modo entrada/mañana  
#define YELLOWLED 12   // LED amarillo - WiFi conectado
#define GREENLED 2     // LED verde - Escaneo exitoso
#define BUZZER_PIN 15  // Buzzer para feedback sonoro
```

### 🌐 Configuración de red WiFi
```cpp
String wifi = "";        // Nombre de la red WiFi
String password = "";   // Contraseña de la red WiFi
```

🔧 Funcionamiento del Sistema

El sistema funciona con múltiples tareas ejecutándose en paralelo:

### � Task de Escaneo (`scanTask`)
```cpp
void scanTask(void *pvParameters) {
    // Lee códigos del Serial2 (scanner)
    // Decodifica DNI usando cifrado César  
    // Proporciona feedback visual y sonoro
    // Agrega datos a cola de procesamiento
}
```

### � Task de Reporte (`statusReportTask`) 
```cpp
void statusReportTask(void *pvParameters) {
    // Envía estado del dispositivo cada 5 segundos
    // Datos: ID dispositivo, contador escaneos, estado conexión
    // URL: http://10.68.184.181:3000/signal
}
```

### 🌐 Task de WiFi (`wifiReconnectTask`)
```cpp 
void wifiReconnectTask(void *pvParameters) {
    // Monitorea conexión WiFi cada segundo
    // Reconecta automáticamente si se pierde conexión
    // Controla LED amarillo como indicador
}
```

### ⏰ Task de Asistencia (`attendanceTypeTask`)
```cpp
void attendanceTypeTask(void *pvParameters) {
    // Cambia automáticamente entre entrada/salida según horario
    // Antes de 12 PM: "entrance" (LED azul)
    // Después de 12 PM: "exit" (LED rojo)
}
```

### 🚀 Task de Envío (`sendDataToServer`)
```cpp
void sendDataToServer(void *pvParameters) {
    // Procesa cola de asistencias
    // Envía datos al servidor mediante HTTP POST
    // Maneja errores y reintentos
}
```

� Estados y Datos del Sistema

### 🎯 Variables de Estado Principales
```cpp
char manualAttendanceType[10] = "entrance";  // Tipo actual: "entrance" o "exit"
char dniData[20];                           // DNI procesado
int scanCount = 0;                          // Contador total de escaneos
bool deviceWorking = true;                  // Estado de funcionamiento
const char* deviceId = "001";               // ID único del dispositivo
```

### 📈 Datos Enviados al Servidor
```json
{
  "id": "001",                    // ID del dispositivo
  "cantidad": 15,                 // Total de escaneos realizados
  "conectado": true               // Estado de conexión WiFi
}
```

### 💡 Estados de LEDs
| LED | Color | Estado | Significado |
|-----|-------|--------|-------------|
| GREENLED | Verde | Parpadeo | Código escaneado exitosamente |
| YELLOWLED | Amarillo | Fijo | WiFi conectado |
| BLUELED | Azul | Fijo | Modo entrada (mañana < 12 PM) |
| REDLED | Rojo | Fijo | Modo salida (tarde > 12 PM) |

⚖️ Instalación y Configuración

### 📋 Requisitos
- **Hardware**: ESP32 DevKit, Scanner de códigos de barras, LEDs, Buzzer, Resistencias
- **Software**: PlatformIO, Visual Studio Code
- **Librerías**: ArduinoJson, HTTPClient, WiFi, time

### 🔧 Configuración del proyecto
1. **Clonar el repositorio**:
```bash
git clone https://github.com/cole-checkBarCodeScaner.git
cd BarCodeScaner
```

2. **Configurar PlatformIO** (`platformio.ini`):
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200
lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
```

3. **Configurar red WiFi** (en `src/main.cpp`):
```cpp
String wifi = "TU_RED_WIFI";        // Cambiar por tu red
String password = "TU_PASSWORD";    // Cambiar por tu contraseña
```

4. **Configurar servidor** (en `statusReportTask`):
```cpp
http.begin("http://TU_IP:3000/signal");  // Cambiar IP del servidor
```

5. **Compilar y subir**:
```bash
pio run --target upload
pio device monitor  # Para ver logs
```
🔄 Estructura del Proyecto

```
BarCodeScaner/
├── include/
│   └── Utils.hpp          # Declaraciones de clases y funciones
├── src/
│   ├── main.cpp          # Punto de entrada principal
│   └── Utils.cpp         # Implementación de funcionalidades
├── test/
│   ├── qrv1.cpp         # Tests de QR versión 1
│   ├── qrv2.cpp         # Tests de QR versión 2
│   └── README           # Documentación de tests
├── platformio.ini        # Configuración del proyecto
└── README.md            # Este archivo
```

### 🏗️ Arquitectura del Sistema
- **Modular**: Código organizado en clases y funciones específicas
- **Multitarea**: Usa FreeRTOS para procesamiento paralelo
- **Event-driven**: Responde a eventos de escaneo y red
- **Configurable**: Parámetros fácilmente modificables

### 🔗 APIs y Endpoints
- **Status Report**: `POST /signal` - Reporta estado del dispositivo
- **Attendance**: `POST /api/register_assistance` - Registra asistencia
📈 Flujo de Operación

### 🔄 Secuencia de Funcionamiento

1. **Inicio del Sistema**
   - ⚡ ESP32 se enciende y ejecuta setup()
   - 🔊 Doble beep de confirmación
   - 🌐 Conexión automática a WiFi
   - ⏰ Sincronización de tiempo NTP
   - 🚀 Creación de tareas en paralelo

2. **Operación Normal**
   - 👁️ Monitoreo continuo del scanner en Serial2
   - 📱 Cuando se escanea un código:
     - 🔊 Feedback sonoro (buzzer)
     - 💚 LED verde indica escaneo
     - 🔓 Decodificación César del DNI
     - ➕ Incremento de contador
     - 📤 Envío a cola de procesamiento

3. **Procesamiento de Datos**
   - 📋 Cola procesa asistencias en segundo plano
   - 🌐 Envío HTTP POST al servidor
   - 📊 Reporte de estado cada 5 segundos
   - ⏰ Cambio automático entrada/salida por horario

4. **Manejo de Errores**
   - 🔄 Reconexión automática WiFi
   - ⚠️ Logs de errores en Serial Monitor
   - 🔁 Reintentos automáticos de envío


📚 Conclusión

Este sistema proporciona una solución completa y robusta para el registro automático de asistencia mediante escaneo de códigos de barras. Con su arquitectura modular y sistema multitarea, ofrece:

- **Operación autónoma** con reconexión automática
- **Feedback inmediato** visual y sonoro  
- **Sincronización temporal** para control por horarios
- **Comunicación en tiempo real** con servidor
- **Manejo robusto de errores** y reconexiones

El código está documentado y estructurado para facilitar el mantenimiento y futuras expansiones del sistema.

💌 Contacto

Si tienes preguntas, mejoras o encuentras bugs, por favor abre un issue en el repositorio. 🚀