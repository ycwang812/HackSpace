#include <Servo.h>;
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// Network settings
#ifndef STASSID
#define STASSID "WIFI-NAME-GOES-HERE"
#define STAPSK  "WIFI-PASSWORD-GOES-HERE"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

// pin assignments
int pot_pin = 5; // potentiometer pin
int therm_pin = 4; // thermistor pin
int heater_pin = 14; // heater power pin
int servo_pin = 12; // servo signal pin


// general
int targetTemp = 18; // target temperature for room
int hyst = 2; // tolerance for error between target temp and actual temp
bool heaterON = false; // state of the solid state relay
unsigned long last_tick = millis(); // setup a clock to cut down on process requests 
unsigned long tick_duration = 1000; // how long between ticks of the clock
Servo dialServo;

// Webserver page handling
void handleRoot() {
  int tmp;
  // if we're setting the temperature
  if (server.hasArg("temp")){
    tmp = server.arg("temp").toInt();
    // sanity check the temp range, don't allow setting it too high or low.
    if (tmp >= 5 and tmp <= 20){
      targetTemp = server.arg("temp").toInt();
      Serial.print("Changing target temp to: ");
      Serial.println(targetTemp);
      setServo();
    } else {
      // bounce back the request if there's an error and then exit the function
      String message = "Temperature is outside safe limits. manual setting needed above 20c";
      server.send(200, "text/html", message);
      return;
    }
  }

  // create form with slider and use it to show and set the temperature
  String message = "<html><header><title>Shedlandia Heating</title></header><body>";
  message += "<h1>Temperature Control</h1>";
  message += "<h2>Current Setting</h2>";
  message += "<p>Actual: ";
  message += String(heatCheck());
  message += "</p><p>Target: ";
  message += String(potCheck());
  message += "</p><h2>Remote Control</h2> ";
  message += "<form action=\"\" method=\"POST\">";
  message += "<input type= \"range\" min=\"5\" max=\"20\" value=\"";
  message += String(targetTemp);
  message += "\" name=\"temp\"/><br/>";
  message += "<button type=\"submit\">Set Temp</button>";
  message += "</form></body></html>";

  server.send(200, "text/html", message);

}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

//
//
// ANALOG DATA HANDLING
int heatCheck(){
  double tempC;

  // turn on the power to the thermistor
  digitalWrite(pot_pin, LOW);  
  digitalWrite(therm_pin, HIGH);   
  delay(50);              // wait for pin to rise
  
  // read the thermistor
  int rawADC = analogRead(A0)+97;

  // turn off the power to the thermisor
  digitalWrite(pot_pin, LOW);   
  digitalWrite(therm_pin, LOW);   
  
  tempC = log(((10240000/rawADC) - 10000));
  tempC = 1 / (0.001129148 + (0.000234125 + (0.0000000876741 *tempC *tempC ))*tempC );
  tempC -= 273.15;

  Serial.print(rawADC);
  Serial.print(" : ");
  Serial.println(tempC);
  return(int(tempC));
  }

int potCheck(){
  double tempC;

  // turn on the power to the potentiometer
  digitalWrite(pot_pin, HIGH);  
  digitalWrite(therm_pin, LOW); 
  delay(50);           // wait for pin to rise
  
  // read the pot
  int rawADC = analogRead(A0);

  // turn off the power to the potentiometer
 
  digitalWrite(pot_pin, LOW);   
  digitalWrite(therm_pin, LOW);  

  // remap the dial to centigrade
  tempC = map(rawADC,0,1024,5,40);

  Serial.print(rawADC);
  Serial.print(" : ");
  Serial.println(tempC);
  return(int(tempC));
  }

void setServo(){
  // called when webpage changes temperature
  // attaches the servo, moves it to new temperature, then detaches it again.
  int pos;

  // power up the servo
  dialServo.attach(12);
  
  delay(500);
  // if heater on, start at top and sweep down
  // otherwise start at bottom and sweep up.
    pos = 0;
    while (pos < 180) {
      dialServo.write(pos);
      if ((potCheck() > targetTemp - hyst) and (potCheck() < targetTemp + hyst)){
        dialServo.detach();
        return;
        }
      pos+= 5;  
      delay(10);
  }
  // power down the servo, just in case target temp couldn't be found.
  dialServo.detach();
  }

void setHeater(){
  // checks the temperature and sets the heater status to on or off
  int current_temp = heatCheck();
  if (current_temp >= targetTemp){
     digitalWrite(heater_pin,LOW);
     Serial.println("Heater has turned off");
    } else if (current_temp <= targetTemp-hyst) { // allow some temperature drop to stop fluttering between on/off state
     digitalWrite(heater_pin,HIGH);
     Serial.println("Heater has turned on");
    }
  }
  
void setup(){
  Serial.begin(115200);
  
  // WIFI CONNECT & WEBSERVER
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("CONNECTING TO WIFI");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }
  
  // set page handlers and start the web server
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();

  // initialize digital pins
  pinMode(2, OUTPUT);
  pinMode(heater_pin,OUTPUT);
  pinMode(pot_pin, OUTPUT);
  pinMode(therm_pin, OUTPUT);
  digitalWrite(heater_pin,LOW);
  }

void loop() {
  // do the web based things every loop
  server.handleClient();
  MDNS.update();

   // do other events when clock tick happens
  if (millis()-last_tick > tick_duration){
    targetTemp = potCheck(); // update target temperature based on dial position
    setHeater(); // set the heater state to on or off
    last_tick = millis(); // update when the last tick happened
    }
}
