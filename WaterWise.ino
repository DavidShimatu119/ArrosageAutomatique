//inclure les bibliotheques utiles
#include <Wire.h>

#include <LiquidCrystal_I2C.h>

#include <DHT.h>
#include <DHT_U.h>


// Pins pour les composants
const int HUMIDITE_MIN = 0;
const int HUMIDITE_MAX = 1023;
const int PIN_HUMIDITE = A0;
const int POMPE = 4;

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Modes de culture
enum CultureMode { HARICOTS, MAIS, TOMATES };
CultureMode mode = HARICOTS;

// Boutons pour changer de mode et réinitialiser
const int BUTTON_HARICOTS = 6;
const int BUTTON_MAIS = 7;
const int BUTTON_TOMATES = 8;
const int BUTTON_RESET = 9;

// Debounce pour les boutons
unsigned long lastDebounceTimeHaricots = 0;
unsigned long lastDebounceTimeMais = 0;
unsigned long lastDebounceTimeTomates = 0;
unsigned long lastDebounceTimeReset = 0;
const unsigned long debounceDelay = 50;

// Seuils d'humidité du sol pour chaque culture
const int HUMIDITE_BASSE_HARICOTS = 30;
const int HUMIDITE_HAUTE_HARICOTS = 70;
const int HUMIDITE_BASSE_MAIS = 20;
const int HUMIDITE_HAUTE_MAIS = 60;
const int HUMIDITE_BASSE_TOMATES = 40;
const int HUMIDITE_HAUTE_TOMATES = 80;

// Seuils de température et d'humidité de l'air pour chaque culture
const int TEMP_BASSE_HARICOTS = 18;
const int TEMP_HAUTE_HARICOTS = 25;
const int HUMID_AIR_BASSE_HARICOTS = 40;
const int HUMID_AIR_HAUTE_HARICOTS = 60;

const int TEMP_BASSE_MAIS = 20;
const int TEMP_HAUTE_MAIS = 30;
const int HUMID_AIR_BASSE_MAIS = 50;
const int HUMID_AIR_HAUTE_MAIS = 70;

const int TEMP_BASSE_TOMATES = 22;
const int TEMP_HAUTE_TOMATES = 28;
const int HUMID_AIR_BASSE_TOMATES = 60;
const int HUMID_AIR_HAUTE_TOMATES = 80;

void setup() {
  Serial.begin(9600);
  dht.begin();
  lcd.begin(16, 2);
  lcd.backlight();

  pinMode(POMPE, OUTPUT);
  pinMode(BUTTON_HARICOTS, INPUT_PULLUP);
  pinMode(BUTTON_MAIS, INPUT_PULLUP);
  pinMode(BUTTON_TOMATES, INPUT_PULLUP);
  pinMode(BUTTON_RESET, INPUT_PULLUP);

  displayOptions();
}

void loop() {
  debounceButton(BUTTON_HARICOTS, HARICOTS, lastDebounceTimeHaricots);
  debounceButton(BUTTON_MAIS, MAIS, lastDebounceTimeMais);
  debounceButton(BUTTON_TOMATES, TOMATES, lastDebounceTimeTomates);
  debounceResetButton(BUTTON_RESET, lastDebounceTimeReset);

  int soilMoistureValue = analogRead(PIN_HUMIDITE);
  int soilMoisturePercent = map(soilMoistureValue, HUMIDITE_MIN, HUMIDITE_MAX, 0, 100);
  
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Erreur de lecture du capteur DHT.");
    lcd.setCursor(0, 1);
    lcd.print("Erreur DHT");
  } else {
    Serial.print("Température: ");
    Serial.print(temperature);
    Serial.print(" °C, Humidité: ");
    Serial.print(humidity);
    Serial.println(" %");

    lcd.setCursor(0, 1);
    lcd.print("Temp: ");
    lcd.print(temperature);
    lcd.print("C Hum: ");
    lcd.print(humidity);
    lcd.print("%");

    checkAmbientConditions(temperature, humidity);
  }

  controlHumidity(soilMoisturePercent);

  delay(1000);
}
//fonction de control de l'ecran
void displayOptions() {
  lcd.clear();
  lcd.print("1. Haricots");
  lcd.setCursor(0, 1);
  lcd.print("2. Mais");
  lcd.setCursor(8, 1);
  lcd.print("3. Tomates");
}
//fonction du debounce
void debounceButton(int buttonPin, CultureMode newMode, unsigned long &lastDebounceTime) {
  int reading = digitalRead(buttonPin);

  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    mode = newMode;
    lcd.clear();
    lcd.print("MODE ");
    if (newMode == HARICOTS) {
      lcd.print("HARICOTS");
      Serial.println("Mode Haricots sélectionné.");
    } else if (newMode == MAIS) {
      lcd.print("MAIS");
      Serial.println("Mode Mais sélectionné.");
    } else if (newMode == TOMATES) {
      lcd.print("TOMATES");
      Serial.println("Mode Tomates sélectionné.");
    }
    delay(1000);  
    lastDebounceTime = millis();
  }
}

