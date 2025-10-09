void setup() {
  pinMode(13, OUTPUT);
}

void loop() {
  // Fade in
  for (int v = 1; v <= 20; v++) {
    unsigned long t = millis();
    while (millis() - t < 20) {
      if (millis() - t < v) digitalWrite(13, 1);
      else digitalWrite(13, 0);
    }
  }

  // Fade out
  for (int v = 20; v >= 1; v--) {
    unsigned long t = millis();
    while (millis() - t < 20) {
      if (millis() - t < v) digitalWrite(13, 1);
      else digitalWrite(13, 0);
    }
  }
}
