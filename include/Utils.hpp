#ifndef UTILS_HPP
#define UTILS_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>

#define REDLED 32
#define BLUELED 33
#define GREENLED 12

class Utils
{
public:
    Utils(const char* ssid,const char* password);
public:
    /************** TASK *******
     *   LED CONFIGURATION FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: None
     *   @return: None
     **************************** */
    void setLeds();
    void redLedBlink();
    void onRedLed();
    void onBlueLed();
    void onGreenLed();
    void lightsTomorrow();
    void lightsAfternoon();
    void blueLedBlink();
    void greenLedBlink();
    void ScanWifi();
    void offLeds();
    /************** TASK *******
     *   WIFI CONFIGURATION FUNCTIONS
     *   @brief: Set the pins of the LEDs as OUTPUT
     *   @param: None
     *   @return: None
     **************************** */
    bool connecToWifi();
    const char* ssid;
    const char* password;
    const int max_retries = 30;
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
    const char* ntpServer;
    const long gmtOffset_sec = -5 * 3600;
    const int daylightOffset_sec = 0;
};

#endif // UTILS_HPP