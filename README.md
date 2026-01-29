
# Servo_Zawor_Supla (Wemos Edition)
### Sterownik zaworu C.O. na Wemos D1 Mini + SG90
**Autor:** Kamil Ból ("Bólu")

Projekt sterownika do zaworu kulowego (lub innego elementu C.O.), opartego na serwomechanizmie modelarskim (np. SG90) i mikrokontrolerze Wemos D1 Mini. Układ działa jako element wykonawczy dla systemu SUPLA (lub innej automatyki), reagując na impuls sterujący.

### 🔥 Główne Funkcje (Bólu Soft v3.0):
* **Sterowanie Mikrosekundami:** Pełna płynność i precyzja dla serw SG90/MG90.
* **Ghost Buster:** Autorski algorytm odszumiania pinów (działa stabilnie bez zewnętrznych rezystorów!).
* **Pamięć EEPROM:** Po zaniku prądu sterownik pamięta, gdzie skończył. Zero szarpania przy starcie.
* **Safety Mode:** Jeśli czujnik (kontaktron) ulegnie awarii, system to wykryje i po 10 próbach zresetuje się, zamiast niszczyć zawór.
* **Tryb Serwisowy:** Możliwość kalibracji kątów i odwracania logiki przyciskami - bez podłączania komputera!

---

### 🛠️ Hardware (Co potrzebujesz)
1.  **Wemos D1 Mini** (ESP8266)
2.  **Serwo SG90** (lub MG996R - wymaga zmiany czasów w kodzie)
3.  **Kontaktron** (Czujnik otwarcia/zamknięcia)
4.  **Drugi moduł Supla** (jako wyzwalacz)
5.  Zasilacz 5V (stabilny!)

---

### 🔌 Pinout (Podłączenie)
Szczegółowy schemat znajdziesz w folderze `/doc`.

| PIN Wemos | Funkcja | Opis |
| :--- | :--- | :--- |
| **D1 (GPIO 5)** | **SERWO** | Sygnał PWM (Żółty/Pomarańczowy kabel) |
| **D2 (GPIO 4)** | **TRIGGER** | Impuls 3.3V (HI) z Supli uruchamia ruch |
| **D5 (GPIO 14)** | **CZUJNIK** | Kontaktron (HI 3.3V = Zamknięty) |
| **D4 (GPIO 2)** | **LED** | Wbudowana dioda sygnalizacyjna |
| **D6 (GPIO 12)** | *Serwis (-)* | Zmniejszanie kąta otwarcia |
| **D7 (GPIO 13)** | *Serwis (+)* | Zwiększanie kąta otwarcia |
| **D0 (GPIO 16)** | *Serwis (L)* | Odwracanie logiki czujnika (trzymać 5s) |

---

### 🚀 Jak to działa?
1.  Wemos czeka na impuls **3.3V (HI)** na pinie **D2**.
2.  Po otrzymaniu impulsu sprawdza stan czujnika na **D5**.
3.  Jeśli czujnik pokazuje "Zamknięty" -> Serwo otwiera zawór.
4.  Jeśli czujnik pokazuje "Otwarty" -> Serwo zamyka zawór.
5.  Ruch trwa 5 sekund (płynny start i stop).
6.  Pozycja zapisywana jest w pamięci stałej.

---

### 📜 Licencja
Róbta co chceta (MIT). Kod jest dla ludzi.
https://github.com/KamilBol/Servo_Zawor_Supla/blob/main/3D/Zrzut%20ekranu%202026-01-29%20003321.png?raw=true

