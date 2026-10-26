# 🚦 Inteligentne sterowanie sygnalizacją świetlną z wykorzystaniem czujników PIR

## 📌 Opis projektu

Projekt przedstawia system sterowania czterema sygnalizatorami świetlnymi z wykorzystaniem mikrokontrolera AVR obsługiwanego przez środowisko Arduino.

System wykorzystuje cztery czujniki PIR, które wykrywają ruch i zgłaszają zapotrzebowanie na zielone światło dla odpowiedniego kierunku. Zgłoszenia są obsługiwane za pomocą kolejki FIFO (First In, First Out), dzięki czemu kierunki otrzymują zielone światło w kolejności zgłoszeń.

Wykrywanie zmian stanu czujników odbywa się z wykorzystaniem przerwań sprzętowych typu Pin Change Interrupt (PCINT). Dzięki temu mikrokontroler nie musi stale odczytywać każdego czujnika w głównej pętli programu.

## 🎯 Cele projektu

- Obsługa czterech niezależnych kierunków ruchu.
- Wykrywanie ruchu za pomocą czujników PIR.
- Wykorzystanie przerwań sprzętowych do rejestrowania zgłoszeń.
- Przechowywanie zgłoszeń w kolejce FIFO.
- Zapewnienie, że w danej chwili tylko jeden kierunek otrzymuje zielone światło.
- Automatyczne przełączanie sygnalizatorów po upływie określonego czasu.

## 🛠️ Wykorzystane komponenty

- Mikrokontroler AVR, np. ATmega328P.
- Płytka kompatybilna z Arduino, np. Arduino Uno.
- 4 czujniki ruchu PIR.
- 4 czerwone diody LED.
- 4 zielone diody LED.
- Rezystory ograniczające prąd diod LED.
- Przewody połączeniowe.
- Płytka stykowa lub inny układ montażowy.

**Uwaga:** Program wykorzystuje rejestry `PINB`, `PCMSK0`, `PCIFR` i `PCICR`, a także wektor `PCINT0_vect`. Jest więc przeznaczony dla mikrokontrolerów AVR obsługujących tę konfigurację rejestrów, np. ATmega328P. Nie jest bezpośrednio przenośny na wszystkie płytki Arduino.

## 🔌 Konfiguracja pinów

### Sygnalizatory świetlne

| Kierunek | Czerwone światło | Zielone światło |
| --- | --- | --- |
| Kierunek 1 | D2 | D6 |
| Kierunek 2 | D3 | D7 |
| Kierunek 3 | D4 | D8 |
| Kierunek 4 | D5 | D9 |

### Czujniki PIR

| Czujnik | Pin Arduino | Wejście mikrokontrolera AVR |
| --- | --- | --- |
| PIR 1 | D10 | PB2 / PCINT2 |
| PIR 2 | D11 | PB3 / PCINT3 |
| PIR 3 | D12 | PB4 / PCINT4 |
| PIR 4 | D13 | PB5 / PCINT5 |

W przypadku Arduino Uno z ATmega328P piny D10–D13 odpowiadają wejściom PB2–PB5 portu B, co umożliwia wykorzystanie przerwań PCINT.

### Zasady podłączania

- Diody LED należy podłączać przez odpowiednio dobrane rezystory ograniczające prąd.
- Czujniki PIR powinny być zasilane zgodnie z dokumentacją konkretnego modelu.
- Wszystkie elementy powinny mieć wspólną masę, jeżeli wymagają tego zastosowane układy.
- Należy sprawdzić zgodność poziomów napięć oraz dopuszczalne obciążenie pinów mikrokontrolera.

## ⚙️ Zasada działania

Program realizuje sterowanie sygnalizacją w kilku etapach.

### 1\. Inicjalizacja systemu

Podczas uruchamiania programu funkcja `setup()`:

- konfiguruje piny sygnalizatorów jako wyjścia;
- konfiguruje piny czujników PIR jako wejścia;
- ustawia wszystkie sygnalizatory na czerwone światło;
- inicjalizuje przerwania PCINT dla czujników podłączonych do portu B.

### 2\. Wykrywanie ruchu

Przerwanie `PCINT0_vect` reaguje na zmianę stanu wejść portu B.

Program porównuje aktualny stan portu z poprzednim stanem i wykrywa zbocza narastające, czyli przejścia sygnału z LOW na HIGH.

Po wykryciu odpowiedniego zbocza ustawiany jest bit zgłoszenia dla danego kierunku w zmiennej `zgloszenia`.

Procedury `przerwaniePIR1()`, `przerwaniePIR2()`, `przerwaniePIR3()` i `przerwaniePIR4()` odpowiadają za oznaczanie zgłoszeń z poszczególnych czujników.

### 3\. Odbieranie zgłoszeń

W głównej pętli `loop()` program tymczasowo wyłącza przerwania, kopiuje zgłoszenia do zmiennej lokalnej i zeruje zmienną współdzieloną.

Przerwania są następnie ponownie włączane, a zgłoszenia przekazywane do kolejki.

Takie rozwiązanie ogranicza ryzyko konfliktu podczas odczytu i modyfikacji danych współdzielonych przez pętlę główną oraz procedurę przerwania.

### 4\. Dodawanie zgłoszeń do kolejki

Funkcja `dodajDoKolejki()` dodaje kierunek do kolejki, jeśli:

- dany kierunek nie ma już oczekującego zgłoszenia;
- kolejka nie jest pełna.

Tablica `kolejka[4]` przechowuje maksymalnie cztery zgłoszenia. Zmienne `poczatek` i `koniec` określają odpowiednio pozycję odczytu i zapisu.

Kolejka jest cykliczna, co oznacza, że po osiągnięciu końca tablicy indeks wraca do zera.

