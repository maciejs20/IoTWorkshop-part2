#define LED 2            

void setup() {
  Serial.begin(115200);
  Serial.println ("");
  pinMode(LED, OUTPUT);    // LED pin as output.
  Serial.println ("Start");
}

void loop() {
  digitalWrite(LED, HIGH);                           
  delay(1000);              // wait for 1 second.
  digitalWrite(LED, LOW);   // turn the LED on.
  delay(1000);              // wait for 1 second.
  Serial.println ("Loop");
}
