#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

RTC_DS1307 rtc;
const int ledPin = 13;

void setup() {
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();  // Clear the buffer
  delay(2000);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(ledPin, OUTPUT);  // Configuration de la broche de la LED en tant que sortie
}

void loop() {
  DateTime now = rtc.now();

  // Clignotement de la LED chaque seconde
  digitalWrite(ledPin, HIGH); // Allumer la LED
  delay(500); // Attendre 500 millisecondes (demi-seconde)
  digitalWrite(ledPin, LOW); // Éteindre la LED
  delay(500); // Attendre 500 millisecondes (demi-seconde)

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(now.year(), DEC);
  display.print('/');
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.day(), DEC);
  display.setCursor(0, 10);
  display.print(now.hour(), DEC);
  display.print(':');
  display.print(now.minute(), DEC);
  display.print(':');
  display.print(now.second(), DEC);

  display.display();
  delay(1000);
}