### 5\. Przyznawanie zielonego światła

Jeżeli żaden kierunek nie jest aktywny, a kolejka zawiera zgłoszenia, program pobiera pierwsze zgłoszenie za pomocą funkcji `pobierzZKolejki()`.

Następnie funkcja `zieloneDla()`:

1. Ustawia wszystkie sygnalizatory na czerwone światło.
2. Włącza zielone światło dla wybranego kierunku.
3. Zapisuje czas rozpoczęcia zielonego światła w zmiennej `czasStartu`.

### 6\. Zakończenie obsługi kierunku

Zielone światło pozostaje aktywne przez czas określony w zmiennej `CZAS_ZIELONEGO`.

Po upływie tego czasu program:

- wyłącza zielone światło;
- ustawia wszystkie sygnalizatory na czerwone;
- zeruje aktywny kierunek;
- usuwa informację o oczekiwaniu dla obsłużonego kierunku.

W kolejnej iteracji pętli program może rozpocząć obsługę następnego zgłoszenia.

## 🧠 Najważniejsze zmienne i funkcje

| Element | Opis |
| --- | --- |
| `CZAS_ZIELONEGO` | Czas trwania zielonego światła w milisekundach. |
| `kolejka[4]` | Tablica przechowująca zgłoszenia kierunków. |
| `poczatek` | Indeks następnego elementu do pobrania. |
| `koniec` | Indeks pozycji, do której zostanie zapisane nowe zgłoszenie. |
| `liczbaWKolejce` | Liczba zgłoszeń znajdujących się w kolejce. |
| `oczekuje[]` | Tablica informująca, czy kierunek ma już oczekujące zgłoszenie lub jest aktualnie obsługiwany. |
| `aktywnyKierunek` | Numer kierunku, który aktualnie ma zielone światło; `0` oznacza brak aktywnego kierunku. |
| `czasStartu` | Moment rozpoczęcia zielonego światła. |
| `zgloszenia` | Maska bitowa zgłoszeń wykrytych przez przerwania. |
| `initPrzerwania()` | Konfiguruje przerwania PCINT. |
| `dodajDoKolejki()` | Dodaje zgłoszenie do kolejki. |
| `pobierzZKolejki()` | Pobiera najstarsze zgłoszenie. |
| `zieloneDla()` | Włącza zielone światło dla wybranego kierunku. |
| `wszystkieCzerwone()` | Ustawia wszystkie kierunki na czerwone światło. |

## ⏱️ Konfiguracja czasu

Domyślny czas zielonego światła wynosi 5000 ms, czyli 5 sekund.

cpp

```
int CZAS_ZIELONEGO = 5000;
```

Aby zmienić czas, należy zmodyfikować wartość tej zmiennej, np.:

cpp

```
int CZAS_ZIELONEGO = 10000; // 10 sekund
```

Pomiar czasu jest realizowany za pomocą funkcji `millis()`, dzięki czemu program nie blokuje działania na czas świecenia zielonego światła.

## ▶️ Uruchomienie projektu

1. Podłącz sygnalizatory LED i czujniki PIR zgodnie z tabelami pinów.
2. Otwórz kod w Arduino IDE.
3. Wybierz odpowiednią płytkę oraz port komunikacyjny.
4. Skompiluj program.
5. Wgraj program do mikrokontrolera.
6. Po uruchomieniu sprawdź, czy wszystkie kierunki początkowo mają czerwone światło.
7. Wywołuj sygnał z kolejnych czujników i obserwuj kolejność przyznawania zielonego światła.

## ⚠️ Ograniczenia i uwagi techniczne

- **Brak obsługi pojazdów i rzeczywistego ruchu drogowego:** czujniki PIR wykrywają zmiany promieniowania podczerwonego i nie są równoważne profesjonalnym czujnikom obecności pojazdów.
- **Brak czasu przejściowego:** program nie przewiduje fazy żółtego światła ani dodatkowego czasu bezpieczeństwa między zmianami kierunków.
- **Zbocze narastające:** zgłoszenie jest rejestrowane przy przejściu sygnału z LOW na HIGH. Sygnał stale utrzymujący się na HIGH nie generuje kolejnych zgłoszeń.
- **Ograniczona pojemność kolejki:** kolejka przechowuje maksymalnie cztery zgłoszenia. Nowe zgłoszenia mogą zostać pominięte, jeśli kolejka jest pełna.
- **Brak priorytetów:** wszystkie kierunki są traktowane jednakowo, a obsługa odbywa się według kolejności zgłoszeń.
- **Zależność od sprzętu:** konfiguracja przerwań i przypisanie pinów są specyficzne dla obsługiwanego mikrokontrolera.
- **Brak gwarancji bezpieczeństwa:** kod ma charakter edukacyjny i demonstracyjny. Nie powinien być stosowany do sterowania rzeczywistą sygnalizacją drogową bez odpowiednich mechanizmów bezpieczeństwa i walidacji.

## 🚀 Możliwe rozszerzenia

- Dodanie żółtego światła i faz przejściowych.
- Wprowadzenie minimalnego i maksymalnego czasu zielonego światła.
- Obsługa priorytetów dla wybranych kierunków.
- Dodanie wyświetlacza pokazującego aktualny stan sygnalizacji.
- Rejestrowanie liczby zgłoszeń i czasu oczekiwania.
- Zastosowanie czujników odpowiednich do wykrywania pojazdów.
- Dodanie obsługi błędów czujników i mechanizmów awaryjnych.

## 📄 Licencja

Jeżeli projekt ma być udostępniany publicznie, należy określić warunki jego wykorzystania, np. wybierając licencję MIT lub inną licencję odpowiednią dla projektu.
