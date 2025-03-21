/* 
 * Project my_seed_dust_sensor_example
 * Author: Adrian Montoya
 * Date: 18 MARCH 2025
 */

// Include Particle Device OS APIs
#include "Particle.h"
#include "Adafruit_BME280.h"
#include "Air_Quality_Sensor.h"
#include <Adafruit_MQTT.h>
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h"
#include "Adafruit_MQTT/Adafruit_MQTT.h"
#include "Adafruit_GFX.h"
#include "Adafruit_SSD1306.h"
#include "credentials.h"

const int OLED_RESET=-1;
const int LOGO16_GLCD_HEIGHT=64;
const int LOGO16_GLCD_WIDTH=128;
const int XPOS = 0;
const int YPOS = 1;
const int TEMPFREQ = 10000;
const int MOISTFREQ = 30000;
const int BME=0x76;

AirQualitySensor sensor(A0);
// const int DUSTPIN = A1;
const int WATERPIN = A2;
const int MOISTUREPIN = A5;
const int PUMPMOTOR = D19;


int value;          // stores returned value
int airQuality;     // stores values from grove air quality sensor
int slopeQuality;   // stores defied slope of grove aqs sensor
// int dust;           // dust value returned
int waterLevelInd;  // water level low detection
int moistRead;      // moisture reading
int butValue;       // get button info

// BME 
float tempC, tempF;
float pressPA;
float humidRH;
bool status;


// Dust sensor var
// float lowPulseOccupancy;
// float ratio;
// float concentration = 0;
// float realConcentration;
// int LPOFAC = 10.0;

unsigned long lastTime;
// unsigned long dustStartTime;
unsigned long waterStartTime;
unsigned long duration;

String dateTime , timeOnly;

TCPClient TheClient; 
 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); 

Adafruit_MQTT_Subscribe buttonFeed = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttononoff"); 
Adafruit_MQTT_Publish aqPub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/airquality");
// Adafruit_MQTT_Publish dustPub = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dustquality");
Adafruit_MQTT_Publish waterLevel = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/waterlevel");
Adafruit_MQTT_Publish moistureLevel = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisturelevel");



// Display setup
Adafruit_SSD1306 display(OLED_RESET);

// BME
Adafruit_BME280 bme;

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

/************Declare Functions*************/
void MQTT_connect();
bool MQTT_ping();
float getTemp(int timeInterval);
int getMoisture(int probePin, int timeInterval);

// Let Device OS manage the connection to the Particle Cloud
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

// setup() runs once, when the device is first turned on
void setup() {
  // Setup time to DST
  Time.zone(-6);
  Particle.syncTime();

  // Used to print/debug for me
  Serial.begin(9600);
  waitFor(Serial.isConnected,10000);

  // text display tests
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  display.clearDisplay();
  display.display();
  delay(200);

  if(sensor.init()) {
    Serial.printf("Sensor Ready\n");
  }
  else {
    Serial.printf("Sensor Error\n");
  }

  status = bme.begin(BME);
  if (status == false) {
    Serial.printf("BME280 at address 0x%02x failed to start\n", BME);
  }

  // Connect to Internet but not Particle Cloud
  WiFi.on();
  WiFi.connect();
  while(WiFi.connecting()) {
    Serial.printf(".");
  }
  Serial.printf("\n\n");

  // setup pin modes
  // pinMode(DUSTPIN,INPUT);
  pinMode(WATERPIN,INPUT);
  pinMode(MOISTUREPIN,INPUT);
  pinMode(PUMPMOTOR,OUTPUT);

  // timing
  // dustStartTime = millis();
  waterStartTime = millis();

  // Setup MQTT button for pump
  mqtt.subscribe(&buttonFeed);

}

