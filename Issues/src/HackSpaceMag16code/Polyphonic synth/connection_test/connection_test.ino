const int ADDRESS_SEND_PINS[] = {2,3,4};
const int ADDRESS_READ_PINS[] = {5,6,7};
const int CONNECTION_SEND_PIN = 10;
const int CONNECTION_READ_PIN = 8;

void setup() {
  for(int i=0;i<3;i++) {
    pinMode(ADDRESS_SEND_PINS[i], OUTPUT);
    pinMode(ADDRESS_READ_PINS[i], OUTPUT);
  }
  
  pinMode(CONNECTION_SEND_PIN, OUTPUT);
  digitalWrite(CONNECTION_SEND_PIN, LOW);

  pinMode(CONNECTION_READ_PIN, INPUT_PULLUP);

  Serial.begin(9600);
}

void loop() {
  for(int a=0;a<8;a++) {
    setSendChannel(a);
    for(int b=0;b<8;b++) {
      setReadChannel(b);
      delayMicroseconds(10);
      if(a < b) {
        boolean connectionReading = !digitalRead(CONNECTION_READ_PIN);
        if(connectionReading) {
          Serial.print(a);
          Serial.print(" is connected to ");
          Serial.print(b);
          Serial.print("\n");          
        }
      }
    }
  }
  delay(250);
}

void setSendChannel(int channel) {
  digitalWrite(ADDRESS_SEND_PINS[0], bitRead(channel, 0));
  digitalWrite(ADDRESS_SEND_PINS[1], bitRead(channel, 1));
  digitalWrite(ADDRESS_SEND_PINS[2], bitRead(channel, 2));
}

void setReadChannel(int channel) {
  digitalWrite(ADDRESS_READ_PINS[0], bitRead(channel, 0));
  digitalWrite(ADDRESS_READ_PINS[1], bitRead(channel, 1));
  digitalWrite(ADDRESS_READ_PINS[2], bitRead(channel, 2));
}

