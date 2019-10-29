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

//#include <WiFiMulti.h>
#include <Arduino.h>
#include <WiFi.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>

#include "fauxmoESP.h"
#define SERIAL_BAUDRATE                 115200
#define LED                             2
#define LIGHT_PIN                       19

IRsend irsend(LIGHT_PIN);

fauxmoESP fauxmo;

char ssid[] = "";
char password[] = "";

const int ledpin = 26;
const char *id_led = "led";

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
    pinMode(ledpin, OUTPUT);
    digitalWrite(ledpin, LOW);
    WifiSetup();

    fauxmo.createServer(true);
    fauxmo.setPort(80);  //※
    fauxmo.enable(true);

//fauxmo.addDevice(id_led);
    fauxmo.addDevice("ライト");
    
    fauxmo.onSetState([](unsigned char device_id, const char *device_name, bool state, unsigned char value) {
      Serial.printf("Device #%d (%s) state: %s value: %d\n", device_id, device_name, state ? "ON" : "OFF", value);

//      if (strcmp(device_name, id_led) == 0) {
      if (strcmp(device_name, "ライト") == 0) {
//          digitalWrite(ledpin, state ? HIGH : LOW);
          if(state) {
            Serial.println("LIGHT ON");
            irsend.sendNEC(0x1768877,32); // ON
            delay(1000);
          }
          else {
            Serial.println("LIGHT OFF");
            irsend.sendNEC(0x17600FF,32); // OFF
            delay(1000);
          }
      }
    });
}

void loop() {
    fauxmo.handle();
}

//
//// -----------------------------------------------------------------------------
//// Wifi
//// -----------------------------------------------------------------------------
//
//void wifiSetup() {
//
//    // Set WIFI module to STA mode
//    WiFi.mode(WIFI_STA);
//
//    // Connect
//    Serial.printf("[WIFI] Connecting to %s ",ssid);
//    WiFi.begin(ssid, password);
//
//    // Wait
//    while (WiFi.status() != WL_CONNECTED) {
//        Serial.print(".");
//        delay(100);
//    }
//    Serial.println();
//
//    // Connected!
//    Serial.printf("[WIFI] STATION Mode, SSID: %s, IP address: %s\n", WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
//
//}
//
//void setup() {
//
//    // Init serial port and clean garbage
//    Serial.begin(SERIAL_BAUDRATE);
//    Serial.println();
//    Serial.println();
//
//    // Wifi
//    wifiSetup();
//
//    // LED
//    pinMode(LED, OUTPUT);
//    digitalWrite(LED, HIGH);
//
//    // You have to call enable(true) once you have a WiFi connection
//    // You can enable or disable the library at any moment
//    // Disabling it will prevent the devices from being discovered and switched
////    fauxmo.createServer(true);
////    fauxmo.setPort(80);
//    fauxmo.enable(true);
//
//    // Add virtual devices
//    fauxmo.addDevice("照明");
//
//    // fauxmoESP 2.0.0 has changed the callback signature to add the device_id,
//    // this way it's easier to match devices to action without having to compare strings.
//    fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state) {
//        Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF");
//        digitalWrite(LED, state);
//    });
//
//    // Callback to retrieve current state (for GetBinaryState queries)
//    fauxmo.onGetState([](unsigned char device_id, const char * device_name) {
//        return digitalRead(LED);
//    });
//
//}
//
//void loop() {
//
//    // Since fauxmoESP 2.0 the library uses the "compatibility" mode by
//    // default, this means that it uses WiFiUdp class instead of AsyncUDP.
//    // The later requires the Arduino Core for ESP8266 staging version
//    // whilst the former works fine with current stable 2.3.0 version.
//    // But, since it's not "async" anymore we have to manually poll for UDP
//    // packets
//    fauxmo.handle();
//
//    static unsigned long last = millis();
//    if (millis() - last > 5000) {
//        last = millis();
//        Serial.printf("[MAIN] Free heap: %d bytes\n", ESP.getFreeHeap());
//    }
//
//}


