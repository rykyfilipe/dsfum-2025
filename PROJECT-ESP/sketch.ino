const int segmentPins[] = {32, 33, 25, 26, 27, 14, 12, 23}; 
const int digitPins[]   = {13, 15, 2, 4, 5, 18, 19, 21};   

const byte hexMap[16] = {
  0b00111111, 0b00000110, 0b01011011, 0b01001111, // 0, 1, 2, 3
  0b01100110, 0b01101101, 0b01111101, 0b00000111, // 4, 5, 6, 7
  0b01111111, 0b01101111, 0b01110111, 0b01111100, // 8, 9, A, B
  0b00111001, 0b01011110, 0b01111001, 0b01110001  // C, D, E, F
};

unsigned long targetValue = 0;
unsigned long currentValue = 0;
bool isRunning = false;

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(segmentPins[i], OUTPUT);
    pinMode(digitPins[i], OUTPUT);
    digitalWrite(digitPins[i], HIGH);
  }
  Serial.println("Sistem pornit. Introduceti parola HEX (ex: A1B2C3D4):");
}

void loop() {
  
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    if (input.length() > 0) {
      targetValue = strtoul(input.c_str(), NULL, 16); // Convertim textul HEX in numar
      currentValue = 0;   
      isRunning = true;    // Pornim simularea
      Serial.print("Cautam parola: "); Serial.println(input);
    }
  }

  
  if (isRunning) {
    if (currentValue < targetValue) {
      if (targetValue - currentValue > 500) currentValue += 500; 
      else currentValue++;
    } else {
      currentValue = targetValue; // Am gasit-o!
      isRunning = false;
      Serial.println("SUCCES! Parola gasita.");
    }
  }

  refreshDisplay(currentValue);
}

void refreshDisplay(unsigned long val) {
  char textHex[9]; 
  sprintf(textHex, "%08X", val); // Transforma numarul in "0000ABCD"

  for (int i = 0; i < 8; i++) {
    char caracter = textHex[i];
    int index;

    // Transformam litera inapoi in cifra (0-15)
    if (caracter >= '0' && caracter <= '9') index = caracter - '0';
    else index = caracter - 'A' + 10;

    // Multiplexare
    opresteTotiDigitii();
    scrieSegmente(index);
    digitalWrite(digitPins[i], LOW); 
    delayMicroseconds(500); 
  }
}

void scrieSegmente(int n) {
  for (int s = 0; s < 7; s++) {
    digitalWrite(segmentPins[s], bitRead(hexMap[n], s));
  }
}

void opresteTotiDigitii() {
  for (int d = 0; d < 8; d++) digitalWrite(digitPins[d], HIGH);
}
