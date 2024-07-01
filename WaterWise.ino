#include <DHT.h>
#include <DHT_U.h>

//bibliotheques

#include <SoftwareSerial.h>

SoftwareSerial Bluetooth(10, 11);

//pins
const int HUMIDITE_MIN= 0;
const int HUMIDITE_MAX = 1023;
const int PIN_HUMIDITE = A0;
const int LED_HUMIDITE_BASSE = 2;
const int LED_HUMIDITE_HAUTE = 3;
const int POMPE = 4;

#define DHTPIN 5
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

enum CultureMode { HARICOTS, MAIS, TOMATES };
CultureMode mode = HARICOTS;

void displayOptions() {
  Serial.println("Sélectionner le mode de culture :");
  Serial.println("1: Haricots");
  Serial.println("2: Mais");
  Serial.println("3: Tomates");
  Serial.println("4: Réinitialiser l'interface de choix");
  Bluetooth.println("1 : Haricots");
  Bluetooth.println("2 : Mais");
  Bluetooth.println("3 : Tomates"); 
  Bluetooth.println("4 : Réinitialiser l'interface de choix"); 
  Bluetooth.println("Sélectionner le mode de culture :");
}
void handleSelection(char selection){
  switch (selection){
      case '1' :
      mode = HARICOTS;
      Serial.println("MODE HARICOTS SELECTIONNE.");
      
      Bluetooth.println("MODE HARICOTS SELECTIONNE.");
      break;
      case '2' :
      mode = MAIS;
      Serial.println("MODE MAIS SELECIONNE");
      Bluetooth.println("MODE MAIS SELECIONNE");
      break;
      case '3' :
      mode = TOMATES;
      Serial.println("MODE TOMATES SELECTIONNE");
      Bluetooth.println("MODE TOMATES SELECTIONNE");
      break;
      case '4' :
  
      Serial.println("Réinitialisation de l'interface de choix ");
      
      Bluetooth.println("Réinitialisation de l'interface de choix ");
      displayOptions();
      break;
      default:
        Serial.println("Seletion invalide.");
        Bluetooth.println("Seletion invalide.");
      break;
    }
}
void setup() {
//moniteur serie 
Serial.begin(9600);
dht.begin();
//
pinMode(LED_HUMIDITE_BASSE,OUTPUT);
pinMode(LED_HUMIDITE_HAUTE,OUTPUT);
pinMode(POMPE, OUTPUT);
delay(3000);
displayOptions();
pinMode(11, OUTPUT);
pinMode(10, INPUT);


}

void loop() {
  if (Serial.available()){
    char selection = Serial.read();
    handleSelection(selection);
  }
  if (Bluetooth.available()){
    char selection = Bluetooth.read();
    handleSelection(selection);
  }
  
  int soilMoistureValue = analogRead(PIN_HUMIDITE);
  int soilMoisturePercent = map(soilMoistureValue, HUMIDITE_MIN, HUMIDITE_MAX, 0, 100);
  Serial.print("l'humidité du sol est :");
  Serial.print(soilMoisturePercent);
  Serial.println("%");
  Bluetooth.print("Humidité du sol :");
  Bluetooth.print(soilMoisturePercent);
  Bluetooth.print("%");
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(t) || isnan(t)) {
    Serial.println("Erreur de lecture du capteur DHT!");
    Bluetooth.print("Erreur de lecture du capteur DHT!");
    return;
  }
   Serial.print("Humidite de l'air :");
   Serial.print(h);
   Serial.println(" %");
   Bluetooth.print("Humidité de l'air :");
   Bluetooth.print(h);
   Bluetooth.print(" %");
   Serial.print("Temperature :");
   Serial.print(t);
   Serial.println(" *C");
   Bluetooth.print("Temperature :");
   Bluetooth.print(t);
   Bluetooth.println(" *C");

   int solSeuilMin, solSeuilMax, airSeuilMin, airSeuilMax, tempSeuilMin, tempSeuilMax;

    switch (mode) {
      case HARICOTS:
         solSeuilMin = 45;
         solSeuilMax = 55;
         airSeuilMin = 50;
         airSeuilMax = 70;
         tempSeuilMin = 18;
         tempSeuilMax = 24;
         break;
       case MAIS:
         solSeuilMin = 40;
         solSeuilMax = 60;
         airSeuilMin = 50;
         airSeuilMax = 70;
         tempSeuilMin = 18;
         tempSeuilMax = 24;
         break;
       case TOMATES:
         solSeuilMin = 60;
         solSeuilMax = 80;
         airSeuilMin = 60;
         airSeuilMax = 80;
         tempSeuilMin = 20;
         tempSeuilMax = 27;
         break;  
    }
  if (soilMoisturePercent <solSeuilMin )
{
    Serial.println("L'humidité du sol est hors du seuil optimal");
    Bluetooth.println("L'humidité du sol est hors du seuil optimal");

    digitalWrite(LED_HUMIDITE_BASSE, HIGH);
    digitalWrite(LED_HUMIDITE_HAUTE, LOW);
    digitalWrite(POMPE, LOW);
  }else {
     digitalWrite(LED_HUMIDITE_BASSE, LOW);
    digitalWrite(LED_HUMIDITE_HAUTE, HIGH);
    digitalWrite(POMPE, HIGH);
  }
  if (h < airSeuilMin || h > airSeuilMax ){
    Serial.println("L'humidité est hors du seuil optimal.");
    Bluetooth.println("L'humidité est hors du seuil optimal.");
    digitalWrite(LED_HUMIDITE_BASSE, HIGH);
    digitalWrite(LED_HUMIDITE_HAUTE, LOW);
    digitalWrite(POMPE, LOW);
  }
  else {
     digitalWrite(LED_HUMIDITE_BASSE, LOW);
    digitalWrite(LED_HUMIDITE_HAUTE, HIGH);
    digitalWrite(POMPE, HIGH);
  }
  if (t < tempSeuilMin || t > tempSeuilMax ){
    Serial.println("La temperature est hors du seuil optimal.");
    Bluetooth.println("La temperature est hors du seuil optimal.");
    digitalWrite(LED_HUMIDITE_BASSE, HIGH);
    digitalWrite(LED_HUMIDITE_HAUTE, LOW);
    digitalWrite(POMPE, LOW);
  }
   else {
     digitalWrite(LED_HUMIDITE_BASSE, LOW);
    digitalWrite(LED_HUMIDITE_HAUTE, HIGH);
    digitalWrite(POMPE, HIGH);
  delay(6000);
   }
}



