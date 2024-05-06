

#include <Arduino.h>
#include <WiFi.h>
#include <FirebaseESP32.h>

#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   
#include "Wire.h"
#include <MPU6050_light.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>

// Provide the RTDB payload printing info and other helper functions.
#include <addons/RTDBHelper.h>

/* 1. Define the WiFi credentials */
#define WIFI_SSID "OPPO F19 Pro"
#define WIFI_PASSWORD "1234567891011"

// For the following credentials, see examples/Authentications/SignInAsUser/EmailPassword/EmailPassword.ino

/* 2. Define the API Key */
#define API_KEY "AIzaSyD5taLrEjA-PG_D_GQbbYOBnIieDxN_kUk"

/* 3. Define the RTDB URL */
#define DATABASE_URL "https://poochpaw-9bf51-default-rtdb.firebaseio.com/" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "sachethana12@gmail.com"
#define USER_PASSWORD "test123$"

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

unsigned long count = 0;

//  Variables
const int PulseWire = 34;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0

int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.

PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"
MPU6050 mpu(Wire);

long timer = 0;

void setup()
{

  Serial.begin(9600);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(100);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h

  // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
  Firebase.reconnectNetwork(true);

  // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
  // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
  fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);


  Firebase.begin(&config, &auth);

  Firebase.setDoubleDigits(5);

  // Configure the PulseSensor object, by assigning our variables to it. 
  pulseSensor.analogInput(PulseWire);   
 // pulseSensor.blinkOnPulse(LED);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);   

  // Double-check the "pulseSensor" object was created and "began" seeing a signal. 
   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }

  Wire.begin();
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
  delay(10);
  mpu.calcOffsets(true,true); // gyro and accelero
  Serial.println("Done!\n");

}

void loop()
{

  if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened".
int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 
 Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
 Serial.print("BPM: ");                        // Print phrase "BPM: " 
 Serial.println(myBPM);                        // Print the value inside of myBPM. 

//MPU 6050  updates;
mpu.update();

  if(millis() - timer > 10){ // print data every second

    Serial.print(F("TEMPERATURE: "));Serial.println(mpu.getTemp());
    Serial.print(F("ACCELERO  X: "));Serial.print(mpu.getAccX());
    Serial.print("\tY: ");Serial.print(mpu.getAccY());
    Serial.print("\tZ: ");Serial.println(mpu.getAccZ());
  
    Serial.print(F("GYRO      X: "));Serial.print(mpu.getGyroX());
    Serial.print("\tY: ");Serial.print(mpu.getGyroY());
    Serial.print("\tZ: ");Serial.println(mpu.getGyroZ());
  
    Serial.print(F("ACC ANGLE X: "));Serial.print(mpu.getAccAngleX());
    Serial.print("\tY: ");Serial.println(mpu.getAccAngleY());
    
    Serial.print(F("ANGLE     X: "));Serial.print(mpu.getAngleX());
    Serial.print("\tY: ");Serial.print(mpu.getAngleY());
    Serial.print("\tZ: ");Serial.println(mpu.getAngleZ());
    Serial.println(F("=====================================================\n"));
    timer = millis();
  }

  // Firebase.ready() should be called repeatedly to handle authentication tasks.

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 1 || sendDataPrevMillis == 0))
  {
    sendDataPrevMillis = millis();

    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/Coller/BPM"), myBPM) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Set int... %s\n", Firebase.setInt(fbdo, F("/Coller/TEMPERATURE"), mpu.getTemp()) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/ACCELERO :X"), mpu.getAccX() + 0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/ACCELERO :Y"), mpu.getAccY() +0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/ACCELERO :Z"), mpu.getAccZ() + 0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());

    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/GYRO :X"), mpu.getGyroX() + 0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/GYRO :Y"), mpu.getGyroY() + 0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set float... %s\n", Firebase.setFloat(fbdo, F("/Coller/GYRO :Z"), mpu.getGyroZ() + 0.01 + random(0, 100)) ? "ok" : fbdo.errorReason().c_str());

    Serial.println();

    count++;
  }

 }
  delay(10);
}

