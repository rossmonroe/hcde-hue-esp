/*************************************************************
Hue @ D1 mini Pro (ESP8266)
Basic demo of switch based on IR sensor to demonstrate 
communication between ESP and Hue gateway
Version: 1.00
by Petr Lukas
Functionality:
Identify IR signal and switch light on and off using IR remote control.
*************************************************************/
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#include <Wire.h>

#include <Adafruit_Sensor.h>
#include "Adafruit_Si7021.h"


// Wifi network SSID
const char* ssid = "University of Washington";
// Wifi network password
const char* password = "YOUR_PASSWORD";

// IP of Hue gateway
String ip = "172.28.219.225";
// Hue gateway user name
String user_name = "lxjNgzhDhd0X-qhgM4lsgakORvWFZPKK70pE0Fja";

bool state = false;

int button = 13;

typedef struct {
  String one; //Status of data call
} lightLevel;

lightLevel light;

//AUDIO INFO
const int sampleWindow = 50; // Sample window width in mS (50 mS = 20Hz)
unsigned int sample;

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password); 
  pinMode(button, INPUT);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to the WiFi network"); 
 
}

void loop() {
    if (digitalRead(button) == LOW){
      Serial.println();
      Serial.println("Button 1 pressed");
      switchLight(1,true);
      delay(2000);
    }
  if(light.one){
      switchLight(1,false); 
  }


  microphoneCheck();


  
  delay(100);
}

void checkLight(byte room, bool current_state){ 
  state = current_state;
  HTTPClient http; 
  String req_string;
  req_string = "http://";
  req_string += ip;
  req_string += "/api/";
  req_string += user_name;
  req_string += "/lights/";
  req_string += room;
  req_string += "/state";
  //Serial.println(req_string);
  http.begin(req_string);
  http.addHeader("Content-Type", "text/plain");
  
  String put_string;
  put_string = "{\"on\":";
  put_string += (current_state)? "true" : "false";
  put_string += "}";
  HTTPClient theClient;
  Serial.println();
  Serial.println("Getting Air Quality Feed...");
  theClient.begin(req_string);
  int httpCode = theClient.GET();
  if (httpCode > 0) {
    if (httpCode == 200) {
      DynamicJsonBuffer jsonBuffer;
      String payload = theClient.getString();
      JsonObject& root = jsonBuffer.parse(payload);

      // Test if parsing succeeds.
      if (!root.success()) {
        Serial.println("parseObject() failed");
        Serial.println(payload);
        return;
      }
      light.one = root["state"].as<String>(); //If it grabbed data successfully
    }
    else {
      Serial.println(httpCode);
      Serial.println("Something went wrong with connecting to the endpoint.");
    }
  }
  
   http.end();
}


void switchLight(byte room, bool current_state){
  state = current_state;
  HTTPClient http; 
  String req_string;
  req_string = "http://";
  req_string += ip;
  req_string += "/api/";
  req_string += user_name;
  req_string += "/lights/";
  req_string += room;
  req_string += "/state";
  //Serial.println(req_string);
  http.begin(req_string);
  http.addHeader("Content-Type", "text/plain");
  
  String put_string;
  put_string = "{\"on\":";
  put_string += (current_state)? "true" : "false";
  put_string += "}";
  
  int httpResponseCode = http.PUT(put_string);
  
  if(httpResponseCode > 0){
    String response = http.getString();   
    Serial.println(httpResponseCode);
    Serial.println(response);          
   } else {
    Serial.print("Error on sending PUT Request: ");
    Serial.println(httpResponseCode);
   }
   http.end();
}


void microphoneCheck(){
  unsigned long startMillis= millis();  // Start of sample window
   unsigned int peakToPeak = 0;   // peak-to-peak level
 
   unsigned int signalMax = 0;
   unsigned int signalMin = 1024;
 
   // collect data for 50 mS
   while (millis() - startMillis < sampleWindow)
   {
      sample = analogRead(A0);
      if (sample < 1024)  // toss out spurious readings
      {
         if (sample > signalMax)
         {
            signalMax = sample;  // save just the max levels
         }
         else if (sample < signalMin)
         {
            signalMin = sample;  // save just the min levels
         }
      }
   }
   peakToPeak = signalMax - signalMin;  // max - min = peak-peak amplitude
   double volts = (peakToPeak * 5.0) / 1024;  // convert to volts
 
   //Serial.println(volts);

   if(volts < 10){
      switchLight(2, true);
      Serial.println();
      Serial.println("LOUD NOISE DETECTED");
      Serial.println(volts);
      Serial.println();
      delay(2000);
      switchLight(2,false);
   }
}

