#include <Arduino.h>

#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Déclaration de l'écran OLED

#define SCREEN_WIDTH 128      // Largeur de l'écran OLED en pixels
#define SCREEN_HEIGHT 64      // Hauteur de l'écran OLED en pixels
#define OLED_RESET -1         // Reset l'écran

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Création de l'objet RTC

RTC_DS3231 rtc;

void setup() {

  // Initialisation de la communication I2C

  Wire.begin();

  // Initialisation du RTC

  if (!rtc.begin()) {
    Serial.println("RTC introuvable");
    while (1);
  }

  // Initialisation de l'écran OLED

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // NB: Normalement,0x3C c'est l'adresse I2C de l'écran
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.clearDisplay();
}

void loop() {

  DateTime now = rtc.now();  // Lire l'heure actuelle

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);

  // Affichage de l'heure et des minutes

  display.print(now.hour(), DEC);
  display.print(':');
  if(now.minute() < 10) display.print('0'); // Ajoute un zéro devant pour les minutes < 10
  display.print(now.minute(), DEC);

  display.display();  // Affiche les informations sur l'écran OLED
  delay(1000); // Mise à jour de l'affichage toutes les secondes
}
