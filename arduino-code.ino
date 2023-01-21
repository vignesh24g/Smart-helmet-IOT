#include <WiFi.h>
#include "DHTesp.h"
#include "ThingSpeak.h"
#include <Adafruit_Sensor.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_MPU6050.h>
#include <Wire.h>
#include "HX711.h"

Adafruit_MPU6050 mpu;
LiquidCrystal_I2C LCD = LiquidCrystal_I2C(0x27, 16, 2);
HX711 scale;

const int DHT_PIN = 15;
const int ldr_pin=14;
const int buzz =13;
const int gas=35;
const int torch=25;
const char* ssid = "Wokwi-GUEST";
const char* pass = "";

WiFiClient client;

unsigned long myChannelNumber = 2;
const char* myWriteAPIKey = "66B5PY1GZ28CY23J";
const char* server = "api.thingspeak.com";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

int temperatureC;
int humidity;
DHTesp dhtSensor;

void setup() {
  Serial.begin(115200);
  LCD.init();
  LCD.backlight();
  LCD.setCursor(0, 0);
  LCD.print("IOT PROJECT");
  LCD.setCursor(0, 1);
  LCD.print("WiFi ");

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);
  dhtSensor.getPin();
  delay(10);
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.println(".");
  }
  Serial.println("WiFi Connected!");
  Serial.println(WiFi.localIP());
  
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  if (!mpu.begin()) {
		Serial.println("Failed to find MPU6050 chip");
		while (1) {
		  delay(10);
		}
	}
	Serial.println("MPU6050 Found!");
  
	mpu.setAccelerometerRange(MPU6050_RANGE_8_G);	// set accelerometer range to +-8G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);// set gyro range to +- 500 deg/s
	mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);// set filter bandwidth to 21 Hz

  scale.begin(32,33);
  pinMode(ldr_pin, INPUT);
  pinMode(buzz, OUTPUT);
  pinMode(gas, INPUT_PULLUP);
  //pinMode(tilt,INPUT);
  pinMode(torch, OUTPUT);
  analogWrite(torch,0);
}

void loop() {
  sensors_event_t a, g, temp;
	mpu.getEvent(&a, &g, &temp);
  LCD.clear();
  LCD.setCursor(0, 0);
  /*The digital output ("DO") pin goes high when it's dark, and low when there's light.On the physical sensor, you tweak the small 
  on-board potentiometer to set the threshold. In the simulator, use the "threshold" attribute to set the threshold voltage.
   The default threshold is 2.5 volts, or about 100 lux.*/
    analogWrite(torch, 0);
    if(scale.get_units()/420>8){
     LCD.clear();
     LCD.setCursor(0,0);
     LCD.println("Crash detected");
      Serial.println("crash warning");
      tone(buzz,1000);
  }else{
    if (digitalRead(gas) == HIGH) {
    LCD.println("Gas detected");
    Serial.println("gas detected");
    tone(buzz,1000);
    }else{
      noTone(buzz);
      if (digitalRead(ldr_pin) == HIGH) {
        LCD.println("Torch on");
        analogWrite(torch, 255);
      } else {
        analogWrite(torch, 0);
      }
      Serial.print("Rotation X: ");
	    Serial.print(g.gyro.x);
	    Serial.print(", Y: ");
	    Serial.print(g.gyro.y);
	    Serial.print(", Z: ");
	    Serial.println(g.gyro.z);
    if(g.gyro.x>0 || g.gyro.y>0 || g.gyro.z>0){
      LCD.println("Fallen down");
      Serial.println("person falled down");
      tone(buzz,1000);
    }else{
      
  delay(200);
  float weigh=scale.get_units();

  float kg=weigh/420;
  Serial.print("Weight on head:");
  Serial.print(kg);
  Serial.println("kg");
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("LOAD ON HEAD:");
  LCD.setCursor(0,1);
  LCD.print(kg);
  LCD.print("Kgs");
  delay(200);

  

  temperatureC = dhtSensor.getTemperature();
  Serial.print("Temperature (°C): ");
  Serial.println(temperatureC);
  humidity = dhtSensor.getHumidity();
  Serial.print("Humidity (%); ");
  Serial.println(humidity);
  LCD.clear();
  LCD.setCursor(0,0);
  LCD.print("TEMP:");
  LCD.print(temperatureC);
  LCD.setCursor(0,1);
  LCD.print("Humidity:");
  LCD.print(humidity);

  if(dhtSensor.getTemperature()>45 ){
          LCD.clear();
          LCD.setCursor(0,0);
          LCD.print("evacuate");
          tone(buzz,500);
          ThingSpeak.setField(1, temperatureC);
          ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    }
  
  ThingSpeak.setField(1, temperatureC);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3,kg);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  if (x == 200) {
    Serial.println("Channel update successful.");
  }
    }
  } 
    }
}
