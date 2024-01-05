#include <Wire.h>
#include <RTClib.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#define RTC_SDA_PIN A4
#define RTC_SCL_PIN A5

#define LED_DIN_PIN 11
#define LED_CLK_PIN 13
#define LED_CS_PIN 10

#define MAX_DEVICES 4  // Définir le nombre de modules MAX7219 que vous avez connectés
#define HARDWARE_TYPE MD_MAX72XX::PAROLA_HW

MD_Parola myDisplay = MD_Parola(HARDWARE_TYPE, LED_DIN_PIN, LED_CLK_PIN, LED_CS_PIN, MAX_DEVICES);

RTC_DS1307 rtc;

void setup() {
  Serial.begin(9600);

  Wire.begin();
  rtc.begin();

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  myDisplay.begin();
  myDisplay.setInvert(false);
}

void loop() {
  DateTime now = rtc.now();
  char timeString[6]; // HH:MM\0

  sprintf(timeString, "%02d:%02d", now.hour(), now.minute());


  myDisplay.displayText(timeString, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
  myDisplay.displayAnimate();
  delay(1000);
}
