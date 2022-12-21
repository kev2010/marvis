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

// Define stepper motor connections and steps per revolution:
#define dirPin 18       // direction pin
#define stepPin 19      // step Pin
#define Motor_Enable 21 // Enable Pin
#define stepsPerRevolution 200
int vel = 0;
bool motorState = LOW;
bool dir = LOW;
unsigned long getDataPrevMillis = 0;
unsigned long currentTime = 0;
bool signupOK = false;
const int port = 80;

WiFiClient client;
int API_REFRESH_RATE = 1000;

void APICheck(void *arg)
{
  for (;;)
  {
    // Update brightness and flashing speed API
    if (Firebase.ready() && signupOK && (millis() - getDataPrevMillis > API_REFRESH_RATE || getDataPrevMillis == 0))
    {
      getDataPrevMillis = millis();
      Serial.println("Checking for update");

      if (Firebase.RTDB.getInt(&fbdo, "marvis/motor"))
      {
        Serial.println("checking..");
        if (fbdo.dataType() == "int")
        {
          int tempSpeed = fbdo.intData();
          printf("\ntempSpeed, vel: %02d %02d", tempSpeed, vel);
          if (tempSpeed != vel)
          {
            printf("\nvel: %02d", vel); // old
            dir = (tempSpeed < 0) ? LOW : HIGH;
            printf("\ndir: %02d", dir);
            vel = abs(tempSpeed);
          }
        }
      }
      else
      {
        Serial.println(fbdo.errorReason());
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);
  // Declare pins as output:
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  pinMode(Motor_Enable, OUTPUT);
  digitalWrite(dirPin, dir);
  digitalWrite(Motor_Enable, LOW);

  // Connect to Wifi
  printf("\nConnecting to WiFi ");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  printf("\nWiFi connected with address %s\n", WiFi.localIP().toString().c_str());

  // Firebase database config
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up as anonymous user
  if (Firebase.signUp(&config, &auth, "", ""))
  {
    printf("\nOk!");
    signupOK = true;
  }
  else
  {
    printf("\n%s", config.signer.signupError.message.c_str());
  }

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize parallel task
  xTaskCreate(
      APICheck,   // name of the task function
      "APICheck", // name of the task
      10000,      // memory assigned for the task (stack size?)
      NULL,       // parameter to pass if any
      tskIDLE_PRIORITY,
      NULL);
}

void loop()
{
  // Set the spinning direction clockwise:
  if (vel != 0)
  {
    if (millis() - currentTime >= (11 - vel))
    {
      //      printf("\nRUNNING %02d %02d",vel, millis() - currentTime);
      motorState = !motorState;
      //      printf("\nmotorState: %02d", motorState);
      digitalWrite(dirPin, dir);
      digitalWrite(stepPin, motorState);
      currentTime = millis();
    }
  }
}
