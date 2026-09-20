#ifndef SCORE_H
#define SCORE_H

typedef enum { PLAYER1, PLAYER2 } ENUM_Player;

typedef struct {
    int A, B, C, D, E, F, G, DP;
    int DIGIT1, DIGIT2, DIGIT3, DIGIT4;
} Pins;

class Score {
public:
    Score(ENUM_Player p);
    void new_score(int score);
    void tick();         // avec timer interne (plus utilisé)
    void tickForced();   // avance d'un digit sans vérifier le timer ← nouveau
    void clearDigits();

private:
    typedef struct { int A, B, C, D, E, F, G, DP; } Digit;

    Digit digit_unite, digit_dizaine, digit_centaine, digit_millier;
    void number_to_digit(int number, Digit* d);
    int  printDigit(Digit d, int numDigit);
    void select_digit(int numDigit);

    Pins pins;
    int numDigitTotal = 0;

    // État interne non-bloquant
    int           _currentDigit = 0;
    unsigned long _lastMicros   = 0;
};

#endif