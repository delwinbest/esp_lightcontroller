#include <Arduino.h>

/*
  This a simple example of the aREST Library for the ESP8266 WiFi chip.
  See the README file for more details.

  Written in 2015 by Marco Schwartz under a GPL license.
*/

// Import required libraries
#include <ESP8266WiFi.h>
#include <aREST.h>
#include "FastLED.h"


// Create aREST instance
aREST rest = aREST();
bool is_AP = false;


// The port to listen for incoming TCP connections
#define LISTEN_PORT           80
#define LED_DATA_PIN 3
// How many leds in your strip?
#define NUM_LEDS 1

// Create an instance of the server
WiFiServer server(LISTEN_PORT);
// Define the array of leds
CRGB leds[NUM_LEDS];

// Variables to be exposed to the API
int temperature;
int humidity;

// Declare functions to be exposed to the API
int ledControl(String command);

void config_AP ();



void setup(void)
{
  // Start Serial
  delay(1000);
  Serial.begin(115200);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;

  FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
  
  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("led",ledControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp8266");
  config_AP();

  // Start the server
  server.begin();
  Serial.println("Server started");

}

void loop(){
  
  leds[0] = CRGB::Red;
  FastLED.show();
  
  // Handle aREST calls
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  Serial.println("Client Connected. Waiting for Data:");

  while (client.connected()) {
    if (client.available()) {
      rest.handle(client);
      client.stop();
    } // if (client.available())
  } // while (client.connected())
} // void loop()



// Custom function accessible by the API
int ledControl(String command) {

  // Get state from command
  int state = command.toInt();

  digitalWrite(6,state);
  return 1;
}

// Either connect to AP or become AP
// This section allows you to either connect to a WiFi AP or
// become a WiFi AP (great for testing)
void config_AP () {
  Serial.println("");
  delay(50);
  if (is_AP==true){
    // WiFi parameters
    char* ssid = "esp8266AP";
    char* password = "thereisnospoon";
    // Setup WiFi network
    WiFi.softAP(ssid, password);
    Serial.println("WiFi created");

    // Print the IP address
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
  } else {
    // Connect to AP/ WiFi parameters
    char* ssid = "arduinowifi";
    char* password = "thereisnospoon";
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

