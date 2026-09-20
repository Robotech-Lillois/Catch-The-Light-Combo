#include "Score.h"

Score scoreP1(PLAYER1);
Score scoreP2(PLAYER2);

#define POINT_PAR_COUP 25
#define SCORE_MAX 1500

// ==========================================
//      BROCHES PHYSIQUES
// ==========================================
const byte BTN_ROWS = 3;
const byte BTN_COLS = 6;
byte btnRowPins[BTN_ROWS] = { 48, 46, 44 };
byte btnColPins[BTN_COLS] = { 52, 50, 47, 49, 51, 53 };

const byte LED_ROWS = 6;
const byte LED_COLS = 3;
byte ledRowPins[LED_ROWS] = { 41, 43, 45, 42, 40, 38 };
byte ledColPins[LED_COLS] = { 39, 37, 35 };

const byte pinStart = 12;  // A relier entre la broche 12 et la masse

// ==========================================
//      LES COORDONNÉES
// ==========================================
const int A1B1 = 0, A2B1 = 1, A3B1 = 2, A4B1 = 3, A5B1 = 4, A6B1 = 5;
const int A1B2 = 6, A2B2 = 7, A3B2 = 8, A4B2 = 9, A5B2 = 10, A6B2 = 11;
const int A1B3 = 12, A2B3 = 13, A3B3 = 14, A4B3 = 15, A5B3 = 16, A6B3 = 17;

String nomDesCases[18] = {
  "A1B1", "A2B1", "A3B1", "A4B1", "A5B1", "A6B1",
  "A1B2", "A2B2", "A3B2", "A4B2", "A5B2", "A6B2",
  "A1B3", "A2B3", "A3B3", "A4B3", "A5B3", "A6B3"
};

// ==========================================
//      LES GRILLES VISUELLES
// ==========================================
int grilleBoutons[BTN_ROWS][BTN_COLS] = {
  { A1B1, A2B1, A3B1, A4B1, A5B1, A6B1 },
  { A1B2, A2B2, A3B2, A4B2, A5B2, A6B2 },
  { A1B3, A2B3, A3B3, A4B3, A5B3, A6B3 }
};

int grilleLEDs[LED_ROWS][LED_COLS] = {
  { A1B1, A1B2, A1B3 },
  { A2B1, A2B2, A2B3 },
  { A3B1, A3B2, A3B3 },
  { A4B1, A4B2, A4B3 },
  { A5B1, A5B2, A5B3 },
  { A6B1, A6B2, A6B3 }
};

// ==========================================
//      LES BOUTONS MORTS
// ==========================================
int listeBoutonsMorts[] = { A2B1, A4B2, A1B2, A6B1, A6B3, A2B3, A6B2, A4B3 };

int nbBoutonsMorts = sizeof(listeBoutonsMorts) / sizeof(listeBoutonsMorts[0]);
bool estMort[18] = { false };

// ==========================================
//      MÉCANIQUES DU JEU COMPÉTITIF
// ==========================================
bool ledMatrixState[18] = { false };
bool prevBtnState[BTN_ROWS][BTN_COLS] = { false };
bool prevStartState = false;

// ETATS DU JEU : Animation d'attente
enum GameState { IDLE,
                 COUNTDOWN,
                 PLAYING,
                 GAMEOVER };
GameState etatJeu = IDLE;
unsigned long timerGlobal = 0;

struct Player {
  int id;
  int score;
  int serie;
  int boutons[9];
  bool isLedOn;
  int currentTarget;
  unsigned long stateTimer;
};

Player J1 = { 1, 0, 0, { A1B1, A2B1, A3B1, A1B2, A2B2, A3B2, A1B3, A2B3, A3B3 }, false, -1, 0 };
Player J2 = { 2, 0, 0, { A4B1, A5B1, A6B1, A4B2, A5B2, A6B2, A4B3, A5B3, A6B3 }, false, -1, 0 };

void setup() {
  Serial1.begin(9600);
  randomSeed(analogRead(A15));

  pinMode(pinStart, INPUT_PULLUP);

  for (int i = 0; i < nbBoutonsMorts; i++) {
    if (listeBoutonsMorts[i] >= 0 && listeBoutonsMorts[i] < 18) {
      estMort[listeBoutonsMorts[i]] = true;
    }
  }

  for (int i = 0; i < BTN_ROWS; i++) pinMode(btnRowPins[i], INPUT_PULLUP);
  for (int i = 0; i < BTN_COLS; i++) {
    pinMode(btnColPins[i], OUTPUT);
    digitalWrite(btnColPins[i], HIGH);
  }
  for (int i = 0; i < LED_ROWS; i++) {
    pinMode(ledRowPins[i], OUTPUT);
    digitalWrite(ledRowPins[i], LOW);
  }
  for (int i = 0; i < LED_COLS; i++) {
    pinMode(ledColPins[i], OUTPUT);
    digitalWrite(ledColPins[i], HIGH);
  }

  Serial1.println("--- BORNE BRANCHEE : MODE ATTENTE ---");
}

