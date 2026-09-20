#include <Arduino.h>
#include "Score.h"

#define MAX_DIGIT    4
#define TEMPO_SYNCRO 1500  // µs par digit — ajuste si scintillement

Score::Score(ENUM_Player p) {
    pins.A  = 7;
    pins.B  = 5;
    pins.C  = 3;
    pins.D  = 0;  
    pins.E  = 2;
    pins.F  = 6;
    pins.G  = 4;
    pins.DP = 1;  

    if (p == PLAYER1) {
        pins.DIGIT1 = 8;
        pins.DIGIT2 = 11;
        pins.DIGIT3 = 10;
        pins.DIGIT4 = 9;
    } else {
        pins.DIGIT1 = A1;
        pins.DIGIT2 = A0;
        pins.DIGIT3 = A3;
        pins.DIGIT4 = A2;
    }

    pinMode(pins.A,  OUTPUT); pinMode(pins.B,  OUTPUT);
    pinMode(pins.C,  OUTPUT); pinMode(pins.D,  OUTPUT);
    pinMode(pins.E,  OUTPUT); pinMode(pins.F,  OUTPUT);
    pinMode(pins.G,  OUTPUT); pinMode(pins.DP, OUTPUT);
    pinMode(pins.DIGIT1, OUTPUT); pinMode(pins.DIGIT2, OUTPUT);
    pinMode(pins.DIGIT3, OUTPUT); pinMode(pins.DIGIT4, OUTPUT);

    digitalWrite(pins.DP, LOW);
}

// --- tick() : affiche UN digit puis rend la main immédiatement ---
void Score::tick() {
    if (numDigitTotal == 0) return;

    unsigned long now = micros();
    if (now - _lastMicros < TEMPO_SYNCRO) return;  // Pas encore l'heure
    _lastMicros = now;

    // Éteindre tous les digits avant de changer
    clearDigits();

    // Afficher le digit courant
    switch (_currentDigit) {
        case 0: printDigit(digit_unite,    4); break;
        case 1: printDigit(digit_dizaine,  3); break;
        case 2: printDigit(digit_centaine, 2); break;
        case 3: printDigit(digit_millier,  1); break;
    }

    // Avancer au digit suivant (cycle sur numDigitTotal)
    _currentDigit = (_currentDigit + 1) % numDigitTotal;
}

void Score::clearDigits() {
    digitalWrite(pins.DIGIT1, 1);
    digitalWrite(pins.DIGIT2, 1);
    digitalWrite(pins.DIGIT3, 1);
    digitalWrite(pins.DIGIT4, 1);
}

// --- Tout le reste inchangé ---
void Score::number_to_digit(int number, Digit* d) {
    switch (number) {
        case 0: d->A=1;d->B=1;d->C=1;d->D=1;d->E=1;d->F=1;d->G=0; break;
        case 1: d->A=0;d->B=1;d->C=1;d->D=0;d->E=0;d->F=0;d->G=0; break;
        case 2: d->A=1;d->B=1;d->C=0;d->D=1;d->E=1;d->F=0;d->G=1; break;
        case 3: d->A=1;d->B=1;d->C=1;d->D=1;d->E=0;d->F=0;d->G=1; break;
        case 4: d->A=0;d->B=1;d->C=1;d->D=0;d->E=0;d->F=1;d->G=1; break;
        case 5: d->A=1;d->B=0;d->C=1;d->D=1;d->E=0;d->F=1;d->G=1; break;
        case 6: d->A=1;d->B=0;d->C=1;d->D=1;d->E=1;d->F=1;d->G=1; break;
        case 7: d->A=1;d->B=1;d->C=1;d->D=0;d->E=0;d->F=0;d->G=0; break;
        case 8: d->A=1;d->B=1;d->C=1;d->D=1;d->E=1;d->F=1;d->G=1; break;
        case 9: d->A=1;d->B=1;d->C=1;d->D=1;d->E=0;d->F=1;d->G=1; break;
    }
    d->DP = 0;
}

void Score::new_score(int score) {
    if      (score >= 1000) numDigitTotal = 4;
    else if (score >= 100)  numDigitTotal = 3;
    else if (score >= 10)   numDigitTotal = 2;
    else                    numDigitTotal = 1;

    if (numDigitTotal <= 4) number_to_digit((score / 1000) % 10, &digit_millier);
    if (numDigitTotal >= 3) number_to_digit((score / 100)  % 10, &digit_centaine);
    if (numDigitTotal >= 2) number_to_digit((score / 10)   % 10, &digit_dizaine);
    if (numDigitTotal >= 1) number_to_digit(score % 10,          &digit_unite);

    _currentDigit = 0; // Recommencer le cycle proprement
}

void Score::select_digit(int numDigit) {
    switch (numDigit) {
        case 1: digitalWrite(pins.DIGIT1,0); digitalWrite(pins.DIGIT2,1); digitalWrite(pins.DIGIT3,1); digitalWrite(pins.DIGIT4,1); break;
        case 2: digitalWrite(pins.DIGIT2,0); digitalWrite(pins.DIGIT1,1); digitalWrite(pins.DIGIT3,1); digitalWrite(pins.DIGIT4,1); break;
        case 3: digitalWrite(pins.DIGIT3,0); digitalWrite(pins.DIGIT1,1); digitalWrite(pins.DIGIT2,1); digitalWrite(pins.DIGIT4,1); break;
        case 4: digitalWrite(pins.DIGIT4,0); digitalWrite(pins.DIGIT1,1); digitalWrite(pins.DIGIT2,1); digitalWrite(pins.DIGIT3,1); break;
    }
}

int Score::printDigit(Digit d, int numDigit) {
    if (numDigit < 1 || numDigit > MAX_DIGIT) return 1;
    select_digit(numDigit);
    digitalWrite(pins.A, d.A); digitalWrite(pins.B, d.B);
    digitalWrite(pins.C, d.C); digitalWrite(pins.D, d.D);
    digitalWrite(pins.E, d.E); digitalWrite(pins.F, d.F);
    digitalWrite(pins.G, d.G);
    return 0;
}

void Score::tickForced() {
    if (numDigitTotal == 0) return;
    clearDigits();
    switch (_currentDigit) {
        case 0: printDigit(digit_unite,    4); break;
        case 1: printDigit(digit_dizaine,  3); break;
        case 2: printDigit(digit_centaine, 2); break;
        case 3: printDigit(digit_millier,  1); break;
    }
    _currentDigit = (_currentDigit + 1) % numDigitTotal;
}