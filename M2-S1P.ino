const byte LED_PIN = 13;
const byte METER_PIN = A4;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(METER_PIN, INPUT);

  Serial.begin(9600);
  
  // Initial timer frequency (0.5 Hz -> 2 sec per blink)
  startTimer(0.5);
}

void loop() { 
  // Read potentiometer value and map it to a frequency range (0.1 Hz to 2 Hz)
  int sensorValue = analogRead(METER_PIN);
  double timerFrequency = map(sensorValue, 0, 1023, 1, 20) / 10.0; // Range: 0.1 Hz to 2 Hz

  Serial.print("Timer Frequency: ");
  Serial.print(timerFrequency);
  Serial.println(" Hz");

  startTimer(timerFrequency);
  delay(500);  // Small delay to prevent excessive updates
}

void startTimer(double timerFrequency) {
  noInterrupts();  // Disable interrupts while configuring

  // Reset Timer1
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;  // Initialize counter

  // Calculate OCR1A value based on frequency
  double timerPeriod = 1.0 / timerFrequency; // Convert frequency to period
  int ocrValue = (16000000 / (1024 * (1 / timerFrequency))) - 1;  // Prescaler = 1024

  OCR1A = ocrValue;  

  // Set CTC (Clear Timer on Compare Match) mode
  TCCR1B |= (1 << WGM12);
  
  // Set 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  
  // Enable Timer Compare Interrupt
  TIMSK1 |= (1 << OCIE1A);

  interrupts();  // Re-enable interrupts
}

// Timer1 Compare Match Interrupt Service Routine
ISR(TIMER1_COMPA_vect) {
  digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toggle LED state
}
