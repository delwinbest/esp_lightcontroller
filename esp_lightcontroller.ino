/*
  Highly adapted NEOPIXEL controller using aREST library.

  Written in 2016 by Delwin Best
*/

// Import required libraries
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <aREST.h>              //https://github.com/marcoschwartz/aREST
#include <Adafruit_NeoPixel.h>  //https://github.com/adafruit/Adafruit_NeoPixel


// Create aREST instance
aREST rest = aREST();
bool is_AP = true;
bool led_rainbow=true;


// The port to listen for incoming TCP connections
#define LISTEN_PORT           80


// Create an instance of the server
WiFiServer server(LISTEN_PORT);
#define LED_PIN D2
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 2
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, LED_PIN, NEO_GRB + NEO_KHZ800);


// Variables to be exposed to the API
int temperature;
int humidity;
uint16_t j=0;

// Declare functions to be exposed to the API
int ledBrightness(String value);
int rainbowControl(String value);

void config_AP ();



void setup(void)
{
  // Start Serial
  delay(1000);
  Serial.begin(115200);

  // Init variables and expose them to REST API
  temperature = 24;
  humidity = 40;


  rest.variable("temperature",&temperature);
  rest.variable("humidity",&humidity);

  // Function to be exposed
  rest.function("brightness",ledBrightness);
  rest.function("rainbow",rainbowControl);

  // Give name & ID to the device (ID should be 6 characters long)
  rest.set_id("1");
  rest.set_name("esp8266");
  config_AP();

  // Start the server
  server.begin();
  Serial.println("Server started");
  pixels.begin();

  pixels.setBrightness(20);
  pixels.show();
}

void loop(){
  if(led_rainbow==true){
    rainbow(20);
  }

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
int ledBrightness(String value) {
  Serial.print("Setting Brightness to ");
  Serial.println(value);
  uint8_t brightness = value.toInt();
  // Set Brightness to param value
  // Command: /led?params=20
  pixels.setBrightness(brightness);
  //pixels.setPixelColor(0, 127,127,127);
  //Serial.println("Setting LED to white");
  pixels.show();
  return 1;
}

// Custom function accessible by the API
// /rainbow?params=true
int rainbowControl(String value) {
  if (value == "true"){
    led_rainbow=true;
  }else {
    led_rainbow=false;
  }
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
    WiFi.softAPdisconnect();
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



void rainbow(uint8_t wait) {
  uint16_t i;

  for(i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, Wheel((i+j) & 255));
  }
  pixels.show();
  //Serial.print(".");
  delay(wait);
  j++;
  if (j==255){ j=0;}

}


// We need to create a fader, 3 colors, 0 - 255 per color, the spectrum looks like this:
// 0......125 .......255
// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return pixels.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return pixels.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return pixels.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}
