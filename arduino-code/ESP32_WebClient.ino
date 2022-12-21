#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

// WiFi Details
#define WIFI_SSID "MIT"
#define WIFI_PASSWORD ""

// Firebase Setup - RealTimeDataBase
#define API_KEY "AIzaSyAtY1d7kKLU69TpfWXP29pO5eX7cL5lguw"
#define DATABASE_URL "https://marvis-2090a-default-rtdb.firebaseio.com/"
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Defining Variables
int API_REFRESH_RATE = 1000;
int LED_RATE = 20;
int MOSFET_PIN = 21;
unsigned long getDataPrevMillis = 0;
unsigned long ledPrevMillis = 0;
int maxBrightnessValue = 0;
int currentBrightness = 0;
int flashingSpeedValue = 0;
const int PWMFreq = 5000;
const int PWMChannel = 0;
const int PWMResolution = 10;
bool signupOK = false;
const int port = 80;
WiFiClient client;

void APICheck(void* arg) {
  for (;;) {
    // Update brightness and flashing speed API
    if (Firebase.ready() && signupOK && (millis() - getDataPrevMillis > API_REFRESH_RATE || getDataPrevMillis == 0)) {
      getDataPrevMillis = millis();
  
      // Check for changes in flashing speed (ABSOLUTE VALUE) - reset if there is
      if (Firebase.RTDB.getFloat(&fbdo, "marvis/flashing_speed")) {
        if (fbdo.dataType() == "int") {
          int tempSpeed = fbdo.intData();
          if (tempSpeed != flashingSpeedValue && tempSpeed != -flashingSpeedValue) {
            currentBrightness = maxBrightnessValue;
            flashingSpeedValue = tempSpeed;
          }
        }
      }
      else {
        Serial.println(fbdo.errorReason());
      }
  
      // Check for changes in brightness - reset if there is
      if (Firebase.RTDB.getInt(&fbdo, "marvis/brightness")) {
        if (fbdo.dataType() == "int") {
          int tempBrightness = fbdo.intData();
          if (tempBrightness != maxBrightnessValue) {
            maxBrightnessValue = tempBrightness;
            currentBrightness = maxBrightnessValue;
            // Need to reset flashing speed too, otherwise might get stuck in loop if flashingSpeedValue is negative and brightness is at max
            flashingSpeedValue = abs(flashingSpeedValue);
          }                        
        }
      }
      else {
        Serial.println(fbdo.errorReason());
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Set up N-channel MOSFET
  ledcAttachPin(MOSFET_PIN, PWMChannel);
  ledcSetup(PWMChannel, PWMFreq, PWMResolution);
  
  // Connect to Wifi
  printf("\nConnecting to WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  printf("\nWiFi connected with address %s\n",WiFi.localIP().toString().c_str());

  // Firebase database config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up as anonymous user 
  if (Firebase.signUp(&config, &auth, "", "")){
    printf("\nOk!");
    signupOK = true;
  }
  else{
    printf("\n%s", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize parallel task
  xTaskCreate (
    APICheck,       // name of the task function
    "APICheck",     // name of the task
    10000,          // memory assigned for the task (stack size?)
    NULL,           // parameter to pass if any
    tskIDLE_PRIORITY,              
    NULL
  );             
}

void loop() {
  // Fading every "LED_RATE" ms
  if (millis() - ledPrevMillis > LED_RATE || ledPrevMillis == 0) {
    ledPrevMillis = millis();
    
    // set brightness value
    printf("\nbrightness: %02d, %02d", currentBrightness, flashingSpeedValue);
    ledcWrite(PWMChannel, currentBrightness);
  
    // reverse the direction of the fading at the ends of the fade  
    if (currentBrightness <= 0 || currentBrightness >= maxBrightnessValue) {
      flashingSpeedValue = -flashingSpeedValue;
    }
    
    // change the brightness for next time through the loop:
    currentBrightness = currentBrightness + flashingSpeedValue;
  }
}
