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

#include <IRremoteESP8266.h>
#include <IRsend.h>

#include <Ticker.h>
#include <SPI.h>

// TFT display module
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

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


IRsend irsend(19);

struct AlexaInfo {
  bool lightIsOn;
};

AlexaInfo alexa = {true};

void setup() {
  Serial.begin(115200);

  initTouch();
  irsend.begin();

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
  touch.timeoutCount = 0;
  touch.isDetected = true;
  startTimer();
}

void loop() {
  delay(50);
  if (touch.isDetected) {
    if (touch.timerIsStopped) {
      tft.println("Touched & LED on");
      digitalWrite(TFT_LED, HIGH); // back light on
      touch.timeoutCount = 0;
      startTimer();

      if (alexa.lightIsOn) {
        alexa.lightIsOn = false;
        Serial.println("LIGHT OFF");
        irsend.sendNEC(0x17600FF, 32); //OFF
      }
      else {
        alexa.lightIsOn = true;
        Serial.println("LIGHT ON");
        irsend.sendNEC(0x1768877, 32); //ON 
      }

    }
    else { // timer is stareted
      if (touch.timeoutCount >= touch.TIMEOUT_MAX_COUNT) {
        stopTimer();
        tft.println("LED off");
        delay(1000);
        portENTER_CRITICAL_ISR(&intrMux);
        touch.isDetected = false;
        portEXIT_CRITICAL_ISR(&intrMux);
        digitalWrite(TFT_LED, LOW); // back light off
      }
    }
  }
}


//#include <IRremoteESP8266.h>
//#include <IRsend.h>
//
//IRsend irsend(19);
//
//void setup()
//{
//    Serial.begin(115200);
//      irsend.begin();
//}
//
//void loop() {
//  Serial.println("LIGHT OFF");
//	irsend.sendNEC(0x17600FF, 32); //OFF
//	delay(5000); //5 second delay between each signal burst
//  Serial.println("LIGHT ON");
//  irsend.sendNEC(0x1768877, 32); //ON 
//  delay(5000); //5 second delay between each signal burst
//}