void debounceResetButton(int buttonPin, unsigned long &lastDebounceTime) {
  int reading = digitalRead(buttonPin);

  if (reading == LOW && (millis() - lastDebounceTime) > debounceDelay) {
    resetInterface();
    lastDebounceTime = millis();
  }
}
//pour la reinitialisation de l'interface
void resetInterface() {
  lcd.clear();
  displayOptions();
  Serial.println("Interface réinitialisée.");
}
//control de l'humidite du sol
void controlHumidity(int soilMoisturePercent) {
  int humiditeBasse, humiditeHaute;

  switch (mode) {
    case HARICOTS:
      humiditeBasse = HUMIDITE_BASSE_HARICOTS;
      humiditeHaute = HUMIDITE_HAUTE_HARICOTS;
      break;
    case MAIS:
      humiditeBasse = HUMIDITE_BASSE_MAIS;
      humiditeHaute = HUMIDITE_HAUTE_MAIS;
      break;
    case TOMATES:
      humiditeBasse = HUMIDITE_BASSE_TOMATES;
      humiditeHaute = HUMIDITE_HAUTE_TOMATES;
      break;
  }

  if (soilMoisturePercent < humiditeBasse) {
    digitalWrite(POMPE, HIGH);
    Serial.println("Humidité du sol basse. Pompe activée.");
  } else if (soilMoisturePercent > humiditeHaute) {
    digitalWrite(POMPE, LOW);
    Serial.println("Humidité du sol élevée. Pompe désactivée.");
  }
}
//control de la temperature et l'humidite de l'air
void checkAmbientConditions(float temperature, float humidity) {
  int tempBasse, tempHaute, humidAirBasse, humidAirHaute;

  switch (mode) {
    case HARICOTS:
      tempBasse = TEMP_BASSE_HARICOTS;
      tempHaute = TEMP_HAUTE_HARICOTS;
      humidAirBasse = HUMID_AIR_BASSE_HARICOTS;
      humidAirHaute = HUMID_AIR_HAUTE_HARICOTS;
      break;
    case MAIS:
      tempBasse = TEMP_BASSE_MAIS;
      tempHaute = TEMP_HAUTE_MAIS;
      humidAirBasse = HUMID_AIR_BASSE_MAIS;
      humidAirHaute = HUMID_AIR_HAUTE_MAIS;
      break;
    case TOMATES:
      tempBasse = TEMP_BASSE_TOMATES;
      tempHaute = TEMP_HAUTE_TOMATES;
      humidAirBasse = HUMID_AIR_BASSE_TOMATES;
      humidAirHaute = HUMID_AIR_HAUTE_TOMATES;
      break;
  }

  if (temperature < tempBasse || temperature > tempHaute) {
    Serial.println("Température hors des seuils !");
    lcd.setCursor(0, 1);
    lcd.print("Temp hors seuil");
  } else {
    Serial.println("Température OK.");
  }

  if (humidity < humidAirBasse || humidity > humidAirHaute) {
    Serial.println("Humidité de l'air hors des seuils !");
    lcd.setCursor(0, 1);
    lcd.print("Humid air hors seuil");
  } else {
    Serial.println("Humidité de l'air OK.");
  }
}



