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

//ADC_MODE(ADC_TOUT);
#include <EEPROM.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>  //https://github.com/adafruit/Adafruit_NeoPixel


// Set SERVER/REPEATER Variables which are consumed by mySensor.h
#define MY_DEBUG 
#define MY_BAUD_RATE 9600
#define MY_GATEWAY_ESP8266
#define MY_ESP8266_SSID "arduinowifi"
#define MY_ESP8266_PASSWORD "thereisnospoon"

#define MY_ESP8266_HOSTNAME "esp8266_bedroom"

#define MY_IP_GATEWAY_ADDRESS 192,168,1,1
#define MY_IP_SUBNET_ADDRESS 255,255,255,0
#define MY_PORT 5003      

#define MY_GATEWAY_MAX_CLIENTS 5

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
uint8_t CHILD1_DIMLEVEL=255;    //This needs to be an 8 bt int as we're relying on a rollover for brightness controll
String CHILD1_COLOR="FFFFFF";
int CHILDREN=2;
int LastLightState=LIGHT_OFF;
int LastDimValue=100; 

int sensorPin1 = D1;    // select the input pin for the button
int sensorPin2 = D4;
int sensorValue = 0;  // variable to store the value coming from the sensor

bool toggle = false;
MyMessage rgbMsg(CHILD1_ID, V_RGB);
MyMessage dimMsg(CHILD1_ID+1, V_PERCENTAGE);
MyMessage vlightMsg(CHILD1_ID, V_LIGHT);
MyMessage vlightMsg1(CHILD1_ID+1, V_LIGHT);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(STRIP1_NUMPIXELS, STRIP1_LEDPIN, NEO_GRB + NEO_KHZ800);



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

void setup() {
  delay(1000);
  Serial.begin(9600);
  pinMode(BUILTIN_LED, OUTPUT);

  pixels.begin();
  //request( CHILD1_ID, V_RGB );
  //request( CHILD1_ID+1, V_PERCENTAGE );
  restore_Eeprom();
  updateLEDStrip();

  
}


void loop() {
//  Serial.print("Pin1 cycle count: ");
  int cycles = readCapacitivePin(sensorPin1);
//  Serial.println(cycles);
  if(cycles > 0){
    handleButton(sensorPin1);
  }
  delay(100);

}


// Capcacitive Touch Detection
// The theory here is that if you have two pins connected by 800K+ resistor, the one an input and the other an output
// And you flip the output pin to HIGH, the time it takes the inpput pin to detect this change is dependent on the capacitance present
// on the the 'wire' ... which changes when you touch it.
// Routing taken from: https://github.com/juanpintom/Souliss_ESP_Examples/blob/master/E08_ESP_Capacitive_Sensor.ino

uint8_t readCapacitivePin(int pinToMeasure) {

  if (pinToMeasure == sensorPin1){
    pinMode(sensorPin2, INPUT);
  } else {
    pinMode(sensorPin1, INPUT);
  }
  
  pinMode(pinToMeasure, OUTPUT);
  digitalWrite(pinToMeasure, LOW);
  delay(1);
  // Prevent the timer IRQ from disturbing our measurement
  noInterrupts();
  // Make the pin an input with the internal pull-up on
  pinMode(pinToMeasure, INPUT_PULLUP);

  // Now see how long the pin to get pulled up. This manual unrolling of the loop
  // decreases the number of hardware cycles between each read of the pin,
  // thus increasing sensitivity.
  uint8_t cycles = 17;
       if (digitalRead(pinToMeasure)) { cycles =  0;}
  else if (digitalRead(pinToMeasure)) { cycles =  1;}
  else if (digitalRead(pinToMeasure)) { cycles =  2;}
  else if (digitalRead(pinToMeasure)) { cycles =  3;}
  else if (digitalRead(pinToMeasure)) { cycles =  4;}
  else if (digitalRead(pinToMeasure)) { cycles =  5;}
  else if (digitalRead(pinToMeasure)) { cycles =  6;}
  else if (digitalRead(pinToMeasure)) { cycles =  7;}
  else if (digitalRead(pinToMeasure)) { cycles =  8;}
  else if (digitalRead(pinToMeasure)) { cycles =  9;}
  else if (digitalRead(pinToMeasure)) { cycles = 10;}
  else if (digitalRead(pinToMeasure)) { cycles = 11;}
  else if (digitalRead(pinToMeasure)) { cycles = 12;}
  else if (digitalRead(pinToMeasure)) { cycles = 13;}
  else if (digitalRead(pinToMeasure)) { cycles = 14;}
  else if (digitalRead(pinToMeasure)) { cycles = 15;}
  else if (digitalRead(pinToMeasure)) { cycles = 16;}

  // End of timing-critical section
  interrupts();

  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.
  digitalWrite(pinToMeasure, LOW);
  pinMode(pinToMeasure, OUTPUT);

  return cycles;
}

void handleButton(int pin){
 

  // Right, with each pres, increase the light brightness by 33% (int 85)
  // If there's a roll over, switch the light off.
  // If the light is off, switch is on at 33%

  switch (CHILD1_STATE){
    case 0: {
      // Switch On
      CHILD1_STATE=1;
      CHILD1_DIMLEVEL=85;
      CHILD1_COLOR="FFFFFF"; 
      break;
    }
    case 1: {
      CHILD1_DIMLEVEL=CHILD1_DIMLEVEL+85; //Add 33%

      if( CHILD1_DIMLEVEL < 85 ){
        CHILD1_STATE=0;
        CHILD1_DIMLEVEL=1; // IF the 8 bit int rolls over, set it to one to avoid wierd crap
      } 
      break;
    }
    default: 
      // if nothing else matches, do the default
      // default is optional
    break;
  }
  updateLEDStrip();
  delay(500); // Delay between button press
  // Remember to ensure the button press is release to avoid debounce, this will cause problems later if I want to detect 
  // button hold for dimming
  while(readCapacitivePin(pin)){};
  return;
}



// Handle the V_Light Command from controller
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
      // Switch On
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

void updateController(){
  if (CHILD1_STATE==LIGHT_OFF) {
    send(vlightMsg.set(0));
    send(vlightMsg1.set(0));
  }
  else {
    send(vlightMsg.set(1));
    send(vlightMsg1.set(1));
    // We need to convert the DIMLEVEL to a PERCENTAGE for the controller:
    // Remember this: CHILD1_DIMLEVEL = map(atoi(message.data),0,100,0,255);  // ((255/100) * atoi(message.data)); // Max is 255, upscale from percentage
    send(dimMsg.set(int(map(CHILD1_DIMLEVEL,0,255,0,100))));
    send(rgbMsg.set(CHILD1_COLOR));
  }
  return;
}

// Save the LED state data and update the strip, by saving you can restore during reset
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
  // Always update the controller with the current 'real worls' state
  updateController();
  return;
}

// Custom function to set Brightness
// Remember: the pixel bringhtness range is 0 to 255, 
// Remember, the controller brightness range is a percentage. ugh
void setBrightness(uint8_t brightness) {
  Serial.print("Setting Brightness: ");
  Serial.println(String(brightness));
  //Serial.println(value);
  //uint8_t brightness = value.toInt();
  pixels.setBrightness(brightness);
  delay(50);
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
      CHILD1_DIMLEVEL = map(atoi(message.data),0,100,0,255);  // ((255/100) * atoi(message.data)); // Max is 255, upscale from percentage
      updateLEDStrip();
    }
  }
  return;
}


