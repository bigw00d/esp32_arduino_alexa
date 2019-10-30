//  note:
//    use Arduino 1.8.5
//    Arduino board manager
//      https://dl.espressif.com/dl/package_esp32_index.json
//    ボード ESP32 Dev Module
//    Flash Mode  QIO
//    Flash Frequency 80MHz
//    Flash Size  4M (32Mb)
//    Partition Scheme  No OTA (2MB APP/2MB FATFS) <- important!
//    Upload Speed  115200
//    Core Debug Level なし

#include <Arduino.h>
#include <WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "fauxmoESP.h"
#define LIGHT_PIN                       19

typedef enum {
    NO_EVENT    = 0,
    LIGHT_ON,
    LIGHT_OFF
} ALEXA_EVENT;

IRsend irsend(LIGHT_PIN);

fauxmoESP fauxmo;

char ssid[] = "";
char password[] = "";

volatile ALEXA_EVENT alexa_event = NO_EVENT;

void WifiSetup()
{
  WiFi.mode(WIFI_STA);
  Serial.printf("connecting to %s¥n", ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }

  irsend.begin();

  Serial.println();
  Serial.print("WiFi connected: ");
  Serial.println(WiFi.localIP());
}

void setup() {
    Serial.begin(115200);

    WifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);
    fauxmo.enable(true);

    fauxmo.addDevice("ライト");
    
    fauxmo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
      //  If you have to do something more involved here set a flag and process it in your main loop
      Serial.printf("Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);
      if (strcmp(device_name, "ライト") == 0) {
          if(state) {
            alexa_event = LIGHT_ON;
          }
          else {
            alexa_event = LIGHT_OFF;
          }
      }
    });

}

void loop() {
    switch (alexa_event) {
      case LIGHT_ON:
        Serial.println("LIGHT ON");
        irsend.sendNEC(0x1768800,32); // ON
        alexa_event = NO_EVENT;
        break;
      case LIGHT_OFF:
        Serial.println("LIGHT OFF");
        irsend.sendNEC(0x17688FF,32); // OFF
        alexa_event = NO_EVENT;
        break;
      default:
        break;
    }
    
    fauxmo.handle();
}

