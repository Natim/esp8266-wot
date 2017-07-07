/*
  Demonstrates how to do something with the ESP8266Kinto library
*/

// LinkNode D1
// Board: "WeMos D1(Retired)"
// Pin Mapping:
//      PIN GPIO
#define D0   3 //      RX0 (Serial console)
#define D1   1 //      TX0 (Serial console)
#define D2  16
#define D3   5 //      I2C Bus SCL (clock)
#define D4   4 //      I2C Bus SDA (data)
#define D5  14 //      SPI Bus SCK (clock) // LED notée SCK sur la carte LinkNode D1
#define D6  12 //      SPI Bus MISO
#define D7  13 //      SPI Bus MOSI
#define D8   0
#define D9   2 //      Same as "LED_BUILTIN", but inverted logic
#define D10 15 //      SPI Bus SS (CS)
#define D11 13 // = D7 SPI Bus MOSI
#define D12 12 // = D6 SPI Bus MISO
#define D13 14 // = D5 SPI Bus SCK (clock) // LED notée SCK sur la carte LinkNode D1
#define D14  4 // = D4 I2C Bus SDA (data)
#define D15  5 // = D3 I2C Bus SCL (clock)

#define LED_ESP 2  // D9, Same as "LED_BUILTIN", but inverted logic
#define LED_SCK 14 // D5 or D13 -> SCK blue LED on LinkNode D1 board

// a push button connected on the pin D2 (default = HIGH)
int pushButton = D2;
int lastButtonState = 1;
int lastLedState = 0;
int lastBuzzerFrequency = -1;

// LED on the pin D13
int led = D13;

// buzzer on the pin D8
int buzzer = D8;

void rencontreDuTroisiemeType() {
  tone(0, 292);
  delay(250);

  tone(0, 440);
  delay(250);

  tone(0, 349);
  delay(250);

  tone(0, 175);
  delay(250);

  tone(0, 262);
  delay(250);

  noTone(0);
}

// Kinto library
#include <ESP8266Kinto.h>

// We need a WiFi connection
#include <ESP8266WiFi.h>
#include <WiFiManager.h>

// Kinto object creation
Kinto kinto(
  "https://acklio.alwaysdata.net/v1", // server url
  "login",                            // login
  "password",                         // password
  "natim",                            // bucket
  "network1",                         // collection
  "6B:5C:80:34:F3:81:59:7F:B1:A3:40:00:84:86:7A:8D:0F:54:C2:B8" // server ssl fingerprint needed for a HTTPS connection
);

ESP8266WebServer server(80);

void handle_root() {
  server.send(200, "application/json",
      "{\"name\":\"My Thing\",\"type\":\"thing\",\"description\":\"Experimentation platform.\"," \
        "\"properties\":{" \
          "\"button\":{\"type\":\"string\",\"href\":\"/button\",\"enum\":[\"pressed\",\"released\"]}," \
          "\"buzzer\":{\"type\":\"integer\",\"href\":\"/buzzer\",\"unit\":\"frequency\",\"description\":\"Buzzer ( send -1 to stop )\"}," \
          "\"led\":{\"type\":\"boolean\",\"href\":\"/led\",\"description\":\"A red LED\"}" \
        "}" \
      "}");
  delay(100);
}

void handle_button() {
  if (lastButtonState == 0) {
    server.send(200, "application/json",
      "{\"button\":\"pressed\"}");
  } else {
    server.send(200, "application/json",
      "{\"button\":\"released\"}");
  }
  delay(100);
};

void handle_led() {
  lastLedState = lastLedState ? 0 : 1;
  if (lastLedState == 0) {
    digitalWrite(led, LOW);
    server.send(200, "application/json",
      "{\"led\":\"off\"}");
  } else {
    digitalWrite(led, HIGH);
    server.send(200, "application/json",
      "{\"led\":\"on\"}");
  }
  delay(100);
};


void handle_buzzer() {
  rencontreDuTroisiemeType();
  server.send(200, "application/json", "{\"buzzer\": -1}");
  delay(100);
};

void setup()
{
  pinMode(pushButton, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  // initialize serial communication
  Serial.begin(115200);

  // we start by connecting to a WiFi network
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP8266");

  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_root);
  server.on("/button", handle_button);
  server.on("/led", handle_led);
  server.on("/buzzer", handle_buzzer);
  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  int buttonState = digitalRead(pushButton);
  if (buttonState != lastButtonState) {
    if (buttonState == 0) {
      digitalWrite(LED_ESP, HIGH);
      Serial.println("pressed");
      kinto.patch("e9d6703a-7511-4351-9ce5-94690fbf8e6e", "{\"data\": {\"value\":\"pressed\"}}");
    }
    else {
      digitalWrite(LED_ESP, LOW);
      Serial.println("released");
      kinto.patch("e9d6703a-7511-4351-9ce5-94690fbf8e6e", "{\"data\": {\"value\":\"released\"}}");
    }
    delay(50); // debouncing
    lastButtonState = buttonState;
  }
  delay(1);
  server.handleClient();
}
