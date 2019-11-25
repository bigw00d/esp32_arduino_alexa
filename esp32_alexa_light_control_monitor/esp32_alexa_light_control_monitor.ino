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

// SPI
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
#define TFT_LED   2

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS,TFT_DC,TFT_MOSI,TFT_CLK,TFT_RST,TFT_MISO);

void setup() {
  Serial.begin(115200);

  pinMode(TFT_LED, OUTPUT);
  digitalWrite(TFT_LED, HIGH);
//  digitalWrite(TFT_LED, LOW); //test 

  tft.begin();
  tft.fillScreen(ILI9341_BLACK);
  yield();
  tft.setCursor(0,0);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  tft.setRotation(3);
  delay(3000);
  
  tft.println("Initialize ...");
  delay(1000);

  tft.println("Setting...");
  delay(500);
  tft.println("OK");
  tft.setTextColor(ILI9341_WHITE);
  tft.println("Ready");
}

void loop() {
  delay(50);
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
