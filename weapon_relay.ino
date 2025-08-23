// Define pins
const int inputPin = 2;   // Signal input pin
const int relayPin = 8;   // Relay control pin

void setup() {
  pinMode(inputPin, INPUT);      // Input pin
  pinMode(relayPin, OUTPUT);     // Relay pin
  digitalWrite(relayPin, LOW);   // Ensure relay starts OFF
}

void loop() {
  int inputState = digitalRead(inputPin); // Read input signal

  if (inputState == HIGH) {
    digitalWrite(relayPin, HIGH); // Turn relay ON
  } else {
    digitalWrite(relayPin, LOW);  // Turn relay OFF
  }
}
