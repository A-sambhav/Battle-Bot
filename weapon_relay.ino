// Define pins
const int inputPin = 2;  
const int relayPin = 8;   

void setup() {
  pinMode(inputPin, INPUT);      
  pinMode(relayPin, OUTPUT);     
  digitalWrite(relayPin, LOW);   
}

void loop() {
  int inputState = digitalRead(inputPin); 

  if (inputState == HIGH) {
    digitalWrite(relayPin, HIGH); // Turn relay ON
  } else {
    digitalWrite(relayPin, LOW);  // Turn relay OFF
  }
}
