#include <Arduino.h>          // Biblioteka podstawowa Arduino
#include <Servo.h>            // Biblioteka do sterowania serwem

Servo myServo;                // Tworzę obiekt serwa

// =============================
// === MIEJSCE NA KALIBRACJĘ ===
// =============================
int START_POS    = 90;   // <<< ZMIENIASZ TU – pozycja ZAMKNIĘTY (grzejniki)
int ROTATE_ANGLE = 88;   // <<< ZMIENIASZ TU – o ile stopni ma się obrócić (bojler)
// =============================

#define INPUT_PIN  0     // GPIO0 – tu przychodzi impuls z Wemosa + ewentualnie kontaktron
#define SERVO_PIN  2     // GPIO2 – sygnał PWM do serwa
#define TX_STATUS  1     // GPIO1 (TX) – HIGH = zawór zamknięty, LOW = otwarty

bool zaworOtwarte = false;              // false = zamknięty (grzejniki), true = otwarty (bojler)
unsigned long ostatniImpuls = 0;        // czas ostatniego przełączenia
const unsigned long debounce = 300;     // ochrona przed drganiami przycisku (300ms)

void setup() {
  pinMode(INPUT_PIN, INPUT_PULLUP);     // GPIO0 jako wejście z podciągnięciem do 3.3V
  pinMode(TX_STATUS, OUTPUT);           // TX jako wyjście sygnału stanu
  digitalWrite(TX_STATUS, HIGH);        // na starcie informujemy że zamknięty

  myServo.attach(SERVO_PIN);            // podpinam serwo do GPIO2
  myServo.write(START_POS);             // zawsze po włączeniu wracam na pozycję ZAMKNIĘTY
  delay(600);                           // czekam aż serwo dojedzie
}

void loop() {
  // Sprawdzam czy na GPIO0 pojawił się impuls HIGH i minęło wystarczająco czasu od ostatniego
  if (digitalRead(INPUT_PIN) == HIGH && millis() - ostatniImpuls > debounce) {
    ostatniImpuls = millis();           // zapisuję czas impulsu

    zaworOtwarte = !zaworOtwarte;       // przełączam stan zaworu

    if (zaworOtwarte) {
      // OTWieram zawór (przełączam na bojler)
      myServo.write(START_POS + ROTATE_ANGLE);
      digitalWrite(TX_STATUS, LOW);     // informuję że już nie jest zamknięty
    } else {
      // ZAMYKam zawór (powrót na grzejniki)
      myServo.write(START_POS);
      digitalWrite(TX_STATUS, HIGH);    // informuję że jest zamknięty
    }
    delay(300);                         // krótka stabilizacja ruchu
  }
}
