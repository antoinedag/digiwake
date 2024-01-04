#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Déclaration de l'écran OLED

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// Déclaration des boutons de reglage

#define BUTTON_HOUR_PIN 2
#define BUTTON_MINUTE_PIN 3
#define BUZZER_PIN 4

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Création de l'objet RTC

RTC_DS1307 rtc;

DateTime alarmTime;
bool alarmSet = false;

void setup() {            // vérification que le systeme reconnait bien les composants
  Serial.begin(9600);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  display.clearDisplay();  
  delay(2000);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  pinMode(BUTTON_HOUR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  alarmTime = DateTime(2024, 4, 1, 14, 30, 0); // Exemple avec un moment d'aujourd'hui
  alarmSet = true;
}

void loop() {
  DateTime now = rtc.now();

  if (digitalRead(BUTTON_HOUR_PIN) == LOW) {
    adjustTime(now, 3600); // Ajoute une heure
  }

  if (digitalRead(BUTTON_MINUTE_PIN) == LOW) {
    adjustTime(now, 60); // Ajoute une minute
  }

  if (alarmSet && now.unixtime() >= alarmTime.unixtime()) {
    soundAlarm();
  }

  displayTime(now);
}

void adjustTime(DateTime currentTime, int secondsToAdd) {   //permet d'eviter les erreurs quand le reglage trop rapide
  DateTime newTime = currentTime + TimeSpan(secondsToAdd);
  rtc.adjust(newTime);
  delay(500); 
}

void displayTime(DateTime currentTime) {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);

  int16_t x, y;
  uint16_t w, h;
  display.getTextBounds("00:00:00", 0, 0, &x, &y, &w, &h);  // Obtient les dimensions du texte
  display.setCursor((SCREEN_WIDTH - w) / 2, (SCREEN_HEIGHT - h) / 2);
  display.print(currentTime.hour(), DEC);
  display.print(':');
  display.print(currentTime.minute(), DEC);
  display.print(':');
  display.print(currentTime.second(), DEC);

  display.display();
  delay(1000);
}

void soundAlarm() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(1000); // Alarme sonne pendant 1 seconde
  digitalWrite(BUZZER_PIN, LOW);
  alarmSet = false; // Réinitialise l'alarme
}
