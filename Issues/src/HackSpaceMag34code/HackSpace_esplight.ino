/*


  Author
  ------
  Demetrio PINNA
  dpinna.ca@gmail.com
  
  
  Creation Date
  -------------
  02.03.2020
  
  
*/


#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// **** PARAMETERS of your NETWORK / ACCESS POINT ****
// ***************************************************
const char* ssid = "**************";                     // Replace the “*” with the ssid     of your Access Point
const char* password = "**********";                     // Replace the “*” with the password of your Access Point
// ***************************************************
// ***************************************************
const char* mqtt_server = "broker.mqtt-dashboard.com";   // This is the MQTT Server you will use

WiFiClient espClient;
PubSubClient client(espClient);
const int relayPin = D1;

void AP_Connection() {
  delay(1000);
  Serial.print("Connecting to AP ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  randomSeed(micros());
  Serial.println();
  Serial.println();
  Serial.print("AP connected, IP Address : ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Command arrived : ");
  Serial.print((char)payload[0]);
  switch((char)payload[0]) {
    case '0':
     digitalWrite(BUILTIN_LED, HIGH);
     digitalWrite(relayPin, LOW);
     Serial.println(" --> the Light is OFF");
     client.publish("outTopicStatus", "The Light is OFF");
    break;
    case '1':
     digitalWrite(BUILTIN_LED, LOW);
     digitalWrite(relayPin, HIGH);
     Serial.println(" --> the Light is ON");
     client.publish("outTopicStatus", "The Light is ON");
    break;
    default:
     Serial.println(" --> UNKNOW COMMAND !");
    break;
  }
}

void reconnect() {
  while (!client.connected()) {
    Serial.println();
    Serial.print("Connecting to MQTT Broker : ");
    String clientId = "D3m3Tr10P1nN4";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("BROKER CONNECTED !");
      Serial.println();
      client.subscribe("inTopicCommand");
    }
    else {
      Serial.print("FAILED ! --> [rc=");
      Serial.print(client.state());
      Serial.println("]");
      Serial.println("Try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(115200);
  pinMode(relayPin, OUTPUT);
  AP_Connection();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
