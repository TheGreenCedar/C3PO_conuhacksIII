int InputPin=D6;

void setup() {
  pinMode(InputPin,INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  Serial.begin(115200);
}

void loop() {
  while(digitalRead(InputPin)==HIGH){
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.println(1);
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(0);
}
