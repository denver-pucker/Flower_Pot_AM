/* 
 * Project groove_air_quality_sensor
 * Author: Adrian Montoya
 * Date: 18 MARCH 2025
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Air_Quality_Sensor.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "credentials.h"

AirQualitySensor sensor(A0);

int value;          // stores returned value
int airQuality;     // stores values from grove air quality sensor
int slopeQuality;   // stores defied slope of grove aqs sensor
int dust;           // dust value returned


const int FORCE_SIGNAL=0;
const int HIGH_POLLUTION=1;
const int LOW_POLLUTION=2;
const int FRESH_AIR=3;

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// setup() runs once, when the device is first turned on
void setup() {
  // Used to print/debug for me
  Serial.begin(9600);
  waitFor(Serial.isConnected,10000);

  // Connect to Internet but not Particle Cloud
  WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

}

// loop() runs over and over again, as quickly as it can execute.
void loop() {
  // The core of your code will likely live here.

  // Example: Publish event to cloud every 10 seconds. Uncomment the next 3 lines to try it!
  // Log.info("Sending Hello World to the cloud!");
  // Particle.publish("Hello world!");
  // delay( 10 * 1000 ); // milliseconds and blocking - see docs for more info!
}
