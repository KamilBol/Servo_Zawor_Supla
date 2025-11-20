#include <Arduino.h>
#include <Servo.h>
#include <EEPROM.h>

/*
   ===================================================================================
   PROJEKT: STEROWNIK ZAWORU C.O. - WERSJA PLATINUM (FULL STABILIZACJA)
   AUTOR: KAMIL BÓL ("BÓLU")
   
   POPRAWKA:
   - Zastosowano "Ghost Buster" (odszumianie) również dla pinów serwisowych (D6, D7, D0).
   - Koniec z samoistną zmianą kątów przez zakłócenia.
   
   PINOUT (Wemos D1 Mini):
   - D1 (GPIO 5)  -> Serwo
   - D2 (GPIO 4)  -> Trigger (Start)
   - D5 (GPIO 14) -> Kontaktron (Czujnik)
   - D4 (GPIO 2)  -> LED (Info)
   
   PINY SERWISOWE:
   - D6 (GPIO 12) -> Zmniejsz kąt (-5 st.)
   - D7 (GPIO 13) -> Zwiększ kąt (+5 st.)
   - D0 (GPIO 16) -> Odwróć logikę (trzymaj 5s)
   ===================================================================================
*/

Servo myServo; // Mój silnik

// === MOJE USTAWIENIA KALIBRACJI (SG90) ===
int US_ZAMKNIETY = 600;   // 0 stopni
int US_OTWARTY   = 2400;  // 180 stopni (To zmienna, bo D6/D7 ją zmienia)

const int CZAS_RUCHU_MS = 5000; // Ma być wolno (5s)

// === MOJE PINY ===
const int PIN_SERVO   = 5;  // D1
const int PIN_TRIGGER = 4;  // D2
const int PIN_SENSOR  = 14; // D5
const int PIN_LED     = 2;  // D4 (LED na płytce)

// === MOJE PINY SERWISOWE ===
const int PIN_KALIB_MINUS = 12; // D6
const int PIN_KALIB_PLUS  = 13; // D7
const int PIN_LOGIKA_SWAP = 16; // D0 

// === ADRESY W PAMIĘCI EEPROM ===
const int ADRES_MEM     = 0;  // Tu siedzi pozycja serwa
const int ADRES_KOREKTY = 20; // Tu siedzi korekta kąta
const int ADRES_LOGIKI  = 30; // Tu siedzi odwrócona logika

// === ZMIENNE SYSTEMOWE ===
int aktualneMikrosekundy = 0;       
unsigned long czasOstatniejAkcji = 0;
const int CZAS_COOLDOWN = 2000;     
int licznikBledow = 0;              

// Zmienne serwisowe
int korektaKonta = 0;         
bool logikaOdwrocona = false; 
unsigned long czasTrzymaniaD0 = 0; // Licznik dla przycisku logiki

// ===================================================================================
// === FUNKCJA: GHOST BUSTER (ODSZUMIANIE) ===
// ===================================================================================
// Trik na wiszące kable bez rezystorów. Zwieram do masy i sprawdzam czy sygnał wróci.
bool sprawdzPin(int pin) {
  if (digitalRead(pin) == LOW) return false;
  
  pinMode(pin, OUTPUT); 
  digitalWrite(pin, LOW); 
  delayMicroseconds(10); 
  pinMode(pin, INPUT);   
  
  if (digitalRead(pin) == HIGH) return true; // To prawdziwy prąd
  return false; // To był szum
}

// ===================================================================================
// === FUNKCJA: MRUGANIE (DISCO) ===
// ===================================================================================
void mrugnij(int ile, int szybkosc) {
  for(int i=0; i<ile; i++) {
    digitalWrite(PIN_LED, LOW);  delay(szybkosc);
    digitalWrite(PIN_LED, HIGH); delay(szybkosc);
  }
}

// ===================================================================================
// === FUNKCJA: ZAPIS DO PAMIĘCI ===
// ===================================================================================
void zapiszPamiec(int wartosc) {
  EEPROM.put(ADRES_MEM, wartosc);
  EEPROM.commit();
}

