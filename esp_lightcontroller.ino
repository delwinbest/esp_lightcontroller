#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>  //https://github.com/adafruit/Adafruit_NeoPixel

#define MY_DEBUG 
#define MY_BAUD_RATE 9600

// Enables and select radio type (if attached)


#define MY_GATEWAY_ESP8266
#define MY_ESP8266_SSID "arduinowifi"
#define MY_ESP8266_PASSWORD "thereisnospoon"

#define MY_ESP8266_HOSTNAME "esp8266_bedroom"

#define MY_IP_GATEWAY_ADDRESS 192,168,1,1
#define MY_IP_SUBNET_ADDRESS 255,255,255,0
#define MY_PORT 5003      

#define MY_GATEWAY_MAX_CLIENTS 3


#define MY_INCLUSION_MODE_FEATURE
#define MY_INCLUSION_MODE_DURATION 60 
#define MY_INCLUSION_MODE_BUTTON_PIN  3 

#define MY_DEFAULT_ERR_LED_PIN BUILTIN_LED  // Error led 
#define MY_DEFAULT_RX_LED_PIN  BUILTIN_LED  // Receive led pin
#define MY_DEFAULT_TX_LED_PIN  BUILTIN_LED  // the PCB, on board LED
#define NUMPIXELS 2
#define LED_PIN D2


#if defined(MY_USE_UDP)
  #include <WiFiUDP.h>
#else
  #include <ESP8266WiFi.h>
#endif




#define CHILD1_ID 1 
#define EPROM_CHILD1_STATE 1
#define EPROM_CHILD1_DIMLEVEL 2
#define EPROM_CHILD1_COLOR 3

#define LIGHT_OFF 0
#define LIGHT_ON 1

#include <MySensors.h>

int LastLightState=LIGHT_OFF;
int LastDimValue=100; 

MyMessage rgbMsg(CHILD1_ID, V_RGB);
MyMessage dimMsg(CHILD1_ID+1, V_PERCENTAGE);

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(2, LED_PIN, NEO_GRB + NEO_KHZ800);
  
void setup() {
  
  delay(1000);
  Serial.begin(9600);
  pixels.begin();

  request( CHILD1_ID, V_RGB );
  request( CHILD1_ID+1, V_PERCENTAGE );

}

void presentation() {
  // Present locally attached sensors here
  present(0, S_ARDUINO_REPEATER_NODE, "ESP8266_lightcontroller", true); //The number 18 signifies S_ARDUINO_REPEATER_NODE
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
}


void loop() {


}

void receive(const MyMessage &message)
{

  //Send msg infoprmation
  Serial.print("node-id: ");Serial.println(message.sender);
  Serial.print("destination-id: ");Serial.println(message.destination);
  Serial.print("child-sensor-id: ");Serial.println(message.sensor);
  Serial.print("message-type: ");Serial.println(message.type);
  Serial.print("data: ");Serial.println(message.data);

  if ((message.sensor == CHILD1_ID) || (message.sensor == CHILD1_ID+1)){
  
    if (message.type == V_RGB) {
      Serial.println( "V_RGB command received..." );
      setColor(message.data);
    }
    if (message.type == V_PERCENTAGE) {
      Serial.println( "V_PERCENTAGE command received..." );
      ledBrightness(message.data);
    }
    if (message.type == V_LIGHT) {
      Serial.println( "V_LIGHT command received..." );
      ledBrightness(message.data);
    }
  }

}



void SendCurrentState2Controller()
{
  if ((LastLightState==LIGHT_OFF)||(LastDimValue==0)) {
    send(rgbMsg.set(0));
  }
  else {
    send(rgbMsg.set("ffffff"));
  }
}



// Custom function to set Brightness
int ledBrightness(String value) {
  Serial.print("Setting Brightness to ");
  Serial.println(value);
  uint8_t brightness = value.toInt();
  // Set Brightness to param value
  if (brightness == 0){
    setColor("0");
  } else if (brightness == 1 ){
    setColor("FFFFFF");
  } else {
    pixels.setBrightness(brightness);
    pixels.show();
  }
}

// Convert HTML color code into RGB values
// and set strip
int setColor(String value) {
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

}



