/*********************************************************************************************
 *  Purpose: ESP8266 device which binds to WiFi SSID and presents itself as a mySensor Device
 *  Current Design: ESP device is designed to ahve two NeoPixel LED strips attached
 *        Each Strip is explicitly referenced and hard coded, this is not OOP unfortunately
 *        The initial design will focus on controlling one strip, but allow for the easy 
 *        addition of the second.
 *  Author: Delwin Best
 *  Date: 19 September 2016
 *  
 *  *****************************************************************************************/

// NEXT STEPS: We want to set device states based on variables, this way we can track and refresh variables 
//             And seperate the data from the controll functions (good for external triggers).

#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>  //https://github.com/adafruit/Adafruit_NeoPixel

// Set SERVER/REPEATER Variables which are consumed by mySensor.h
#define MY_DEBUG 
#define MY_BAUD_RATE 9600

#define MY_GATEWAY_ESP8266
#define MY_ESP8266_SSID "XXX"
#define MY_ESP8266_PASSWORD "YYY"

#define MY_ESP8266_HOSTNAME "esp8266_bedroom"

#define MY_IP_GATEWAY_ADDRESS 192,168,1,1
#define MY_IP_SUBNET_ADDRESS 255,255,255,0
#define MY_PORT 5003      

#define MY_GATEWAY_MAX_CLIENTS 3

//#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_MODE_DURATION 60 
#define MY_INCLUSION_MODE_BUTTON_PIN  3 

#define MY_DEFAULT_ERR_LED_PIN BUILTIN_LED  // Error led 
#define MY_DEFAULT_RX_LED_PIN  BUILTIN_LED  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  BUILTIN_LED  // the PCB, on board LED

#include <ESP8266WiFi.h>


//This section relates to the 'CHILD' sensors reporting to the repeater, they are presented independently.
#define CHILD1_ID 1 
#define EPROM_CHILD1_STATE 1
#define EPROM_CHILD1_DIMLEVEL 2
#define EPROM_CHILD1_COLOR 3
#define STRIP1_NUMPIXELS 2
#define STRIP1_LEDPIN D2

#define LIGHT_OFF 0
#define LIGHT_ON 1

#include <MySensors.h>

int CHILD1_STATE=1;
int CHILD_ID;
int CHILD1_DIMLEVEL=255;
String CHILD1_COLOR="FFFFFF";
int CHILDREN=2;
int LastLightState=LIGHT_OFF;
int LastDimValue=100; 

MyMessage rgbMsg(CHILD1_ID, V_RGB);
MyMessage dimMsg(CHILD1_ID+1, V_PERCENTAGE);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STRIP1_NUMPIXELS, STRIP1_LEDPIN, NEO_GRB + NEO_KHZ800);
  
void setup() {
  delay(1000);
  Serial.begin(9600);
  pixels.begin();

  //request( CHILD1_ID, V_RGB );
  //request( CHILD1_ID+1, V_PERCENTAGE );
  restore_Eeprom();
  updateLEDStrip();

  
}

// mySensor device presentation configuration
void presentation() {
  // Present locally attached sensors here
  present(0, S_ARDUINO_REPEATER_NODE, "ESP8266_lightcontroller", true);
  present(CHILD1_ID, S_RGB_LIGHT, "RGB", true );
  present(CHILD1_ID+1, S_DIMMER, "RGB", true );
  sendSketchInfo("esp8266_lighting", "1.0");
  typedef enum {
    S_RGB_LIGHT, S_DIMMER
  } sensor;
  typedef enum {
    V_RGB, V_PERCENTAGE, V_WATT
  } variableType;
  sleep(100);
  return;  
}


void loop() {


  
}

