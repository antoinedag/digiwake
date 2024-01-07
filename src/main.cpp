#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ultrasonic.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define MEMORY_START 1
byte nombreValeurs=0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
RTC_DS1307 rtc;
Ultrasonic ultrasonic(9, 8);
int tempsActuelBuzzer, tempsDepartBuzzer;
float tempsTotalBuzzer;
const int LED_PIN = 2;
const int BUTTON_HOUR_PIN = A2;
const int BUTTON_MINUTE_PIN = A3;
const int BUTTON_ALARM_TOGGLE_PIN = A1;
const int BUZZER_PIN = 5;
int alarm = 0;
int heureAlarme = 12;
int minuteAlarme = 58;
bool settingMode = false; // false pour le réglage de l'alarme, true pour le réglage de l'heure

DateTime alarmTime;
bool alarmActive = false;

void adjustTime(DateTime currentTime, int secondsToAdd) {
  DateTime newTime = currentTime + TimeSpan(secondsToAdd);
  rtc.adjust(newTime);
}

void showTime(DateTime currentTime) {
  DateTime now = rtc.now();
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.print("R");
  display.setCursor(32, 10);
  display.print(heureAlarme);
  display.print(':');
  display.print(minuteAlarme);
  display.setCursor(5, 40);
  display.print("H");
  display.setCursor(32, 40);
  display.print(now.hour(), DEC);
  display.print(':');
  display.print(now.minute(), DEC);

  display.display();
  delay(1000);
}

void ecrireTempsEcoule(float temps){
  
  nombreValeurs = EEPROM.read(0);
  if(nombreValeurs==255){
    EEPROM.put(MEMORY_START, temps);
    nombreValeurs++;
    EEPROM.update(0, nombreValeurs);
  }else{
    EEPROM.put(4*nombreValeurs+MEMORY_START,temps);
    nombreValeurs++;
    EEPROM.update(0, nombreValeurs);
  }

}
void AfficherTempsEcoule(){
int valeurs;
nombreValeurs = EEPROM.read(0);
for (int i = nombreValeurs-10; i < nombreValeurs; i++)
{
  EEPROM.get(4*i+MEMORY_START, valeurs);
  Serial.print(valeurs);
}
}


void soundAlarm() {
  tone(BUZZER_PIN, 1000);
  tempsDepartBuzzer = millis();

  while (alarmActive) {
    tempsActuelBuzzer = millis();
    tempsTotalBuzzer = tempsActuelBuzzer - tempsDepartBuzzer;

    int dist = ultrasonic.read();
    Serial.print(dist);
    Serial.println(" cm");

    if (dist < 10 || (tempsTotalBuzzer > 10000)) {
      ecrireTempsEcoule(tempsTotalBuzzer/1000);
      Serial.println(tempsTotalBuzzer / 1000);
      alarmActive = false;
      noTone(BUZZER_PIN);
      digitalWrite(LED_PIN, HIGH);
    }
    delay(100);
  }
}

void stopAlarm() {
  if (alarm == 1) {
    noTone(BUZZER_PIN);
    alarm = 0;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_HOUR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ALARM_TOGGLE_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  display.clearDisplay();
  display.display();

  alarmTime = DateTime(2024, 5, 1, 7, 30, 0);
  alarmActive = true;

  pinMode(3, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(3), stopAlarm, CHANGE);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  DateTime now = rtc.now();

  if (digitalRead(BUTTON_ALARM_TOGGLE_PIN) == LOW) {
    settingMode = !settingMode;
    delay(200); 
  }

  if (settingMode) {
    if (digitalRead(BUTTON_HOUR_PIN) == HIGH) {
      adjustTime(now, 3600); 
      delay(200); 
    }
    if (digitalRead(BUTTON_MINUTE_PIN) == HIGH) {
      adjustTime(now, 60);
      delay(200); 
    }
  } else {
    if (digitalRead(BUTTON_HOUR_PIN) == HIGH) {
      heureAlarme = (heureAlarme + 1) % 24;
      delay(200); 
    }
    if (digitalRead(BUTTON_MINUTE_PIN) == HIGH) {
      minuteAlarme = (minuteAlarme + 1) % 60;
      delay(200); 
    }
  }

  if (heureAlarme == now.hour() && minuteAlarme == now.minute()) {
    alarm = 1;
    soundAlarm();
  }

  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);

  showTime(now);
}
