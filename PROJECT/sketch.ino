#include <Arduino.h>
#include <LedControl.h>

// --- PIN DEFINITIONS ---
#define BUTTON_UP_PIN 41
#define BUTTON_RIGHT_PIN 45
#define BUTTON_DOWN_PIN 47
#define BUTTON_LEFT_PIN 43
#define BUZZER_PIN 49

#define SR_DATA 9
#define SR_LATCH 8
#define SR_CLK 7

#define DIN_PIN   11
#define CLK_PIN   13
#define CS_PIN    10
#define NR_MATRICI 8  

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, NR_MATRICI);

// --- GAME VARIABLES ---
uint32_t lastButtonTime = 0;
uint32_t buttonDebounce = 150;
uint32_t lastMoveTime = 0;   
uint32_t moveInterval = 300; 

int score = 0;
int digitsBuffer[4] = {0, 0, 0, 0}; 

// --- GAME BOARD MEMORY (32 Rows x 16 Columns) ---
// Folosim 2 bytes per rand: byte[0] pt coloanele 0-7, byte[1] pt coloanele 8-15
uint8_t gameBoard[32][2]; 

// --- 7 SEGMENT DATA ---
const uint8_t digitCodes[10] = {
  0b01010000, 0b11010111, 0b01100100, 0b01000101, 
  0b11000001, 0b01001001, 0b01001000, 0b11010101, 
  0b01000000, 0b01000001
};
const uint8_t digitPosMap[4] = { 0x20, 0x40, 0x80, 0x01 };

// --- TETROMINOS ---
#define NUM_SHAPES 7
#define NUM_ROTATIONS 4
#define SHAPE_HEIGHT 4 

const uint8_t tetrominos[NUM_SHAPES][NUM_ROTATIONS][SHAPE_HEIGHT] = {
  { { B00000000, B11110000, B00000000, B00000000 }, { B01000000, B01000000, B01000000, B01000000 }, { B00000000, B11110000, B00000000, B00000000 }, { B01000000, B01000000, B01000000, B01000000 } }, // I
  { { B10000000, B11100000, B00000000, B00000000 }, { B01100000, B01000000, B01000000, B00000000 }, { B00000000, B11100000, B00100000, B00000000 }, { B01000000, B01000000, B11000000, B00000000 } }, // J
  { { B00100000, B11100000, B00000000, B00000000 }, { B01000000, B01000000, B01100000, B00000000 }, { B00000000, B11100000, B10000000, B00000000 }, { B11000000, B01000000, B01000000, B00000000 } }, // L
  { { B01100000, B01100000, B00000000, B00000000 }, { B01100000, B01100000, B00000000, B00000000 }, { B01100000, B01100000, B00000000, B00000000 }, { B01100000, B01100000, B00000000, B00000000 } }, // O
  { { B01100000, B11000000, B00000000, B00000000 }, { B01000000, B01100000, B00100000, B00000000 }, { B00000000, B01100000, B11000000, B00000000 }, { B10000000, B11000000, B01000000, B00000000 } }, // S
  { { B01000000, B11100000, B00000000, B00000000 }, { B01000000, B01100000, B01000000, B00000000 }, { B00000000, B11100000, B01000000, B00000000 }, { B01000000, B11000000, B01000000, B00000000 } }, // T
  { { B11000000, B01100000, B00000000, B00000000 }, { B00100000, B01100000, B01000000, B00000000 }, { B00000000, B11000000, B01100000, B00000000 }, { B01000000, B11000000, B10000000, B00000000 } }  // Z
};

int currentPieceType = 0; 
int currentRotation = 0;  
int currentPieceX = 0;
int currentPieceY = 0;


void setPhysicalLed(int x, int y, bool state) {
  if (x >= 0 && x < 16 && y >= 0 && y < 32) {
    int matrixIndex;
    int localX;
    int localY;

    if (x < 8) {
      matrixIndex = y / 8; 
      localX = x;
      localY = y % 8;
      lc.setLed(matrixIndex, localX, localY, state);
    } else {
      // Dreapta: Rotatie 90 (Inversata complet)
      int rowGroup = y / 8; 
      matrixIndex = 7 - rowGroup; 
      
      localX = x - 8;
      localX = 7 - localX; 
      
      localY = y % 8;
      localY = 7 - localY;

      lc.setLed(matrixIndex, localX, localY, state);
    }
  }
}

