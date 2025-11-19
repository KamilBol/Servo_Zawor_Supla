#include <Arduino.h>          // podstawowa biblioteka Arduino
#include <Servo.h>            // biblioteka do serwa

Servo myServo;                // obiekt serwa

// =============================
// === KALIBRACJA – ZMIENIAJ TU ===
// =============================
const int START_POS    = 90;   // pozycja ZAMKNIĘTY (grzejniki)
const int ROTATE_ANGLE = 88;   // o ile stopni obrócić na otwarty (bojler)
// =============================

#define INPUT_PIN  0     // GPIO0 – impuls z Wemosa + kontaktron zamknięcia
#define SERVO_PIN  2     // GPIO2 – sygnał PWM do serwa
#define TX_STATUS  1     // GPIO1 (TX) – HIGH = zamknięty, LOW = otwarty

bool zaworOtwarte = false;              // aktualny stan logiczny zaworu
unsigned long ostatniImpuls = 0;        // czas ostatniego impulsu
const unsigned long debounce = 300;     // ochrona przed drganiami

void setup() {
  pinMode(INPUT_PIN, INPUT_PULLUP);     // GPIO0 jako wejście z pull-up
  pinMode(TX_STATUS, OUTPUT);           // TX jako wyjście stanu

  myServo.attach(SERVO_PIN);            // podpinam serwo (bez ruchu!)
  
  // === KLUCZOWA ZMIANA ===
  // Sprawdzamy kontaktron – on mówi nam fizyczną pozycję zaworu
  if (digitalRead(INPUT_PIN) == LOW) {   // kontaktron zamknięty = zwiera do GND
    zaworOtwarte = false;                // wiemy że jest zamknięty
    digitalWrite(TX_STATUS, HIGH);       // TX = HIGH → zamknięty
  } else {
    zaworOtwarte = true;                 // inaczej zakładamy że otwarty
    digitalWrite(TX_STATUS, LOW);        // TX = LOW → otwarty
  }
  // KONIEC – serwo się nie rusza po restarcie!
}

void loop() {
  // Reakcja tylko na impuls HIGH z Wemosa (krótki skok do 3.3V)
  if (digitalRead(INPUT_PIN) == HIGH && millis() - ostatniImpuls > debounce) {
    ostatniImpuls = millis();           // zapis czasu

    zaworOtwarte = !zaworOtwarte;       // przełączamy stan logiczny

    if (zaworOtwarte) {
      myServo.write(START_POS + ROTATE_ANGLE);  // jedziemy na bojler
      digitalWrite(TX_STATUS, LOW);              // TX = LOW
    } else {
      myServo.write(START_POS);                  // wracamy na grzejniki
      digitalWrite(TX_STATUS, HIGH);             // TX = HIGH
    }
    delay(400);                         // czekamy aż serwo dojedzie
  }
}
