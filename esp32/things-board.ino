#include <WiFi.h>
#include "time.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <HTTPClient.h>
#include <stdio.h>
#include <stdlib.h>
#include <ArduinoJson.h>

#define TEMPRATURE_MAX_VALUE 50
#define TEMPRATURE_MIN_VALUE 5

#define HUMIDITY_MAX_VALUE 100
#define HUMIDITY_MIN_VALUE 0

#define MOISTURE_MAX_VALUE 100
#define MOISTURE_MIN_VALUE 0

#define LIGHT_MAX_VALUE 100
#define LIGHT_MIN_VALUE 0

#define EC_MAX_VALUE 100
#define EC_MIN_VALUE 0

#define PH_MAX_VALUE 14
#define PH_MIN_VALUE 0

#define USERNAME "tenant@thingsboard.org"
#define PASSWORD "tenant"

const char* ssid     = "Mehr";
const char* password = "910093mehr";
const char* thingsBoardUrl = "http://45.138.132.164:8080/api/plugins/telemetry/DEVICE/8e70e2b0-1c9d-11ee-a44e-b9f1484bae02/values/timeseries?keys=lightStatus,control_key";
const char* authUrl = "http://45.138.132.164:8080/api/auth/login";
const char* sendDataPath = "http://45.138.132.164:8080/api/v1/esp32/telemetry";
const char* thingsBoardBaseUrl = "http://45.138.132.164:8080/api";
const char* sendTimeSeriesDataPath = "%f/v1/%f/telemetry";
const char* getLatestDataPath = "%f/plugins/telemetry/DEVICE/%f/values/timeseries?keys=lightStatus";
const char* loginUrlPath = "%f/auth/login";
const char* deviceId = "8e70e2b0-1c9d-11ee-a44e-b9f1484bae02";
const char* deviceToken = "esp32";

String token = " ";


#define RELAY_PIN 25
#define LED_PIN 26


void sendToServer() {

  DynamicJsonDocument doc_auth(2048);
  doc_auth["username"] = "tenant@thingsboard.org";
  doc_auth["password"] = "tenant";

  String json_auth;
  serializeJson(doc_auth, json_auth);

  WiFiClient client_auth;  // or WiFiClientSecure for HTTPS
  HTTPClient http_auth;

  http_auth.begin(client_auth, authUrl);
  http_auth.POST(json_auth);
  Serial.println("auth token from server...");
  // Serial.println(http_auth.getString());
  
  DynamicJsonDocument auth_result(10096);
  DeserializationError error = deserializeJson(auth_result, http_auth.getString());

  // String token = http_auth.getString().substring(10, 582);
  String token = auth_result["token"];
  http_auth.end();


  


  // Serial.println("test");


  WiFiClient client;  // or WiFiClientSecure for HTTPS
  HTTPClient http;

  http.begin(client, thingsBoardUrl);
  // Serial.println(auth_result);
  // Serial.println(token);
  http.addHeader("X-Authorization", String("Bearer ") + token);
  // Serial.println("test");
  http.GET();
  Serial.println("get state...");
  // Serial.println(http.getString());
    DynamicJsonDocument jsonDoc(2048);
   error = deserializeJson(jsonDoc, http.getString());
  
  if (error) {
Serial.print(F("deserializeJson() failed with code "));
Serial.println(error.f_str());
}

  http.end();

  String state = jsonDoc["lightStatus"][0]["value"];
  String key_state = jsonDoc["control_key"][0]["value"];
  bool k_state = key_state == "on";

  Serial.println(jsonDoc.as<String>());

  delay(10);
  digitalWrite(RELAY_PIN, state == "on");
  digitalWrite(LED_PIN, k_state);

  delay(10);
  Serial.println(state);

}


String getRandomData() {
  DynamicJsonDocument timeseries(2048);
  timeseries["temperature_1"] = random(TEMPRATURE_MIN_VALUE, TEMPRATURE_MAX_VALUE+1);
  timeseries["temperature_2"] = random(TEMPRATURE_MIN_VALUE, TEMPRATURE_MAX_VALUE+1);
  
  timeseries["humidity_1"] = random(HUMIDITY_MIN_VALUE, HUMIDITY_MAX_VALUE+1);
  timeseries["humidity_2"] = random(HUMIDITY_MIN_VALUE, HUMIDITY_MAX_VALUE+1);

  timeseries["soil_1"] = random(MOISTURE_MIN_VALUE, MOISTURE_MAX_VALUE+1);
  timeseries["soil_2"] = random(MOISTURE_MIN_VALUE, MOISTURE_MAX_VALUE+1);
  timeseries["soil_3"] = random(MOISTURE_MIN_VALUE, MOISTURE_MAX_VALUE+1);
  timeseries["soil_4"] = random(MOISTURE_MIN_VALUE, MOISTURE_MAX_VALUE+1);
  timeseries["soil_5"] = random(MOISTURE_MIN_VALUE, MOISTURE_MAX_VALUE+1);

  timeseries["brightness"] = random(LIGHT_MIN_VALUE, LIGHT_MAX_VALUE+1);

  timeseries["ec"] = random(EC_MIN_VALUE, EC_MAX_VALUE+1);

  timeseries["ph"] = random(PH_MIN_VALUE, PH_MAX_VALUE+1);
  timeseries["lightStatus"] = "off";

  String json_str;
  serializeJson(timeseries, json_str);

  return json_str;
}

void sendRandomDataToServer() {
  WiFiClient client;
  HTTPClient http;
  

  String json_str = getRandomData();

  Serial.println(json_str);

  http.begin(client, sendDataPath);
  http.POST(json_str);
  http.end();
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  /* Initialize SPI bus */
  Serial.println("test");

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, 0); 

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, 0); 
  /* Initialize MFRC522 Module */


  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
}

void loop() {
  // put your main code here, to run repeatedly:
  sendRandomDataToServer();
  sendToServer();
  delay(3000);

}
