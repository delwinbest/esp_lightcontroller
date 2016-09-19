ESP8266 Open Source Lighting Controller
==========
.. image:: https://travis-ci.org/delwinbest/esp_lightcontroller.svg?branch=master
    :target: https://travis-ci.org/delwinbest/esp_lightcontroller
    :alt: Travis.CI Build Status

Another home automation project using $4 ESP8266 device with Platformio compiler and HTTP aREST interface.

Intro

I was looking for a cheap and inclusive micro I could use in with my existing Domoticz home automation system. I've experimented with raspberry pi (Nano) and the nRF24L01 board and chip for comms but found myself writing chucks of code just to support the basic platform and API.

A month later I discovered the WeMos D1 (http://www.wemos.cc/Products/d1_mini.html). Cheap as chips with the following specs:

Microcontroller ESP-8266EX Operating Voltage 3.3V Digital I/O Pins 11 Analog Input Pins 1(Max input: 3.2V) Clock Speed 80MHz/160MHz Flash 4M bytes

The only downside here is the clock speed, but for it's use case it's not a concern.

System Design

MCU: ESP-8266EX with the Arduino compiler/interface (this is a $%#$@ to get set up on OSX, but easy on Windows, just search for the 'how to's'.

WIFI: ESP8266WiFi library included in Platformio configured project directory when selecting the WeMos Mini D1 device

API: Opensource mySensor API (https://github.com/mysensors/MySensors).

Installing & Getting Started

Install Arduino App, recommend Windows for this.

More to come.....

Disclaimer: Please excuse in spelling/grammar mistakes, this is a bit of a braindump.
