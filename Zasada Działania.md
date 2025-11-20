START SYSTEMU
   |
   v
CZY PAMIĘĆ EEPROM JEST PUSTA?
   |-- TAK --> Ustaw pozycję wg czujnika D5 -> Zapisz -> STOP
   |-- NIE --> Wczytaj ostatnią pozycję -> STOP (Czekaj na sygnał)

OCZEKIWANIE NA IMPULS (D2)
   |
   v
WYKRYTO SYGNAŁ 3.3V NA D2?
   |-- NIE --> Czekaj dalej...
   |-- TAK --> SPRAWDŹ CZUJNIK D5 (Kontaktron)
                  |
                  |-- D5 = HI (ZAMKNIĘTY) --> CEL: OTWÓRZ (Jazda do 2400us)
                  |-- D5 = LO (OTWARTY)   --> CEL: ZAMKNIJ (Jazda do 600us)
                  |
                  v
       CZY CEL JEST INNY NIŻ OBECNA POZYCJA?
          |-- NIE (Błąd) --> Mrugaj LED (Błąd) -> Licznik Błędów +1
          |                  (Jeśli 10 błędów -> RESTART WEMOSA)
          |
          |-- TAK (OK) --> WYKONAJ RUCH (5 sekund)
                             |
                             v
                           ZAPISZ NOWĄ POZYCJĘ W EEPROM
                             |
                             v
                           BLOKADA 2 SEKUNDY (Cooldown)
