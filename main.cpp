
#include <avr/interrupt.h>
#define RED_1 2
#define GREEN_1 6
#define RED_2 3
#define GREEN_2 7
#define RED_3 4
#define GREEN_3 8
#define RED_4 5
#define GREEN_4 9
#define PIR_1 10
#define PIR_2 11
#define PIR_3 12
#define PIR_4 13
int CZAS_ZIELONEGO = 5000;
int kolejka[4];
int poczatek = 0;
int koniec = 0;
int liczbaWKolejce = 0;
bool oczekuje[5] = {false, false, false, false, false};
int aktywnyKierunek = 0;
unsigned long czasStartu = 0;
volatile int zgloszenia = 0;
volatile int poprzedniStanPINB = 0;
void przerwaniePIR1() {
  zgloszenia |= (1 << 0);
}
void przerwaniePIR2() {
  zgloszenia |= (1 << 1);
}
void przerwaniePIR3() {
  zgloszenia |= (1 << 2);
}
void przerwaniePIR4() {
  zgloszenia |= (1 << 3);
}
void initPrzerwania() {
  cli();
  poprzedniStanPINB = PINB;
  PCMSK0 |= (1 << PCINT2) | (1 << PCINT3) | (1 << PCINT4) | (1 << PCINT5);
  PCIFR |= (1 << PCIF0);
  PCICR |= (1 << PCIE0);
  sei();
}
ISR(PCINT0_vect) {
  uint8_t aktualnyStan = PINB;
  uint8_t noweZbocza = aktualnyStan & ~poprzedniStanPINB;
  if (noweZbocza & (1 << PB2)) {
    przerwaniePIR1();
  }
  if (noweZbocza & (1 << PB3)) {
    przerwaniePIR2();
  }
  if (noweZbocza & (1 << PB4)) {
    przerwaniePIR3();
  }
  if (noweZbocza & (1 << PB5)) {
    przerwaniePIR4();
  }
  poprzedniStanPINB = aktualnyStan;
}
void setup() {
  pinMode(RED_1, OUTPUT);
  pinMode(GREEN_1, OUTPUT);
  pinMode(RED_2, OUTPUT);
  pinMode(GREEN_2, OUTPUT);
  pinMode(RED_3, OUTPUT);
  pinMode(GREEN_3, OUTPUT);
  pinMode(RED_4, OUTPUT);
  pinMode(GREEN_4, OUTPUT);
  pinMode(PIR_1, INPUT);
  pinMode(PIR_2, INPUT);
  pinMode(PIR_3, INPUT);
  pinMode(PIR_4, INPUT);
  wszystkieCzerwone();
  initPrzerwania();
}
void loop() {
  int noweZgloszenia;
  noInterrupts();
  noweZgloszenia = zgloszenia;
  zgloszenia = 0;
  interrupts();
  for (int i = 0; i < 4; i++) {
    if (noweZgloszenia & (1 << i)) {
      dodajDoKolejki(i + 1);
    }
  }
  if (aktywnyKierunek == 0 && liczbaWKolejce > 0) {
    aktywnyKierunek = pobierzZKolejki();
    zieloneDla(aktywnyKierunek);
    czasStartu = millis();
  }
  if (aktywnyKierunek != 0 && millis() - czasStartu >= CZAS_ZIELONEGO) {
    int zakonczonyKierunek = aktywnyKierunek;
    wszystkieCzerwone();
    aktywnyKierunek = 0;
    oczekuje[zakonczonyKierunek] = false;
  }
}
void dodajDoKolejki(int kierunek) {
  if (oczekuje[kierunek]) {
    return;
  }
  if (liczbaWKolejce >= 4) {
    return;
  }
  kolejka[koniec] = kierunek;
  koniec = (koniec + 1) % 4;
  liczbaWKolejce++;
  oczekuje[kierunek] = true;
}
int pobierzZKolejki() {
  int kierunek = kolejka[poczatek];
  poczatek = (poczatek + 1) % 4;
  liczbaWKolejce--;
  return kierunek;
}
void zieloneDla(int strona) {
  wszystkieCzerwone();
  switch (strona) {
    case 1:
      digitalWrite(RED_1, LOW);
      digitalWrite(GREEN_1, HIGH);
      break;
    case 2:
      digitalWrite(RED_2, LOW);
      digitalWrite(GREEN_2, HIGH);
      break;
    case 3:
      digitalWrite(RED_3, LOW);
      digitalWrite(GREEN_3, HIGH);
      break;
    case 4:
      digitalWrite(RED_4, LOW);
      digitalWrite(GREEN_4, HIGH);
      break;
  }
}
void wszystkieCzerwone() {
  digitalWrite(GREEN_1, LOW);
  digitalWrite(GREEN_2, LOW);
  digitalWrite(GREEN_3, LOW);
  digitalWrite(GREEN_4, LOW);
  digitalWrite(RED_1, HIGH);
  digitalWrite(RED_2, HIGH);
  digitalWrite(RED_3, HIGH);
  digitalWrite(RED_4, HIGH);
}
