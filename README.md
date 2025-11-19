# Servo_Zawor_Supla
### Sterowanie zaworem czterodrożnym C.O. przez Supla – Kamil Ból 2025

Zawór czterodrożny (obrotowy) w instalacji C.O. sterowany serwem DS3225-270° przez ESP-01S.  
Całość działa z aplikacją Supla jako kanał „brama wjazdowa” – klikam „otwórz/zamknij” → zawór się obraca.  
Dwa kontaktrony podpięte do Wemos D1 Mini dają 100% pewność stanu w aplikacji.  
Po zaniku zasilania zawsze wraca do pozycji ZAMKNIĘTEJ (obieg grzejniki).

**Co teraz działa:**
- Pierwszy impuls → zawór otwiera (np. na obieg bojlera)
- Drugi impuls → zawór zamyka (powrót na grzejniki)
- Kontaktron potwierdza mechanicznie pozycję zamkniętą
- ESP-01S wystawia na TX: HIGH = zamknięty, LOW = otwarty
- Po restarcie ESP zawsze ustawia serwo na pozycję startową
- Kalibracja pozycji i kąta w dwóch linijkach kodu
- Zero przekaźników, cicha praca, wodoodporne serwo

**Zastosowanie:** przełączanie obiegu C.O. grzejniki / bojler, mieszanie obiegów, automatyka C.O.

Autor: Kamil Ból  
Licencja: MIT – róbta co chceta
