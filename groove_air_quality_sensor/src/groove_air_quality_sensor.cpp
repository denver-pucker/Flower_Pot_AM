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

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(SEMI_AUTOMATIC);

// setup() runs once, when the device is first turned on
void setup() {
  // Used to print/debug for me
  Serial.begin(9600);
  waitFor(Serial.isConnected,10000);

  if(sensor.init()) {
    Serial.printf("Sensor Ready\n");
  }
  else {
    Serial.printf("Sensor Error\n");
  }

  // Connect to Internet but not Particle Cloud
  WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

}

void loop() {
  int quality = sensor.slope();
  Serial.printf("Sensor value: %i\n",sensor.getValue());
  
  if (quality == AirQualitySensor::FORCE_SIGNAL) {
    Serial.printf("High pollution! Force signal active.\n");
  }
  else if (quality == AirQualitySensor::HIGH_POLLUTION) {
    Serial.printf("High pollution!\n");
  }
  else if (quality == AirQualitySensor::LOW_POLLUTION) {
    Serial.printf("Low pollution!\n");
  }
  else if (quality == AirQualitySensor::FRESH_AIR) {
    Serial.printf("Fresh air.\n");
  }

  delay(1000);
}
