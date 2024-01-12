#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Ultrasonic.h>
#include <EEPROM.h>

#include <Encoder.h>



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
const int BUTTON_HOUR_PIN = A3;
const int BUTTON_MINUTE_PIN = A2;
const int BUTTON_ALARM_TOGGLE_PIN = A1;
const int STOP_BUTTON = 12;
const int BUTTON_FORMAT_TOGGLE_PIN = 4;  // Nouveau bouton pour le changement de format d'heure
const int BUZZER_PIN = 5;
int alarm = 0;
bool menu = true;
bool reveilActif = true;

int heureAlarme = 15;
int minuteAlarme = 40;
bool settingMode = false;
bool isNumberDisplayed = false;
int displayedNumber = 0;
bool is24HourFormat = true;  // Variable pour stocker le format d'heure
const int ENCODER_CLK_PIN = 4; 
const int ENCODER_DT_PIN = 5;  
Encoder myEncoder(ENCODER_CLK_PIN, ENCODER_DT_PIN);
long oldPosition  = -999;
DateTime alarmTime;
bool alarmActive = false;
bool heure = true;
bool buzzer = true;

void clearEEPROM() {
  int EEPROMSize = EEPROM.length();
  for (int i = 0; i < EEPROMSize; i++) {
    EEPROM.write(i, 0);
  }
}
void toggleTimeFormat() {
  is24HourFormat = !is24HourFormat;
}
void playMelody() {
  int melody[] = {262, 330, 392, 523, 659, 784, 1046};  // Exemple de fréquences
  int noteDuration = 500;  // Durée de chaque note en millisecondes

  for (int i = 0; i < 7; i++) {
    tone(BUZZER_PIN, melody[i], noteDuration);
    delay(noteDuration + 50);  // Pause entre les notes
  }
}
void activateAlarm(){
  alarmActive=!alarmActive;
}
void adjustTime(DateTime currentTime, int secondsToAdd) {
  DateTime newTime = currentTime + TimeSpan(secondsToAdd);
  rtc.adjust(newTime);
}

void showTime(DateTime currentTime) {
  DateTime now = rtc.now();
  display.clearDisplay();
  display.setTextSize(1.5);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 10);
  display.print("H ");

  if (is24HourFormat) {
    display.print(currentTime.hour(), DEC);
  } else {
    int hour = currentTime.hour() % 12;
    if (hour == 0) hour = 12;
    display.print(hour);
  }
  display.print(':');
  if (currentTime.minute() < 10) {
    display.print('0');
  }
  display.print(currentTime.minute());

  if (is24HourFormat) {
    display.print(" (24h)");
  } else {
    if (currentTime.hour() < 12) {
      display.print(" AM");
    } else {
      display.print(" PM");
    }
  }
  if(alarmActive){
  display.setCursor(100, 24);
  display.print("A");
  }

  display.setCursor(5, 40);
  display.print("R ");
  if (is24HourFormat) {
    display.print(heureAlarme, DEC);
  } else {
    int hour = heureAlarme % 12;
    if (hour == 0) hour = 12;
    display.print(hour);
  }
  display.print(':');
  if (minuteAlarme < 10) {
    display.print('0');
  }
  display.print(minuteAlarme);

  display.display();
}



void ecrireTempsEcoule(float temps){
  nombreValeurs = EEPROM.read(0);
  if(nombreValeurs==255){
    EEPROM.put(MEMORY_START, temps);
    nombreValeurs++;
    EEPROM.update(0, nombreValeurs);
  } else {
    EEPROM.put(4*nombreValeurs+MEMORY_START,temps);
    nombreValeurs++;
    EEPROM.update(0, nombreValeurs);
  }
}

