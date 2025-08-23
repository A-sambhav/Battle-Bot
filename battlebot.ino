#include <HardwareSerial.h>

// ---------- Hardware Configuration ----------
#define IBUS_RX_PIN   4  // GPIO pin for IBUS data reception

// ---------- Global Variables ----------
uint16_t rcChannels[6] = {1500, 1500, 1000, 1500, 1000, 1000}; // Default values for 6 channels

// ================= Motor Driver Pins =================
// Right motor
const int RPWM   = 32;   // Right motor forward PWM
const int LPWM   = 33;   // Right motor reverse PWM
const int REN    = 26;   // Right enable
const int LEN    = 27;   // Right enable

// Left motor
const int RPWM_1 = 14;   // Left motor forward PWM
const int LPWM_1 = 12;   // Left motor reverse PWM
const int REN_1  = 13;   // Left enable
const int LEN_1  = 15;   // Left enable

// PWM Properties for motors
const int freq = 5000;
const int resolution = 8;

// Weapon Relay
const int RELAY_PIN = 2;

// ---------- IBUS Reading Function ----------
void readIbus() {
  // Check if we have enough bytes available for a complete IBUS frame (32 bytes)
  if (Serial1.available() >= 32) {
    // Check for IBUS header bytes (0x20 0x40)
    if (Serial1.read() == 0x20 && Serial1.read() == 0x40) {
      // Read 6 channels (each channel is 2 bytes, low byte first)
      for (int i = 0; i < 6; i++) { 
        rcChannels[i] = Serial1.read() | (Serial1.read() << 8); 
      }
      // Clear any remaining bytes in the buffer
      while (Serial1.available()) Serial1.read();
    }
  }
}

// ---------- Display Function ----------
void displayChannels() {
  Serial.print("RC Channels: ");
  for (int i = 0; i < 6; i++) {
    Serial.print("CH");
    Serial.print(i + 1);
    Serial.print(":");
    Serial.print(rcChannels[i]);
    if (i < 5) Serial.print(" | ");
  }
  Serial.println();
  
  // Display interpreted values
  Serial.print("Interpreted: ");
  Serial.print("Roll:");
  Serial.print(map(rcChannels[0], 1000, 2000, -100, 100));
  Serial.print(" Pitch:");
  Serial.print(map(rcChannels[1], 1000, 2000, -100, 100));
  Serial.print(" Throttle:");
  Serial.print(map(rcChannels[2], 1000, 2000, 0, 100));
  Serial.print(" Yaw:");
  Serial.print(map(rcChannels[3], 1000, 2000, -100, 100));
  Serial.print(" AUX1:");
  Serial.print(rcChannels[4] > 1500 ? "HIGH" : "LOW");
  Serial.print(" AUX2:");
  Serial.print(rcChannels[5] > 1500 ? "HIGH" : "LOW");
  Serial.println();
}

void driveMotor(int pwmForward, int pwmReverse, int pinFwd, int pinRev) {
  ledcWrite(pinFwd, pwmForward);
  ledcWrite(pinRev, pwmReverse);
}

void setMotorSpeed(int motor, int speed) {
  // Constrain speed to valid range
  speed = constrain(speed, -255, 255);
  
  if (motor == 0) { // Right motor
    if (speed >= 0) {
      driveMotor(speed, 0, RPWM, LPWM);
    } else {
      driveMotor(0, -speed, RPWM, LPWM);
    }
  }
  if (motor == 1) { // Left motor
    if (speed >= 0) {
      driveMotor(speed, 0, RPWM_1, LPWM_1);
    } else {
      driveMotor(0, -speed, RPWM_1, LPWM_1);
    }
  }
}

// ---------- Setup Function ----------
void setup() {
  // Initialize Serial for debugging
  Serial.begin(115200);
  
  // Initialize Serial1 for IBUS communication
  // Baud rate: 115200, Data bits: 8, Parity: None, Stop bits: 1
  // RX pin: IBUS_RX_PIN, TX pin: not used (-1)
  Serial1.begin(115200, SERIAL_8N1, IBUS_RX_PIN, -1);

  ledcAttach(RPWM, freq, resolution);
  ledcAttach(LPWM, freq, resolution);
  ledcAttach(RPWM_1, freq, resolution);
  ledcAttach(LPWM_1, freq, resolution);

   // Motor enable pins
  pinMode(REN, OUTPUT);
  pinMode(LEN, OUTPUT);
  pinMode(REN_1, OUTPUT);
  pinMode(LEN_1, OUTPUT);

  // Enable motors
  digitalWrite(REN, HIGH);
  digitalWrite(LEN, HIGH);
  digitalWrite(REN_1, HIGH);
  digitalWrite(LEN_1, HIGH);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW); // Weapon OFF initially
  
  Serial.println("FSi6 IBUS Receiver Initialized");
  Serial.println("Waiting for IBUS data...");
  
  delay(1000);
}

// ---------- Main Loop ----------
void loop() {
  // Read IBUS data from FSi6 receiver
  readIbus();

  displayChannels();

  uint16_t ch1 = rcChannels[0]; // Steering (Roll)
  uint16_t ch2 = rcChannels[2]; // Throttle 
  uint16_t ch3 = rcChannels[4]; // Weapon switch (AUX1)
  
  if (ch1 < 800 || ch1 > 2200 || ch2 < 800 || ch2 > 2200) {
    setMotorSpeed(0, 0);
    setMotorSpeed(1, 0);
    digitalWrite(RELAY_PIN, LOW);
    Serial.println("Invalid IBUS signal - motors stopped");
    delay(100);
    return;
  }

  int throttle = map(ch2, 1000, 2000, -255, 255);
  int steering = map(ch1, 1000, 2000, -255, 255);

  // Calculate differential steering
  int leftMotor  = constrain(throttle + steering, -255, 255);
  int rightMotor = constrain(throttle - steering, -255, 255);

  // Apply motor speeds
  setMotorSpeed(0, rightMotor);  // Right motor
  setMotorSpeed(1, leftMotor);   // Left motor

  // Weapon control
  if (ch3 > 1500) {
    digitalWrite(RELAY_PIN, LOW);
    // Serial.println("Weapon ON");
  } else {
    digitalWrite(RELAY_PIN, HIGH);
    // Serial.println("Weapon OFF");
  }
  
  // Small delay to prevent excessive CPU usage
  delay(10);
}