// main receive function for the mySensor receive handling
void receive(const MyMessage &message)
{
  //Send msg infoprmation
  Serial.print("node-id: ");Serial.println(message.sender);
  Serial.print("destination-id: ");Serial.println(message.destination);
  Serial.print("child-sensor-id: ");Serial.println(message.sensor);
  Serial.print("message-type: ");Serial.println(message.type);
  Serial.print("data: ");Serial.println(message.data);

  if ((message.sensor == CHILD1_ID) || (message.sensor == CHILD1_ID+1)){
    if (message.type == V_LIGHT) {
      Serial.println( "V_LIGHT command received..." );
      handleVLIGHT(1, message.data);
    }
    if (message.type == V_RGB) {
      Serial.println( "V_RGB command received..." );
      CHILD1_COLOR=message.data;
      setColor(CHILD1_COLOR);   // I still need to stop CHILD1_COLOR in eeprom and change to updateledstrip();
    }
    if (message.type == V_PERCENTAGE) {
      Serial.println( "V_PERCENTAGE command received..." );
      CHILD1_STATE=1;
      CHILD1_DIMLEVEL = ((255/100) * atoi(message.data)); // Max is 255, upscale from percentage
      updateLEDStrip();
    }
  }
  return;
}

void handleVLIGHT(uint8_t CHILD, String data){
  //Doing this for Child 1 for now
  //Extend using CHILD variable
  uint8_t state = data.toInt();
  switch (state){
    case 0:
    {
      // Switch Off
      CHILD1_STATE=0; 
      break;
    }
    case 1:
    {
      // Switch Off
      CHILD1_STATE=1; 
      break;
    }
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }

  updateLEDStrip();
  //updateController
  return; 
}

void SendCurrentState2Controller(){
  if ((LastLightState==LIGHT_OFF)||(LastDimValue==0)) {
    send(rgbMsg.set(0));
  }
  else {
    send(rgbMsg.set("ffffff"));
  }
  return;
}

//
void updateLEDStrip(){
  save_Eeprom();  
  switch (CHILD1_STATE){
    case 0:
    {
      //pixels.clear();
      setColor("0");     //Setting this to 0 disabled the LED strip. will need to reinitialise 
      break;
    }
    case 1:
    {
      pixels.begin();
      setColor(CHILD1_COLOR);
      setBrightness(CHILD1_DIMLEVEL);
      break;
    }
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }
  return;
}

// Custom function to set Brightness
void setBrightness(uint8_t brightness) {
  Serial.print("Setting Brightness: ");
  Serial.println(String(brightness));
  //Serial.println(value);
  //uint8_t brightness = value.toInt();
  pixels.setBrightness(brightness);
  pixels.show();
  delay(100);
  return; 
}

// Convert HTML color code into RGB values
// and set strip
void setColor(String value) {
  Serial.print("Setting Color To: ");
  Serial.println(value);
  long number = (long) strtol( &value[0], NULL, 16);
  int red = number >> 16;
  int green = number >> 8 & 0xFF;
  int blue = number & 0xFF;
  uint16_t i;

  for(i=0; i<pixels.numPixels(); i++) {
    pixels.setPixelColor(i, red, green, blue);
  }
  
  pixels.show();
  delay(100);
  return;
}

void save_Eeprom(){
  saveState(EPROM_CHILD1_STATE, CHILD1_STATE);
  saveState(EPROM_CHILD1_DIMLEVEL,CHILD1_DIMLEVEL);
  // Save Color Array
  for(int i=0; i<CHILD1_COLOR.length(); i++) { 
    saveState(EPROM_CHILD1_COLOR+i,CHILD1_COLOR[i]);
  }
}

void restore_Eeprom(){
  CHILD1_STATE=loadState(EPROM_CHILD1_STATE);
  CHILD1_DIMLEVEL=loadState(EPROM_CHILD1_DIMLEVEL);
  // Save Color Array
  for(int i=0; i<CHILD1_COLOR.length(); i++) { 
    CHILD1_COLOR[i]=loadState(EPROM_CHILD1_COLOR+i);
  }
}


