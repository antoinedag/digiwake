#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Définition des dimensions de l'écran OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Pin de reset pour l'écran OLED, -1 si non utilisé

// Initialisation de l'écran OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialisation du module RTC (Real Time Clock)
RTC_DS1307 rtc;

// Définition des pins pour l'encodeur et le buzzer
const int BUZZER_PIN = 5;
const int ENCODER_PIN_A = 6;
const int ENCODER_PIN_B = 7;
const int ENCODER_BUTTON_PIN = 8;

// Variables pour la gestion de l'encodeur
volatile int encoderPos = 0; // Position actuelle de l'encodeur
int lastEncoderPos = 0; // Dernière position enregistrée de l'encodeur
bool settingHour = false; // Mode de réglage (false pour les minutes, true pour les heures)

// Variables pour la gestion de l'alarme
DateTime alarmTime;
bool alarmActive = false;

// Fonction de lecture de l'encodeur
void readEncoder() {
  static int lastEncoded = 0;
  int MSB = digitalRead(ENCODER_PIN_A); // Bit le plus significatif
  int LSB = digitalRead(ENCODER_PIN_B); // Bit le moins significatif
  int encoded = (MSB << 1) | LSB; // Conversion en nombre
  int sum  = (lastEncoded << 2) | encoded; // Somme des valeurs pour détermination de la direction

  // Incrémentation ou décrémentation de la position de l'encodeur selon la direction
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderPos++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderPos--;

  lastEncoded = encoded; // Mise à jour de la dernière valeur encodée
}

// Fonction pour ajuster l'heure ou les minutes
void adjustTime(DateTime currentTime, int amount) {
  DateTime newTime;
  if (settingHour) {
    newTime = currentTime + TimeSpan(amount * 3600); // Ajuste les heures
  } else {
    newTime = currentTime + TimeSpan(amount * 60); // Ajuste les minutes
  }
  rtc.adjust(newTime);
  alarmTime = newTime; // Mise à jour du moment de l'alarme
  delay(200); // Délai pour éviter le rebond
}

void setup() {
  Serial.begin(9600);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(ENCODER_PIN_A, INPUT);
  pinMode(ENCODER_PIN_B, INPUT);
  pinMode(ENCODER_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), readEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), readEncoder, CHANGE);

  // Initialisation de l'écran OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Boucle infinie en cas d'erreur
  }

  // Initialisation du module RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1); // Boucle infinie en cas d'erreur
  }

  // Vérification si le RTC fonctionne
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__))); // Réglage de l'heure si le RTC ne fonctionne pas
  }

  display.clearDisplay();
  display.display();

  // Configuration initiale de l'heure de l'alarme
  alarmTime = DateTime(2024, 5, 1, 7, 30, 0);
  alarmActive = true; // Activation de l'alarme
}

void loop() {
  DateTime now = rtc.now(); // Lecture de l'heure actuelle

  // Vérification de l'état du bouton de l'encodeur pour basculer entre heures et minutes
  if (digitalRead(ENCODER_BUTTON_PIN) == LOW) {
    settingHour = !settingHour;
    delay(200); // Délai pour éviter le rebond
  }

  // Ajustement de l'heure si la position de l'encodeur a changé
  if (encoderPos != lastEncoderPos) {
    adjustTime(now, encoderPos - lastEncoderPos);
    lastEncoderPos = encoderPos;
  }

  // Déclenchement de l'alarme si l'heure actuelle correspond à l'heure de l'alarme
  if (alarmActive && now >= alarmTime) {
    digitalWrite(BUZZER_PIN, HIGH); // Activation du buzzer
    delay(1000); // Durée de l'alarme
    digitalWrite(BUZZER_PIN, LOW); // Désactivation du buzzer
    alarmActive = false; // Réinitialisation de l'alarme
  }

  // Affichage de l'heure sur l'écran OLED
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print(now.hour());
  display.print(':');
  if (now.minute() < 10) {
    display.print('0'); // Ajout d'un zéro pour les minutes inférieures à 10
  }
  display.print(now.minute());
  display.display();
  delay(100); // Délai pour réduire le scintillement
}
