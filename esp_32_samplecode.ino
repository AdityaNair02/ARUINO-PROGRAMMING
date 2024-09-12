#include <HTTPClient.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "POCO"
#define WIFI_PASSWORD "123456789"

// Insert Firebase project API Key
#define API_KEY "REPLACE_WITH_YOUR_FIREBASE_PROJECT_API_KEY"

// Insert RTDB URL
#define DATABASE_URL "REPLACE_WITH_YOUR_FIREBASE_DATABASE_URL"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

// Define serial communication pins
#define RX 13
#define TX 12

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RX, TX); // Initialize serial communication with Arduino Uno

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  if (Firebase.ready() && signupOK) {
    if (Serial2.available() > 0) {
      String data = Serial2.readStringUntil('\n');
      Serial.println("Received data from Arduino Uno: " + data);

      // Parse the data
      int index = 0;
      String waterLevel = "";
      String motionStatus = "";
      String temperature = "";
      String soilMoisture = "";
      String fireStatus = "";
      String lpgStatus = "";
      String irSensorStatus = "";

      while (index < data.length()) {
        if (data.substring(index, index + 5) == "water") {
          index += 6;
          waterLevel = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 6) == "motion") {
          index += 7;
          motionStatus = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 10) == "temperature") {
          index += 11;
          temperature = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 4) == "soil") {
          index += 5;
          soilMoisture = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 4) == "fire") {
          index += 5;
          fireStatus = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 3) == "lpg") {
          index += 4;
          lpgStatus = data.substring(index, data.indexOf('&', index));
          index = data.indexOf('&', index) + 1;
        } else if (data.substring(index, index + 2) == "ir") {
          index += 3;
          irSensorStatus = data.substring(index);
        }
      }

      // Upload data to Firebase
      if (Firebase.RTDB.setInt(&fbdo, "water_level", waterLevel.toInt())) {
        Serial.println("Water level uploaded successfully");
      } else {
        Serial.println("Failed to upload water level");
      }

      if (Firebase.RTDB.setString(&fbdo, "motion_status", motionStatus)) {
        Serial.println("Motion status uploaded successfully");
      } else {
        Serial.println("Failed to upload motion status");
      }

      if (Firebase.RTDB.setFloat(&fbdo, "temperature", temperature.toFloat())) {
        Serial.println("Temperature uploaded successfully");
      } else {
        Serial.println("Failed to upload temperature");
      }

      if (Firebase.RTDB.setInt(&fbdo, "soil_moisture", soilMoisture.toInt())) {
        Serial.println("Soil moisture uploaded successfully");
      } else {
        Serial.println("Failed to upload soil moisture");
      }

      if (Firebase.RTDB.setString(&fbdo, "fire_status", fireStatus)) {
        Serial.println("Fire status uploaded successfully");
      } else {
        Serial.println("Failed to upload fire status");
      }

      if (Firebase.RTDB.setString(&fbdo, "lpg_status", lpgStatus)) {
        Serial.println("LPG status uploaded successfully");
      } else {
        Serial.println("Failed to upload LPG status");
      }

      if (Firebase.RTDB.setString(&fbdo, "ir_sensor_status", irSensorStatus)) {
        Serial.println("IR sensor status uploaded successfully");
      } else {
        Serial.println("Failed to upload IR sensor status");
      }
    }
  }
  delay(100);
}
