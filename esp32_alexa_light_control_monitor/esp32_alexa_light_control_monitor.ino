//  note:
//    use Arduino 1.8.5
//    Arduino board manager
//      https://dl.espressif.com/dl/package_esp32_index.json
//    ボード ESP32 Dev Module
//    Flash Mode  QIO
//    Flash Frequency 80MHz
//    Flash Size  4M (32Mb)
//    Partition Scheme  No OTA (2MB APP/2MB FATFS)
//    Upload Speed  115200
//    Core Debug Level なし

#include <Arduino.h>
#include <WiFi.h>

#include <IRremoteESP8266.h>
#include <IRsend.h>

#include <Ticker.h>
#include <SPI.h>

#include "fauxmoESP.h"
#include "config.h"

// TFT display module
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

#define LIGHT_PIN 19

#define TFT_CS   13
#define TFT_DC   14
#define TFT_MOSI 27
#define TFT_CLK  26
#define TFT_RST  12
#define TFT_MISO 25
#define TFT_LED  17

#define TFT_T_IRQ   23

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS,TFT_DC,TFT_MOSI,TFT_CLK,TFT_RST,TFT_MISO);

struct Touch {
  const uint8_t PIN;
  bool isDetected;
  bool timerIsStopped;
  uint8_t timeoutCount;
  const uint8_t TIMEOUT_MAX_COUNT;
};

Touch touch = {TFT_T_IRQ, false, true, 0, 5};

portMUX_TYPE intrMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTouch() {
  Serial.println("touched");
  portENTER_CRITICAL_ISR(&intrMux);
  touch.isDetected = true;
  portEXIT_CRITICAL_ISR(&intrMux);
}

void initTouch() {
  pinMode(touch.PIN, INPUT_PULLUP);
  attachInterrupt(touch.PIN, onTouch, FALLING);
}

void onTimer() {
  Serial.println("timeout");
  portENTER_CRITICAL_ISR(&intrMux);
  touch.timeoutCount++; // inside a critical section
  portEXIT_CRITICAL_ISR(&intrMux);
}

Ticker ticker1;

void startTimer() {
  Serial.println("start timer");
  ticker1.attach_ms(1000, onTimer);
  touch.timerIsStopped = false;
}

void stopTimer() {
  Serial.println("stop timer");
  ticker1.detach();
  touch.timerIsStopped = true;
}


typedef enum {
    NO_EVENT    = 0,
    LIGHT_ON,
    LIGHT_OFF
} ALEXA_EVENT;

struct AlexaInfo {
  ALEXA_EVENT event;
  bool lightIsOn;
};

AlexaInfo alexa = {NO_EVENT, true};

IRsend irsend(LIGHT_PIN);

fauxmoESP fauxmo;

char ssid[] = AP_SSID;
char password[] = AP_PASS;

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

    tft.begin();
    tft.fillScreen(ILI9341_BLACK);
    yield();
    tft.setCursor(0,0);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.setRotation(3);
  
    pinMode(TFT_LED, OUTPUT);
    digitalWrite(TFT_LED, HIGH); // back light on
  
    tft.println("Start");

}

void handleEvent() {
    switch (alexa_event) {
      case LIGHT_ON:
        Serial.println("LIGHT ON");
        irsend.sendNEC(LIGHT_COMMAND_ON,32); // ON
        delay(50);
        irsend.sendNEC(LIGHT_COMMAND_ON,32); // ON
        delay(50);
        irsend.sendNEC(LIGHT_COMMAND_ON,32); // ON
        delay(100);
        tft.println("LIGHT ON");
        alexa_event = NO_EVENT;
        break;
      case LIGHT_OFF:
        Serial.println("LIGHT OFF");
        irsend.sendNEC(LIGHT_COMMAND_OFF,32); // OFF
        delay(50);
        irsend.sendNEC(LIGHT_COMMAND_OFF,32); // OFF
        delay(50);
        irsend.sendNEC(LIGHT_COMMAND_OFF,32); // OFF
        delay(100);
        tft.println("LIGHT OFF");
        alexa_event = NO_EVENT;
        break;
      default:
        break;
    }
}

void loop() {
    handleEvent();
    fauxmo.handle();
}

//void setup() {
//  Serial.begin(115200);
//
//  initTouch();
//  irsend.begin();
//
//  tft.begin();
//  tft.fillScreen(ILI9341_BLACK);
//  yield();
//  tft.setCursor(0,0);
//  tft.setTextColor(ILI9341_GREEN);
//  tft.setTextSize(2);
//  tft.setRotation(3);
//
//  pinMode(TFT_LED, OUTPUT);
//  digitalWrite(TFT_LED, HIGH); // back light on
//
//  tft.println("Start");
//  touch.timeoutCount = 0;
//  touch.isDetected = true;
//  startTimer();
//}
//
//void loop() {
//  delay(50);
//  if (touch.isDetected) {
//    if (touch.timerIsStopped) {
//      tft.println("Touched & LED on");
//      digitalWrite(TFT_LED, HIGH); // back light on
//      touch.timeoutCount = 0;
//      startTimer();
//
//      if (alexa.lightIsOn) {
//        alexa.lightIsOn = false;
//        Serial.println("LIGHT OFF");
//        irsend.sendNEC(0x0000000, 32); //OFF
//      }
//      else {
//        alexa.lightIsOn = true;
//        Serial.println("LIGHT ON");
//        irsend.sendNEC(0x0000000, 32); //ON 
//      }
//
//    }
//    else { // timer is stareted
//      if (touch.timeoutCount >= touch.TIMEOUT_MAX_COUNT) {
//        stopTimer();
//        tft.println("LED off");
//        delay(1000);
//        portENTER_CRITICAL_ISR(&intrMux);
//        touch.isDetected = false;
//        portEXIT_CRITICAL_ISR(&intrMux);
//        digitalWrite(TFT_LED, LOW); // back light off
//      }
//    }
//  }
//}