void loop() {
  unsigned long tempsActuel = millis();

  // ===================================================
  // GESTION DU BOUTON START
  // ===================================================
  static unsigned long lastDebounceTime = 0;
  static bool lastRawState = HIGH;
  static bool debouncedState = HIGH;

  bool rawState = digitalRead(pinStart);

  if (rawState != lastRawState) {
    lastDebounceTime = tempsActuel;
  }
  if (tempsActuel - lastDebounceTime > 50) {
    if (rawState != debouncedState) {
      debouncedState = rawState;

      if (debouncedState == LOW) {  // Front descendant = appui confirmé
        if (etatJeu == IDLE || etatJeu == GAMEOVER) {
          Serial1.println(">>> START PRESSE : DECOMPTE 5 SECONDES <<<");
          etatJeu = COUNTDOWN;
          timerGlobal = tempsActuel;
          eteindreToutesLEDs();
          scoreP1.new_score(8888);
          scoreP2.new_score(8888);
        } else if (etatJeu == PLAYING || etatJeu == COUNTDOWN) {
          Serial1.println(">>> PARTIE ANNULEE : RETOUR AU MENU <<<");
          etatJeu = IDLE;
          eteindreToutesLEDs();
        }
      }
    }
  }
  lastRawState = rawState;
  // ===================================================
  // MACHINE A ETATS
  // ===================================================

  if (etatJeu == IDLE) {
    jouerAnimationAttente(tempsActuel);
  }

  else if (etatJeu == COUNTDOWN) {
    if (tempsActuel - timerGlobal >= 5000) {
      Serial1.println("GO GO GO !!!");
      lancerPartie(tempsActuel);
    }
  }

  else if (etatJeu == PLAYING) {
    updatePlayerLogic(&J1, tempsActuel);
    updatePlayerLogic(&J2, tempsActuel);

    lireBoutonsEnJeu(tempsActuel);

    if (J1.score >= SCORE_MAX || J2.score >= SCORE_MAX) {
      etatJeu = GAMEOVER;
      eteindreToutesLEDs();
      timerGlobal = tempsActuel;

      Serial1.println("===============================");
      Serial1.println("         FIN DU JEU !          ");
      Serial1.println("===============================");
      if (J1.score >= SCORE_MAX) Serial1.println(">>> LE JOUEUR 1 EST LE GRAND GAGNANT ! <<<");
      else Serial1.println(">>> LE JOUEUR 2 EST LE GRAND GAGNANT ! <<<");
      Serial1.println("Appuyez sur START pour rejouer...");
    }
  }

  else if (etatJeu == GAMEOVER) {
    if (tempsActuel - timerGlobal >= 10000) {
      Serial1.println(">>> RETOUR A L'ANIMATION D'ATTENTE <<<");
      etatJeu = IDLE;
    }
  }

  // if (etatJeu == PLAYING || etatJeu == GAMEOVER) {
    scoresTick();
  // }


  multiplexageRapide();
}

// ==========================================
//      FONCTIONS DU JEU ET ANIMATIONS
// ==========================================

void jouerAnimationAttente(unsigned long tempsActuel) {
  static unsigned long timerAnimation = 0;

  // On modifie l'affichage aléatoire toutes les 300 millisecondes
  if (tempsActuel - timerAnimation >= 300 || timerAnimation == 0) {
    timerAnimation = tempsActuel;
    eteindreToutesLEDs();

    // Joueur 1 : On tire 3 cibles au hasard parmi ses 9 boutons
    ledMatrixState[J1.boutons[random(0, 9)]] = true;
    ledMatrixState[J1.boutons[random(0, 9)]] = true;
    ledMatrixState[J1.boutons[random(0, 9)]] = true;

    // Joueur 2 : On tire 3 cibles au hasard parmi ses 9 boutons
    ledMatrixState[J2.boutons[random(0, 9)]] = true;
    ledMatrixState[J2.boutons[random(0, 9)]] = true;
    ledMatrixState[J2.boutons[random(0, 9)]] = true;
  }
}

void lancerPartie(unsigned long tempsActuel) {
  etatJeu = PLAYING;

  J1.score = 0;
  J1.serie = 0;
  J1.isLedOn = false;
  J1.currentTarget = -1;
  J1.stateTimer = tempsActuel + random(150, 800);

  J2.score = 0;
  J2.serie = 0;
  J2.isLedOn = false;
  J2.currentTarget = -1;
  J2.stateTimer = tempsActuel + random(150, 800);

  eteindreToutesLEDs();
}

