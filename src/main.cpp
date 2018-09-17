#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

const char* ssid = "Orange-D8EE";
const char* password = "1234Airbox";
const int GPIO_NUMBER = 3;

const int gpio_pin[10] = {16,5,4,0,2,14,12,13,15,17};

ESP8266WebServer server(80);

const int led = 2;

const int gpio[GPIO_NUMBER] = {D0, D1, D2};

int sensor = D7;              // the pin that the sensor is atteched to
int state = LOW;             // by default, no motion detected
int val = 0;                 // variable to store the sensor status 

void handleRoot();
void handleNotFound();
void start();
void motion();

void setup(void){
  pinMode(led, OUTPUT);
  pinMode(sensor, INPUT);

  for(int i=0; i<GPIO_NUMBER; i++){
    pinMode(gpio[i], OUTPUT);
    digitalWrite(gpio[i], LOW);
  }
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
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

  start();
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void){
  server.handleClient();
  motion();
}

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", "hello from esp8266!");
  digitalWrite(led, 0);
}

void handleNotFound(){
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void start() {

  // 1 GET 
  // 2 POST
  server.on("/", handleRoot);

  server.on("/inline", [](){
    server.send(200, "text/plain", "this works as well");
  });

  //Get states , Method POST
  server.on("/gpio/state", HTTPMethod(2) , [](){
    // if(server.hasArg('pin')){
      int pin_number = atoi(server.arg(String("pin").c_str()).c_str()); // Get arg from request
      int pin_number_gpio = gpio_pin[pin_number]; // Convert received pin into gpio pin
      String message = "{ \"pin\" : \"";
      message += String(pin_number);
      message +=  "\" , \"state\": \"";
      message += String(digitalRead(pin_number_gpio));
      message += "\"}";
      server.send(200, "application/json", message ); //Get the state of the pin_number
    // }
  });

  //Set states. Method POST
  server.on("/gpio/set", HTTPMethod(2), [](){
    // if(server.hasArg(pin) && server.hasArg(to)){
      int pin_number = atoi(server.arg(String("pin").c_str()).c_str()); //pin number received correspond to D0, D1
      int pin_number_gpio = gpio_pin[pin_number]; // Convert received pin into gpio pin
      int pin_state = atoi(server.arg(String("to").c_str()).c_str());
      digitalWrite(pin_number_gpio, pin_state);
      server.send(200, "text/plain", "pin " + String(pin_number) + " set to " + String(pin_state));
    // }
  });

  // server.on("/dht", [](){
  //   dht_sensor();
  //   String message = "Temperature: ";
  //   message += celsiusTemp;
  //   message += "\nHumidity: ";
  //   message += humidityTemp;
  //   server.send(200, "text/plain", message);
  // });

  server.onNotFound(handleNotFound);
}

void motion(){
  val = digitalRead(sensor);   // read sensor value
  if (val == HIGH) {           // check if the sensor is HIGH
    digitalWrite(led, HIGH);   // turn LED ON
    delay(100);                // delay 100 milliseconds 
    
    if (state == LOW) {
      Serial.println("Motion detected!"); 
      state = HIGH;       // update variable state to HIGH
    }
  } 
  else {
      digitalWrite(led, LOW); // turn LED OFF
      delay(200);             // delay 200 milliseconds 
      
      if (state == HIGH){
        Serial.println("Motion stopped!");
        state = LOW;       // update variable state to LOW
    }
  }
}