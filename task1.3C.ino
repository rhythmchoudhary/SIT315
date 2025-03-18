const int pirPin = 2;       // PIR sensor connected to digital pin 2
const int ledPin = 13;      // LED connected to digital pin 13
const int moisturePin = 3;  // Soil Moisture sensor connected to digital pin 3

volatile bool motionDetected = false;
volatile bool moistureDetected = false;

void motionISR() {
    motionDetected = true;
}

void moistureISR() {
    moistureDetected = true;
}

void setup() {
    pinMode(pirPin, INPUT);
    pinMode(moisturePin, INPUT);
    pinMode(ledPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(pirPin), motionISR, RISING);
    attachInterrupt(digitalPinToInterrupt(moisturePin), moistureISR, RISING);
    Serial.begin(9600);
}

void loop() {
    if (moistureDetected) {
        Serial.println("Moisture detected! Interrupting motion detection.");
        moistureDetected = false;  // Reset flag
    } 
    else if (motionDetected) {
        digitalWrite(ledPin, HIGH);
        Serial.println("Motion detected!");
        delay(1000);
        digitalWrite(ledPin, LOW);
        motionDetected = false;  // Reset flag
    }
}
