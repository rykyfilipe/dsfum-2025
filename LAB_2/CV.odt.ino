void setup() {
  pinMode(13, OUTPUT);

}

void loop() {
  
  for (int vController = 1; vController < 20; vController++){
    unsigned long t = millis();
    while(millis()-t < 20){
        if(millis()-t < vController) digitalWrite(13,1);
        else digitalWrite(13,0);
  }
  
  for (int vController = 20; vController > 1; vController--){
    unsigned long t = millis();
    while(millis()-t < 20){
        if(millis()-t < vController) digitalWrite(13,1);
        else digitalWrite(13,0);
  }
  }
}

}