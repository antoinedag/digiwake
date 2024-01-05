#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ultrasonic.h>
#include <BitBang_I2C.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS1307 rtc;
Ultrasonic ultrasonic(9, 8);
int tempsActuelBuzzer, tempsDepartBuzzer;
float tempsTotalBuzzer;
const int LED_PIN = 3;
const int BUTTON_HOUR_PIN = A2;
const int BUTTON_MINUTE_PIN = A3;
const int BUTTON_ALARM_TOGGLE_PIN = A1;
const int BUZZER_PIN = 3;
int alarm =0;

DateTime alarmTime;
bool alarmActive = false;

void adjustTime(DateTime currentTime, int secondsToAdd) {     
  DateTime newTime = currentTime + TimeSpan(secondsToAdd);
  rtc.adjust(newTime);
  delay(500); // Permet d'éviter les problemmes quand saisie trop rapide avec le delay
}

void showTime(DateTime currentTime) { //permet d'afficher l'heure quand elle est appelée
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(currentTime.hour());
  display.print(':');
  if (currentTime.minute() < 10) {
    display.print('0');
  }
  display.print(currentTime.minute());
  display.display();
}

void soundAlarm() {
  tone(5, 1000);
  tempsDepartBuzzer = millis();
  
  while(alarmActive){
  tempsActuelBuzzer = millis();
  tempsTotalBuzzer = tempsActuelBuzzer - tempsDepartBuzzer;
  
  int dist = ultrasonic.read();
  Serial.print(dist);
  Serial.println(" cm");
  
  if(dist<10 || (tempsTotalBuzzer > 10000)){
    Serial.println(tempsTotalBuzzer/1000);
    alarmActive = false;
    digitalWrite(3, HIGH);
     // Réinitialise l'alarme
  }
  delay(100);
  } 
}

void stopAlarm(){
  if(alarm==1){
    noTone(5);
    alarm=0;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_HOUR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ALARM_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // vérification que le systeme reconnait bien les composants

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {   // NB: Normalement,0x3C c'est l'adresse I2C de l'écran
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  display.clearDisplay();
  display.display();

  alarmTime = DateTime(2024, 5, 1, 7, 30, 0); // Exemple avec un moment d'aujourd'hui
  alarmActive = true; 
  
  pinMode(3, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(3),stopAlarm, CHANGE);
  pinMode(BUZZER_PIN, OUTPUT); 
  digitalWrite(3, LOW);
}

void loop() {
  DateTime now = rtc.now();   // Lire l'heure actuelle
  
  if (digitalRead(BUTTON_HOUR_PIN) == LOW) {
    adjustTime(now, 3600); 
  }
  if (digitalRead(BUTTON_MINUTE_PIN) == LOW) {
    adjustTime(now, 60); 
  }
  if (digitalRead(BUTTON_ALARM_TOGGLE_PIN) == LOW) {
    alarmActive = !alarmActive;
    delay(200); 
  }

  if (alarmActive && now >= alarmTime) {
    alarm = 1;
    soundAlarm();
  }

  // Clignotement de la LED chaque seconde
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  showTime(now);
  delay(100); 
}