void updatePlayerLogic(Player* p, unsigned long tempsActuel) {
  if (!p->isLedOn) {
    if (tempsActuel >= p->stateTimer) {

      int tirage;
      do {
        tirage = p->boutons[random(0, 9)];
      } while (estMort[tirage] == true);

      p->currentTarget = tirage;
      ledMatrixState[p->currentTarget] = true;
      p->isLedOn = true;
      p->stateTimer = tempsActuel + random(500, 1500);
    }
  } else {
    if (tempsActuel >= p->stateTimer) {
      Serial1.print("J");
      Serial1.print(p->id);
      Serial1.println(" : Trop lent ! Combo a zero.");
      p->serie = 0;
      ledMatrixState[p->currentTarget] = false;
      p->isLedOn = false;
      p->currentTarget = -1;
      p->stateTimer = tempsActuel + random(150, 800);
    }
  }
}

void lireBoutonsEnJeu(unsigned long tempsActuel) {
  for (int c = 0; c < BTN_COLS; c++) {
    digitalWrite(btnColPins[c], LOW);

    for (int r = 0; r < BTN_ROWS; r++) {
      bool isPressed = (digitalRead(btnRowPins[r]) == LOW);
      int caseAppuyee = grilleBoutons[r][c];

      if (estMort[caseAppuyee]) {
        prevBtnState[r][c] = isPressed;
        continue;
      }

      if (isPressed && !prevBtnState[r][c]) {
        Player* joueurConcerne = NULL;
        for (int i = 0; i < 9; i++) {
          if (J1.boutons[i] == caseAppuyee) joueurConcerne = &J1;
          if (J2.boutons[i] == caseAppuyee) joueurConcerne = &J2;
        }

        if (joueurConcerne != NULL && joueurConcerne->isLedOn) {

          if (caseAppuyee == joueurConcerne->currentTarget) {
            joueurConcerne->serie++;

            int multiplicateur = 1;
            if (joueurConcerne->serie >= 2) multiplicateur = 2;
            if (joueurConcerne->serie >= 7) multiplicateur = 3;
            if (joueurConcerne->serie >= 15) multiplicateur = 4;

            int pointsGagnes = POINT_PAR_COUP * multiplicateur;
            joueurConcerne->score += pointsGagnes;

            Serial1.print("J");
            Serial1.print(joueurConcerne->id);
            Serial1.print(" +");
            Serial1.print(pointsGagnes);
            Serial1.print(" (x");
            Serial1.print(multiplicateur);
            Serial1.print(") | Score total: ");
            Serial1.println(joueurConcerne->score);

            if (joueurConcerne->id == 1) scoreP1.new_score(joueurConcerne->score);
            else scoreP2.new_score(joueurConcerne->score);

          } else {
            joueurConcerne->serie = 0;
            Serial1.print("J");
            Serial1.print(joueurConcerne->id);
            Serial1.println(" ERREUR ! Combo a zero.");
          }

          ledMatrixState[joueurConcerne->currentTarget] = false;
          joueurConcerne->isLedOn = false;
          joueurConcerne->currentTarget = -1;
          joueurConcerne->stateTimer = tempsActuel + random(200, 1500);
        }
      }
      prevBtnState[r][c] = isPressed;
    }
    digitalWrite(btnColPins[c], HIGH);
  }
}

// ==========================================
//      MOTEUR D'AFFICHAGE MULTIPLEXÉ
// ==========================================
void multiplexageRapide() {
  static int currentRow = 0;
  static unsigned long lastMicros = 0;

  unsigned long now = micros();
  if (now - lastMicros < 2500) return;  // Une ligne toutes les 500µs
  lastMicros = now;

  // Éteindre la ligne précédente
  int prevRow = (currentRow + LED_ROWS - 1) % LED_ROWS;
  digitalWrite(ledRowPins[prevRow], LOW);
  for (int c = 0; c < LED_COLS; c++) digitalWrite(ledColPins[c], HIGH);

  // Préparer et allumer la ligne courante
  bool hasLed = false;
  for (int led_c = 0; led_c < LED_COLS; led_c++) {
    int caseActuelle = grilleLEDs[currentRow][led_c];
    bool autorisation = !estMort[caseActuelle] || (etatJeu == IDLE);
    if (ledMatrixState[caseActuelle] && autorisation) {
      digitalWrite(ledColPins[led_c], LOW);
      hasLed = true;
    }
  }
  if (hasLed) digitalWrite(ledRowPins[currentRow], HIGH);

  currentRow = (currentRow + 1) % LED_ROWS;
}

void eteindreToutesLEDs() {
  for (int i = 0; i < 18; i++) ledMatrixState[i] = false;
}

void scoresTick() {
  static uint8_t slot = 0;
  static unsigned long lastUs = 0;

  unsigned long now = micros();
  if (now - lastUs < 1500) return;  // ajuste ce délai pour la luminosité
  lastUs = now;

  // Éteindre les deux avant de changer
  scoreP1.clearDigits();
  scoreP2.clearDigits();

  // Un slot sur deux : P1, puis P2, puis P1...
  if (slot % 2 == 0) scoreP1.tickForced();
  else scoreP2.tickForced();

  slot++;
}