// ===================================================================================
// === FUNKCJA SERWISOWA (D6, D7, D0) - TERAZ STABILNA! ===
// ===================================================================================
void sprawdzTrybSerwisowy() {
  
  // --- 1. KALIBRACJA KĄTA (D6 / D7) ---
  // Używam sprawdzPin zamiast digitalRead, żeby wyeliminować zakłócenia!
  
  // D6 -> ZMNIEJSZ KĄT
  if (sprawdzPin(PIN_KALIB_MINUS)) { 
     delay(100); 
     if (sprawdzPin(PIN_KALIB_MINUS)) { // Podwójne sprawdzenie
        korektaKonta -= 50; // Odejmij ~5 stopni
        US_OTWARTY += -50;  // Zmień zakres
        
        Serial.print("!!! SERWIS D6: Zmniejszam kat. Nowy MAX: "); Serial.println(US_OTWARTY);
        mrugnij(1, 500); // 1 długi błysk
        
        EEPROM.put(ADRES_KOREKTY, korektaKonta); EEPROM.commit();
        delay(1000); 
     }
  }
  
  // D7 -> ZWIĘKSZ KĄT
  if (sprawdzPin(PIN_KALIB_PLUS)) {
     delay(100); 
     if (sprawdzPin(PIN_KALIB_PLUS)) {
        korektaKonta += 50; // Dodaj ~5 stopni
        US_OTWARTY += 50;   
        
        Serial.print("!!! SERWIS D7: Zwiekszam kat. Nowy MAX: "); Serial.println(US_OTWARTY);
        mrugnij(2, 150); // 2 szybkie błyski
        
        EEPROM.put(ADRES_KOREKTY, korektaKonta); EEPROM.commit();
        delay(1000);
     }
  }

  // --- 2. ODWRACANIE LOGIKI (D0 - Trzymaj 5s) ---
  if (sprawdzPin(PIN_LOGIKA_SWAP)) { // Tu też Ghost Buster
    if (czasTrzymaniaD0 == 0) czasTrzymaniaD0 = millis();
    
    // Czy trzymam już 5 sekund?
    if (millis() - czasTrzymaniaD0 > 5000) {
       logikaOdwrocona = !logikaOdwrocona; // Zmień flagę
       
       Serial.print("!!! SERWIS D0: ZMIANA LOGIKI !!! Odwrocona: "); Serial.println(logikaOdwrocona);
       mrugnij(5, 50); // DISCO Potwierdzenie
       
       EEPROM.put(ADRES_LOGIKI, logikaOdwrocona); EEPROM.commit();
       
       czasTrzymaniaD0 = 0; 
       delay(3000); 
    }
  } else {
    czasTrzymaniaD0 = 0;
  }
}

// ===================================================================================
// === FUNKCJA RUCHU ===
// ===================================================================================
void wykonajRuch(int celUS) {
  Serial.println("\n----------------------------------------");
  int startUS = aktualneMikrosekundy;
  
  if (celUS == US_OTWARTY) Serial.println("Decyzja: OTWIERAM");
  else Serial.println("Decyzja: ZAMYKAM");
  
  Serial.print("   Start: "); Serial.print(startUS);
  Serial.print(" -> Cel: "); Serial.println(celUS);

  // Jeśli każę mu jechać tam gdzie jest -> Blokada
  if (abs(celUS - startUS) < 50) {
    Serial.println("!!! BLOKADA: Juz tam jestem.");
    return;
  }

  int kroki = abs(celUS - startUS) / 10; 
  if (kroki == 0) kroki = 1;
  int opoznienie = CZAS_RUCHU_MS / kroki;

  // START
  Serial.println("   [Silnik] Start...");
  myServo.writeMicroseconds(startUS); 
  myServo.attach(PIN_SERVO);
  delay(200); 

  // JAZDA
  if (startUS < celUS) {
    for (int i = startUS; i <= celUS; i += 10) {
      myServo.writeMicroseconds(i); delay(opoznienie); yield();
    }
  } else {
    for (int i = startUS; i >= celUS; i -= 10) {
      myServo.writeMicroseconds(i); delay(opoznienie); yield();
    }
  }
  
  // Dociągnij i Zapisz
  myServo.writeMicroseconds(celUS);
  delay(opoznienie);
  
  Serial.println("   [Silnik] Stop. Zapis EEPROM.");
  aktualneMikrosekundy = celUS;
  zapiszPamiec(aktualneMikrosekundy); 
  
  delay(500); 
  myServo.detach();
  licznikBledow = 0; // Reset błędów, bo poszło gładko
  Serial.println("--- KONIEC ---");
}

