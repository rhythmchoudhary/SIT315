#define MOTION_SENSOR_PIN 2
#define POTENTIOMETER_PIN A0
#define LED_PIN 13

volatile bool motionDetected = false;

void motionISR() {
  motionDetected = true;
}

void setup() {
  pinMode(MOTION_SENSOR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR_PIN), motionISR, RISING);
  
  Serial.begin(9600);
  Serial.println("PIR + Potentiometer Multiple Inputs Board Ready");
}

void loop() {
  if (motionDetected) {
    Serial.println("Motion Detected!");
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    motionDetected = false;
  }
  
  int potValue = analogRead(POTENTIOMETER_PIN);
  Serial.print("Potentiometer Value: ");
  Serial.println(potValue);
  
  if (potValue > 700) { // Threshold for high reading
    digitalWrite(LED_PIN, HIGH);
    delay(300);
    digitalWrite(LED_PIN, LOW);
  }
}