void update_score_buffer(){
  int tempScore = score;
  digitsBuffer[0] = tempScore % 10;        
  digitsBuffer[1] = (tempScore / 10) % 10; 
  digitsBuffer[2] = (tempScore / 100) % 10;
  digitsBuffer[3] = (tempScore / 1000) % 10;
}

void drawScore() {
  static int currentDigit = 0;
  static unsigned long lastUpdate = 0;
  
  if (micros() - lastUpdate > 2000) { 
    lastUpdate = micros();

    digitalWrite(SR_LATCH, LOW);
    shiftOut(SR_DATA, SR_CLK, MSBFIRST, 0x00); 
    shiftOut(SR_DATA, SR_CLK, MSBFIRST, 0xFF); 
    digitalWrite(SR_LATCH, HIGH);

    digitalWrite(SR_LATCH, LOW);
    shiftOut(SR_DATA, SR_CLK, MSBFIRST, digitPosMap[currentDigit]); 
    shiftOut(SR_DATA, SR_CLK, MSBFIRST, digitCodes[digitsBuffer[currentDigit]]); 
    digitalWrite(SR_LATCH, HIGH);

    currentDigit++;
    if (currentDigit > 3) currentDigit = 0;
  }
}


void drawPiece(int currentX, int currentY, int shapeIndex, int rotation, bool state) {
  for (int row = 0; row < 4; row++) {
    uint8_t rowData = tetrominos[shapeIndex][rotation][row];
    for (int col = 0; col < 4; col++) {
      if (rowData & (0x80 >> col)) {
        setPhysicalLed(currentX + col, currentY + row, state);
      }
    }
  }
}

// Redeseneaza toata tabla din memorie pe LED-uri
void drawBoard() {
  for (int y = 0; y < 32; y++) {
    for (int x = 0; x < 16; x++) {
      int byteIndex = (x < 8) ? 0 : 1;
      int bitIndex = (x < 8) ? (7 - x) : (15 - x);
      bool state = (gameBoard[y][byteIndex] >> bitIndex) & 1;
      setPhysicalLed(x, y, state);
    }
  }
}

void clearBoardMemory() {
  for(int i=0; i<32; i++) {
    gameBoard[i][0] = 0;
    gameBoard[i][1] = 0;
  }
}

bool isValidMove(int x, int y, int rotation) {
  for (int row = 0; row < 4; row++) {
    uint8_t rowData = tetrominos[currentPieceType][rotation][row];
    for (int col = 0; col < 4; col++) {
      if (rowData & (0x80 >> col)) {
        int pixelX = x + col;
        int pixelY = y + row;

        // 1. Verificare limite ecran
        if (pixelX < 0 || pixelX > 15) return false;
        if (pixelY >= 32) return false;

        // 2. Verificare coliziune cu piese existente (din gameBoard)
        if (pixelY >= 0) {
           int byteIndex = (pixelX < 8) ? 0 : 1;
           int bitIndex = (pixelX < 8) ? (7 - pixelX) : (15 - pixelX);
           if ((gameBoard[pixelY][byteIndex] >> bitIndex) & 1) {
             return false;
           }
        }
      }
    }
  }
  return true;
}

// Salveaza piesa curenta in matricea gameBoard
void lockPiece() {
  for (int row = 0; row < 4; row++) {
    uint8_t rowData = tetrominos[currentPieceType][currentRotation][row];
    for (int col = 0; col < 4; col++) {
      if (rowData & (0x80 >> col)) {
        int pixelX = currentPieceX + col;
        int pixelY = currentPieceY + row;
        
        if (pixelY >= 0 && pixelY < 32 && pixelX >=0 && pixelX < 16) {
          int byteIndex = (pixelX < 8) ? 0 : 1;
          int bitIndex = (pixelX < 8) ? (7 - pixelX) : (15 - pixelX);
          gameBoard[pixelY][byteIndex] |= (1 << bitIndex);
        }
      }
    }
  }
}

