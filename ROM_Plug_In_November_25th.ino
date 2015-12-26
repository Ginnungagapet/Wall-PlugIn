word powerValue = 0;                            // Packet of Data to send to the database                  // Amps
word interval = 3000;                           // Interval at which to sample the circuit (ms)
unsigned long previousTime = 0;                 // The last time you sampled the circuit (ms)
byte count = 0;
char data;
int switchPin = 8;
char name[24];
boolean switchPinHigh = false;

void setup() {
  // initialize serial communication at 9600 bits per second
  Serial.begin(9600);
  Serial.setTimeout(1500);
  delay(500);
  extern char name[24];
  readNodeName(name);
  
  pinMode(switchPin, OUTPUT);                          // Set Pin 8 to output for switching circuit
  digitalWrite(switchPin, LOW);
}

void loop() {
  
  unsigned long currentTime = millis();
  
  if (!Serial.available()) {                           // If there is no data waiting in the Xbee buffer
    if (currentTime - previousTime > interval)
      readData(currentTime);
    if (count >=10)                                    // If the count is at 10, at least 30 seconds has passed, then send the value to the Raspberry Pi
      transmitPowerData();
  }
  else {
    data = Serial.read();
    if (data == 'r')
      renameNode();
    else if (data == 's' && switchPinHigh == false){
      digitalWrite(switchPin, HIGH);
      Serial.write("Pin switched to high");
      switchPinHigh = true;
    }
    else if (data == 's' && switchPinHigh == true){
      digitalWrite(switchPin, LOW);
      Serial.write("Pin switched to low");
      switchPinHigh = false;
    }
  }
}

/*
 * A function called every 3 seconds to read the voltage and calculate the power
 */
void readData(unsigned long currentTime) {
  previousTime = currentTime;
  // read the input on analog pin A3
  word voltageReading = analogRead(A3);
  delay (10);                                      // Delays allow the ADC to settle before the next command
  // Convert the analog reading to the output current of the wall
  word current = (voltageReading * 73.3 / 1023.0) - 36.7;
  powerValue += current * 110;
  count++;
}

/*
 * A function to transmit the power data that has been compiiled and calculated
 */
void transmitPowerData() {
  powerValue = powerValue / count;
  extern char name[24];
  Serial.print(name);
  Serial.write("-Power Reading: ");
  Serial.println(powerValue);
  count = 0;                                 // Reset the count and power reading
  powerValue = 0;
}

/*
 * A small function for reading the name of the Xbee this controller is attached to
 */
void readNodeName(char* name) {
  extern char name[24];                     // Declaration of external name variable
  
  Serial.print("+++");
  delay(1000);
  data = Serial.read();
  data = Serial.read();
  data = Serial.read();                     // After '+++' is sent you need to empty the buffer
  delay(200);
  Serial.println("ATNI");                   // Send the AT command to read the node name
  delay(200);
  if(Serial.available() > 0)
    Serial.readBytesUntil('\n', name, 24);  // Store it in the string
}

/*
 * A small function for renaming the Xbee this controller is attached to
 */
void renameNode() {
  char newName[24];
  data = Serial.read();
  if(Serial.available() > 0)
    Serial.readBytesUntil('\n', newName, 24);  // Store it in the string
  
  Serial.print("+++");
  delay(1000);
  data = Serial.read();
  data = Serial.read();
  data = Serial.read();                      // After '+++' is sent you need to empty the buffer
  delay(200);
  Serial.print("ATNI ");                     // Send the AT command to change the node name
  Serial.println(newName);
  delay(200);
}