// ===================================================================================
// === SETUP ===
// ===================================================================================
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512); 
  delay(100);
  Serial.println("\n\n=== WITAJ KAMILU. STABILIZACJA PEŁNA! ===");

  // Ustawiam piny
  pinMode(PIN_LED, OUTPUT); digitalWrite(PIN_LED, HIGH); 
  pinMode(PIN_SERVO, OUTPUT); digitalWrite(PIN_SERVO, LOW);
  pinMode(PIN_TRIGGER, INPUT);
  pinMode(PIN_SENSOR, INPUT);
  
  // Piny serwisowe
  pinMode(PIN_KALIB_MINUS, INPUT);
  pinMode(PIN_KALIB_PLUS, INPUT);
  pinMode(PIN_LOGIKA_SWAP, INPUT); 

  // --- WCZYTANIE USTAWIEŃ SERWISOWYCH ---
  EEPROM.get(ADRES_KOREKTY, korektaKonta);
  if (korektaKonta < -1000 || korektaKonta > 1000) korektaKonta = 0; 
  US_OTWARTY += korektaKonta; // Aplikuję korektę
  Serial.print("[SERWIS] Korekta kata: "); Serial.println(korektaKonta);

  EEPROM.get(ADRES_LOGIKI, logikaOdwrocona);
  if (logikaOdwrocona != 0 && logikaOdwrocona != 1) logikaOdwrocona = false;
  Serial.print("[SERWIS] Logika odwrocona: "); Serial.println(logikaOdwrocona);

  // --- WCZYTANIE POZYCJI SERWA ---
  int pamiec;
  EEPROM.get(ADRES_MEM, pamiec);
  
  if (pamiec < 500 || pamiec > 3000) {
    Serial.println("[PAMIEC] Pusta. Ustawiam wg czujnika.");
    
    bool rawSensor = sprawdzPin(PIN_SENSOR);
    bool logicSensor = (logikaOdwrocona) ? !rawSensor : rawSensor;

    if (logicSensor) pamiec = US_ZAMKNIETY;
    else pamiec = US_OTWARTY;
    zapiszPamiec(pamiec); 
  }
  
  aktualneMikrosekundy = pamiec;
  Serial.print("Pozycja startowa: "); Serial.println(aktualneMikrosekundy);
  
  Serial.println("Czekam 10s na stabilizację...");
  for(int i=0; i<10; i++) { delay(1000); Serial.print("."); }
  Serial.println("\nGOTOWY.");

  // LED START
  if (aktualneMikrosekundy == US_ZAMKNIETY) {
    mrugnij(6, 100); // 6x Zamknięty
  } else {
    mrugnij(3, 100); // 3x Otwarty
  }
  
  licznikBledow = 0; 
}

// ===================================================================================
// === LOOP ===
// ===================================================================================
void loop() {
  
  // 1. SPRAWDŹ SERWIS (Teraz stabilnie przez Ghost Buster!)
  sprawdzTrybSerwisowy();

  // 2. NORMALNA PRACA
  bool trigger = sprawdzPin(PIN_TRIGGER);

  if (trigger) {
    if (millis() - czasOstatniejAkcji > CZAS_COOLDOWN) {
      
      Serial.println("\n!!! DOSTAŁEM IMPULS NA D2 !!!");
      
      // Sygnalizacja LED (1s)
      Serial.println("LED: Potwierdzam odbiór.");
      digitalWrite(PIN_LED, LOW); delay(1000); digitalWrite(PIN_LED, HIGH);
      
      // ODCZYT D5 (Z UWZGLĘDNIENIEM LOGIKI)
      bool sensorFizyczny = sprawdzPin(PIN_SENSOR);
      bool sensorLogiczny = (logikaOdwrocona) ? !sensorFizyczny : sensorFizyczny;
      
      int celUS;
      
      if (sensorLogiczny) {
        Serial.println("Czujnik D5: ZAMKNIETY -> Cel: OTWARCIE");
        celUS = US_OTWARTY; 
      } else {
        Serial.println("Czujnik D5: OTWARTY -> Cel: ZAMKNIECIE");
        celUS = US_ZAMKNIETY;
      }
      
      // WALIDACJA
      if (abs(aktualneMikrosekundy - celUS) < 100) {
        
        // Błąd logiczny (czujnik kłamie)
        licznikBledow++;
        Serial.print("!!! BŁĄD LOGICZNY !!! Licznik: ");
        Serial.print(licznikBledow); Serial.println("/10");
        
        mrugnij(2, 200); // Błąd
        
        if (licznikBledow >= 10) {
          Serial.println(">>> ZA DUŻO BŁĘDÓW. RESTART... <<<");
          delay(1000); ESP.restart();
        }
        
      } else {
        // Wszystko OK
        mrugnij(2, 80); // Potwierdzenie
        wykonajRuch(celUS);
      }
      
      czasOstatniejAkcji = millis();
    }
  }
  delay(50);
}