void AfficherTempsEcoule(){
  float valeurs;
  nombreValeurs = EEPROM.read(0);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  for (int i = nombreValeurs-5; i < nombreValeurs; i++)
  {
    EEPROM.get(4*i+MEMORY_START, valeurs);
    Serial.println(valeurs);
    display.println(valeurs, DEC);
    display.display();
  }
}
void stopBuzzer(){
  ecrireTempsEcoule(tempsTotalBuzzer/1000);
    Serial.println(tempsTotalBuzzer / 1000);
    alarmActive = false;
    noTone(BUZZER_PIN);
    alarm = 0;
}
void soundAlarm() {
  
  tempsDepartBuzzer = millis();

  while (alarmActive) {
    tone(5,1000);
    tempsActuelBuzzer = millis();
    tempsTotalBuzzer = tempsActuelBuzzer - tempsDepartBuzzer;

    int dist = ultrasonic.read();
    Serial.print(dist);
    Serial.println(" cm");

    if (dist<10 || digitalRead(STOP_BUTTON)==HIGH || (tempsTotalBuzzer > 10000)) {
      buzzer = !buzzer;
      digitalWrite(3, buzzer);
    }
    delay(100);
  }
}

void showMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("1 - Changer l'heure");
  display.println("2 - Changer le format d'heure");
  display.println("3 - Afficher temps d'extinction reveil");
  display.println("4- Effacer la memoire");
  display.display();
  while (!(Serial.available()))
  {
    delay(100);
  }
  
  int choix = Serial.parseInt();
    switch (choix) {
      case 1:
        Serial.println("Choix 1 - Heure");
        // Faites ce que vous voulez avec ce choix ici
        menu = false;
        break;
      case 2:
        Serial.println("Choix 2 - Changer le format d'heure");
        toggleTimeFormat();
        menu = false;
        break;
      case 3:
        Serial.println("Choix 3 - Changer le temps d'extinction du réveil");
        AfficherTempsEcoule() ;
        delay(5000);       
        menu = false;
        break;
        case 4:
        clearEEPROM();
    delay(1000);
        menu = false;
        break;
        
      default:
        Serial.println("Choix non valide");
        break;
    
  }
  menu = false;
  
}

void setup() {
  //clearEEPROM();
  Serial.begin(9600);
  pinMode(LED_PIN, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(BUTTON_HOUR_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MINUTE_PIN, INPUT_PULLUP);
  pinMode(BUTTON_ALARM_TOGGLE_PIN, INPUT);
  pinMode(BUTTON_FORMAT_TOGGLE_PIN, INPUT_PULLUP);  // Nouveau bouton pour le changement de format d'heure
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STOP_BUTTON, INPUT);
  attachInterrupt(digitalPinToInterrupt(3), stopBuzzer, CHANGE);
  attachInterrupt(digitalPinToInterrupt(2), activateAlarm, CHANGE);
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
  oldPosition = myEncoder.read();


  pinMode(3, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
}


void loop() {
  if(menu){
    showMenu();
  }
  DateTime now = rtc.now();
  int buttonState = digitalRead(BUTTON_ALARM_TOGGLE_PIN);

  if (digitalRead(BUTTON_ALARM_TOGGLE_PIN) == HIGH) {
    settingMode = !settingMode;
    delay(200); 
  }

  if (digitalRead(BUTTON_FORMAT_TOGGLE_PIN) == HIGH) {
    menu = !menu;
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
  if(digitalRead(7)==HIGH){
    heure = !heure;
  }

/*long newPosition = myEncoder.read();
  
  if (newPosition != oldPosition) {
    oldPosition = newPosition;

    if (settingMode) {
      if (heure) {
        // Ajuster l'heure du réveil en fonction de la position de l'encodeur
        heureAlarme = (heureAlarme + newPosition) % 24;
      } else {
        // Ajuster les minutes du réveil en fonction de la position de l'encodeur
        minuteAlarme = (minuteAlarme + newPosition) % 60;
      }
    } else {
      if (heure) {
        // Ajuster l'heure de l'horloge en fonction de la position de l'encodeur
        adjustTime(now, 3600 * newPosition);
      } else {
        // Ajuster les minutes de l'horloge en fonction de la position de l'encodeur
        adjustTime(now, 60 * newPosition);
      }
    }

    delay(200);  // Ajouter un délai si nécessaire
  }*/



    if(digitalRead(STOP_BUTTON)==HIGH){
      reveilActif = !reveilActif;
      digitalWrite(2, reveilActif);
      delay(200);
    }
  if (heureAlarme == now.hour() && minuteAlarme == now.minute()) {
    alarm = 1;
    soundAlarm();
  }

  //digitalWrite(LED_PIN, HIGH);
  //delay(500);
  //digitalWrite(LED_PIN, LOW);
  //delay(500);

  showTime(now);
}