void checkLines() {
  int linesCleared = 0;
  
  // Parcurgem de sus in jos
  for (int y = 0; y < 32; y++) {
    // Verificam daca linia e plina (ambii bytes sunt 0xFF)
    if (gameBoard[y][0] == 0xFF && gameBoard[y][1] == 0xFF) {
      linesCleared++;
      
      // Efect vizual rapid (blink linia)
      for(int k=0; k<3; k++){
        for(int x=0; x<16; x++) setPhysicalLed(x, y, false);
        delay(30);
        drawScore(); // Continua refresh la 7seg
        for(int x=0; x<16; x++) setPhysicalLed(x, y, true);
        delay(30);
        drawScore();
      }

      // Mutam totul de deasupra cu o linie mai jos
      for (int row = y; row > 0; row--) {
        gameBoard[row][0] = gameBoard[row - 1][0];
        gameBoard[row][1] = gameBoard[row - 1][1];
      }
      // Linia de sus devine goala
      gameBoard[0][0] = 0;
      gameBoard[0][1] = 0;
    }
  }

  if (linesCleared > 0) {
    score += (linesCleared * 100); // 100 pct per linie
    tone(BUZZER_PIN, 600, 150);
    update_score_buffer();
    drawBoard(); // Redesenam totul curat
  }
}

void spawnPiece() {
  currentPieceType = random(0, 7);
  currentRotation = 0;
  currentPieceY = 0;       
  currentPieceX = 6; 
}

void gameOver() {
  tone(BUZZER_PIN, 200, 500);
  delay(500);
  score = 0;
  update_score_buffer();
  clearBoardMemory();
  for(int index = 0; index < NR_MATRICI; index++) lc.clearDisplay(index);
}

void setup(){
  Serial.begin(9600);
  randomSeed(analogRead(A5));

  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_RIGHT_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_LEFT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(SR_DATA, OUTPUT);
  pinMode(SR_LATCH, OUTPUT);
  pinMode(SR_CLK, OUTPUT);

  digitalWrite(SR_LATCH, LOW);
  shiftOut(SR_DATA, SR_CLK, MSBFIRST, 0);
  shiftOut(SR_DATA, SR_CLK, MSBFIRST, 0);
  digitalWrite(SR_LATCH, HIGH);
  
  update_score_buffer(); 
  clearBoardMemory();

  for(int index = 0; index < NR_MATRICI; index++) {
    lc.shutdown(index, false);
    lc.setIntensity(index, 2); 
    lc.clearDisplay(index);
  }

  spawnPiece();
}

void handle_buttons_input(){
  if(millis() - lastButtonTime < buttonDebounce) return;
  
  bool actionTaken = false;

  // ROTATIE
  if(digitalRead(BUTTON_UP_PIN) == HIGH){
    tone(BUZZER_PIN, 400, 20); 
    int nextRotation = currentRotation + 1;
    if (nextRotation > 3) nextRotation = 0;

    if (isValidMove(currentPieceX, currentPieceY, nextRotation)) {
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, false);
      currentRotation = nextRotation;
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
      actionTaken = true;
    }
  }
  // STANGA
  else if(digitalRead(BUTTON_LEFT_PIN) == HIGH){
    tone(BUZZER_PIN, 300, 20);
    if (isValidMove(currentPieceX - 1, currentPieceY, currentRotation)) {
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, false);
      currentPieceX--;
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
      actionTaken = true;
    }
  }
  // DREAPTA
  else if(digitalRead(BUTTON_RIGHT_PIN) == HIGH){
    tone(BUZZER_PIN, 350, 20);
    if (isValidMove(currentPieceX + 1, currentPieceY, currentRotation)) {
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, false);
      currentPieceX++;
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
      actionTaken = true;
    }
  }
  // JOS (Soft Drop)
  else if(digitalRead(BUTTON_DOWN_PIN) == HIGH){
    tone(BUZZER_PIN, 250, 20);
    if (isValidMove(currentPieceX, currentPieceY + 1, currentRotation)) {
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, false);
      currentPieceY++;
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
      update_score_buffer();
      actionTaken = true;
    }
  }

  if(actionTaken) {
    lastButtonTime = millis();
  }
}

void loop(){
  handle_buttons_input();
  drawScore();

  if (millis() - lastMoveTime > moveInterval) {
    lastMoveTime = millis();

    // Incarcam sa coboram piesa
    if (isValidMove(currentPieceX, currentPieceY + 1, currentRotation)) {
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, false);
      currentPieceY++;
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
    } else {
      lockPiece();   
      checkLines();  
      
      if (currentPieceY <= 1) {
          gameOver();
      } 
      
      spawnPiece(); 
      
      if (!isValidMove(currentPieceX, currentPieceY, currentRotation)) {
         gameOver();
      }
      
      drawPiece(currentPieceX, currentPieceY, currentPieceType, currentRotation, true);
    }
  }
}