void loop() {
  // get button on/off from adafruit.io 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(100))) {
    if (subscription == &buttonFeed) {
      butValue = atoi((char *)buttonFeed.lastread);
      Serial.printf("buttonFeed = %i\n",butValue); 
      if (butValue) {
        Serial.printf("Turning on pump\n"); 
        digitalWrite(PUMPMOTOR,HIGH);
        delay(1000);
        digitalWrite(PUMPMOTOR,LOW);
      }
      if (!butValue) {
        Serial.printf("Turning off pump\n"); 
        digitalWrite(PUMPMOTOR,LOW);
      }
    }
  }

  // soil moisture reading
  dateTime = Time.timeStr(); //Current Date and Time from Particle Time class 
  timeOnly = dateTime.substring(11,19); //Extract the Time from the DateTime String 

  if(millis()-lastTime >5000) {
    lastTime = millis();

    // Read Moisture
    moistRead=analogRead(MOISTUREPIN);
    if ( moistRead > 3000){
      digitalWrite(PUMPMOTOR,HIGH);
      Serial.printf("Turning on Pump Motor for 1 second\n");
      delay(1000);
      digitalWrite(PUMPMOTOR,LOW);
      Serial.printf("Turning off Pump Motor");
    }
    Serial.printf("Moisture = %d \n%s\n",moistRead,timeOnly.c_str());

    // Display to OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.printf("MOISTURE\n%d\nTIME:%s",moistRead,timeOnly.c_str());
    display.display();
    delay(3000);

  // Water level sensor
    waterLevelInd = analogRead(WATERPIN);
    Serial.printf("Water Level: %i\n",waterLevelInd);

    // Display to OLED
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.printf("Water\nLevel\n%i\nTIME:%s",waterLevelInd,timeOnly.c_str());
    display.display();
    delay(3000);
  }

  // Dust sensor
  // lowPulseOccupancy = lowPulseOccupancy + duration;
  // if((millis() - dustStartTime) >= 30000) {
  //   ratio = lowPulseOccupancy / (30000 * LPOFAC);
  //   concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
  //   if(concentration > 1){
  //     realConcentration = concentration;
  //   }
  //   Serial.printf("\nConcentration = %0.5f pcs/0.01cf \n",realConcentration);
  //   lowPulseOccupancy = 0;
  //   dustStartTime = millis();
  // }

  // Air quality sensor information
  int slopeQuality = sensor.slope();
  int airQuality = sensor.getValue();
  Serial.printf("Air Quality: %i\n",airQuality);
  
  if (slopeQuality == AirQualitySensor::FORCE_SIGNAL) {
    Serial.printf("High pollution! Force signal active.\n");
        // Display to OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.printf("RUN!!\nAir is\nBAD!");
        display.display();
        delay(3000);
  }
  else if (slopeQuality == AirQualitySensor::HIGH_POLLUTION) {
    Serial.printf("High pollution!\n");
        // Display to OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.printf("Hight\nAir\nPolution");
        display.display();
        delay(3000);
  }
  else if (slopeQuality == AirQualitySensor::LOW_POLLUTION) {
    Serial.printf("Low pollution!\n");
        // Display to OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.printf("Low\nAir\nPolution");
        display.display();
        delay(3000);
  }
  else if (slopeQuality == AirQualitySensor::FRESH_AIR) {
    Serial.printf("Fresh air.\n");
        // Display to OLED
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.printf("Air\nIs\nGreat!");
        display.display();
        delay(3000);
  }
  delay(3000);

  if((millis()-lastTime > 10000)) {
    if(mqtt.Update()) {
      aqPub.publish(airQuality);
      moistureLevel.publish(moistRead);
      waterLevel.publish(waterLevelInd);
      // Serial.printf("Air Quality: %i Dust: %f Water Level: %i\n",airQuality, realConcentration,waterLevelInd
      Serial.printf("Air Quality: %i Water Level: %i\n",airQuality,waterLevelInd);

    } 
    lastTime = millis();
    // Display to OLED Air Quality
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.printf("Air Qlty\n%i\nTIME:%s\n",airQuality,timeOnly.c_str());
    display.display();
    delay(3000);

    // Display to OLED Water Level
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.printf("Water Lvl\n%i\n@\nTIME:%s\n",waterLevelInd,timeOnly.c_str());
    display.display();
    delay(3000);
  }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;
 
  // Return if already connected.
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf("Error Code %s\n",mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...\n");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds and try again
  }
  Serial.printf("MQTT Connected!\n");
}

bool MQTT_ping() {
  static unsigned int last;
  bool pingStatus;

  if ((millis()-last)>120000) {
      Serial.printf("Pinging MQTT \n");
      pingStatus = mqtt.ping();
      if(!pingStatus) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
  return pingStatus;
}

float getTemp(int timeInterval) {
  int currentTime;
  static int lastTime = -999999;
  static float data;

  currentTime = millis();
  if(currentTime - lastTime > timeInterval) {
    lastTime = millis();
    data = bme.readTemperature();
  }
  return data;
}

int getMoisture(int probePin, int timeInterval) {
  static int lastTime = -999999;
  static int data;

  if(millis() - lastTime > timeInterval) {
    lastTime = millis();
    data = analogRead(probePin);
  }
  return data;
}