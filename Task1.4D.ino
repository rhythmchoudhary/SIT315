#define PIR1_PIN 2      // First PIR motion sensor
#define PIR2_PIN 4      // Second PIR motion sensor (new)
#define LED_PIN 7       // LED output
#define BUTTON_PIN 3    // Button input

volatile bool motionDetected1 = false;
volatile bool motionDetected2 = false;
volatile bool buttonPressed = false;
bool ledState = false;
unsigned long previousMillis = 0;
const long interval = 2000; // 2 seconds

// ISR for PIR1 (External Interrupt)
void motionISR1() {
  motionDetected1 = true;
}

// ISR for Button (External Interrupt)
void buttonISR() {
  buttonPressed = true;
}

void setup() {
  pinMode(PIR1_PIN, INPUT);
  pinMode(PIR2_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Internal pull-up enabled

  attachInterrupt(digitalPinToInterrupt(PIR1_PIN), motionISR1, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);

  Serial.begin(9600);
}

void loop() {
  // Check for motion on PIR2 manually (since PCINT is not supported in Tinkercad)
  if (digitalRead(PIR2_PIN) == HIGH) {
    motionDetected2 = true;
  }

  // Handle PIR1 event
  if (motionDetected1) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Motion detected on PIR1! LED ON.");
    motionDetected1 = false;
  }

  // Handle PIR2 event
  if (motionDetected2) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Motion detected on PIR2! LED ON.");
    motionDetected2 = false;
  }

  // Handle button press event
  if (buttonPressed) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Button pressed! LED OFF.");
    buttonPressed = false;
  }

  // Simulate Timer1 using millis()
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(LED_PIN, ledState);
  }
}
