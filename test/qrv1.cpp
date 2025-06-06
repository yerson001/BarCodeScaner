#include <Arduino.h>
#include "Utils.hpp"
#include <vector>
#include <ArduinoJson.h>
#include <HTTPClient.h>


Utils *ut = new Utils();

//ESP32AP::Config apConfig;

/************** TASK *******
 *   SETTING CONFIG
 *   @brief: Set the pins of the LEDs as OUTPUT
 *   @param: pool.ntp.org
 *   @return:
 **************************** */
char manualAttendanceType[10] = "entrance";
char dniData[20];
bool manualOverride = false;
bool attendanceToggle = true;
const char *prevAttendanceType = "";

String wifi = "CLARO_B253";
String password = "5wEs6DQpcp";


void setup()
{
  ut->onBuzzer();
  delay(2000);
  ut->onBuzzer();
  //Serial.begin(9600);
  //Serial2.begin(9600, SERIAL_8N1, 13, 14);
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, 13, 14);
   
 
  ut->setLeds();

  Serial.println("******** INIT SYSTEM ********");

  //if (ut->connecToWifi("CLARO_B253", "5wEs6DQpcp"))
  //if (ut->connecToWifi("YVONNE_EXT_5G", "PRESIOSARAV77"))
  if (ut->connecToWifi(wifi.c_str(),password.c_str()))
  {
    Serial.println("Conectado a la red WiFi");
  }
}

void loop()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    ut->offLeds();
    if (!ut->connecToWifi(wifi.c_str(),password.c_str()))
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
      //ut->clearProcessedDnis();
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
      //ut->clearProcessedDnis();
      delay(500);
    }

    if (Serial2.available())
    {
      ut->onBuzzer();
      //ut->greenFlagLedBlink();
      String dni = Serial2.readStringUntil('\n');
      //ut->greenFlagLedBlink();

      Serial.println("BEFORE DNI: ");
      Serial.println(dni);
      dni = ut->cesarCipherDecode(dni, 3);
      
      Serial.println("AFTER DNI: ");
      Serial.println(dni);

      

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
        ut->addToQueueIfUnique(dni, attendanceType);
      }
    }
  }
}