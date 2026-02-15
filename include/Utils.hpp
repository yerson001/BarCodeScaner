#ifndef UTILS_HPP
#define UTILS_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#define REDLED 32
#define BLUELED 33
#define YELLOWLED 12

#define BUZZER_PIN 15
#define GREENLED 2
#define BUTTON_PIN 23


class Utils
{
public:
    Utils();

public:
    /************** TASK *******
     *   LED CONFIGURATION FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: None
     *   @return: None
     **************************** */
    time_t hour();
    void setLeds();
    void redLedBlink();
    void onRedLed();
    void onBlueLed();
    void onYellowLed();
    void onGreenLed();
    void lightsTomorrow();
    void lightsAfternoon();
    void blueLedBlink();
    void greenLedBlink();
    void yellowLedBlink();
    void ScanWifi();
    void offLeds();
    void onBuzzer();
    /************** TASK *******
     *   WIFI CONFIGURATION FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: None
     *   @return: None
     **************************** */
    bool connecToWifi(const char *, const char *);
    const int max_retries = 5;
    int retries;
    /************** TASK *******
     *   DECODE  FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: KEY 3
     *   @return:
     **************************** */
    bool isUpper(char c);
    bool isAlpha(char c);
    bool isDigit(char c);
    String cesarCipherDecode(String text, int shift);

    /************** TASK *******
     *   NTP SERVER  FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: pool.ntp.org
     *   @return:
     **************************** */
    const char *ntpServer;
    const long gmtOffset_sec = -5 * 3600;
    const int daylightOffset_sec = 0;
    /************** TASK *******
     *   SEND DATA TO SERVER  FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: pool.ntp.org
     *   @return:
     **************************** */

    static void sendDataToServer(void *);
    QueueHandle_t attendanceQueue;
    void addToQueueIfUnique(String dni, String attendanceType);
    //const char api_url[] PROGMEM = "https://colecheck.com/api/register_assistance";
    //const char auth_token[] PROGMEM = "1ae9465337bf4747def0bdca0a1eb3dac096fd52";

    //std::vector<String> processedDnis;
    //const int MAX_PROCESSED_SIZE = 450;

    //void clearProcessedDnis();
};

#endif // UTILS_HPP