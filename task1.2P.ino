#define PIR_PIN 2      // PIR motion sensor connected to digital pin 2
#define LED_PIN 7      // LED connected to digital pin 3
#define BUTTON_PIN 3   // Button connected to digital pin 4

volatile bool motionDetected = false;
volatile bool buttonPressed = false;

// ISR for PIR sensor
void motionISR() {
  motionDetected = true;
}

// ISR for button press
void buttonISR() {
  buttonPressed = true;
}

void setup() {
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Internal pull-up enabled

  attachInterrupt(digitalPinToInterrupt(PIR_PIN), motionISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
  
  Serial.begin(9600);
}

void loop() {
  if (motionDetected) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Motion detected! LED ON.");
    motionDetected = false;
  }

  if (buttonPressed) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("Button pressed! LED OFF.");
    buttonPressed = false;
  }
}
