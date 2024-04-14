#include <Wire.h>
#include <BH1750.h>
#include <SimpleTimer.h>
#include <ld2410.h>
//#include "HLK-LD2450.h"

#define PIR_SENSOR_PIN 5
#define RADAR_POWER_PIN 8
#define SDA_PIN 10
#define SCL_PIN 9
#define LD2410_RX_PIN 20
#define LD2410_TX_PIN 21
#define LD2450_RX_PIN 6
#define LD2450_TX_PIN 7

#define LD2410_BAUDRATE 256000

#define PIR_SENSOR_TIMEOUT 60000 //60s

#define PIR_UPDATE_INTERVAL 500 //0.5s
#define LIGHT_METER_UPDATE_INTERVAL 15000 //15s
#define RADAR_UPDATE_INTERVAL 1000 //1s

BH1750 lightMeter;

//HardwareSerial Ld2450Serial(0);
//HLK_LD2450 ld2450(LD2450_RX_PIN, LD2450_TX_PIN, &Ld2450Serial);

HardwareSerial Ld2410Serial(1);
ld2410 radar;

SimpleTimer timer;

int radarUpdateTimerId;

bool isRadarEnabled = false;
bool radarStationaryTargetDetected = false;
bool radarMovingTargetDetected = false;
int radarStationaryTargetDistance;
int radarStationaryTargetEnergy;
int radarMovingTargetDistance;
int radarMovingTargetEnergy;

void updatePir() {
  bool pirSensorStatus = digitalRead(PIR_SENSOR_PIN);
  if (pirSensorStatus && !isRadarEnabled) {
    enableRadar();
  }
}

void updateLightMeter() {
  float lux = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
}

void reportRadarData() {
  if (radarStationaryTargetDetected) {
    Serial.print("Stationary target: ");
    Serial.print(radarStationaryTargetDistance);
    Serial.print(" cm energy:");
    Serial.println(radarStationaryTargetEnergy);
  }
  if (radarMovingTargetDetected) {
    Serial.print("Moving target: ");
    Serial.print(radarMovingTargetDistance);
    Serial.print(" cm energy:");
    Serial.println(radarMovingTargetEnergy);
  }
}  

void updateRadar() {
  radar.read();
  //ld2450.read();
  if (radar.presenceDetected()) {
    if (radar.stationaryTargetDetected()) {
      radarStationaryTargetDetected = true;
      radarStationaryTargetDistance = radar.stationaryTargetDistance();
      radarStationaryTargetEnergy = radar.stationaryTargetEnergy();
    } else {
      radarStationaryTargetDetected = false;
    }
    if (radar.movingTargetDetected()) {
      radarMovingTargetDetected = true;
      radarMovingTargetDistance = radar.movingTargetDistance();
      radarMovingTargetEnergy = radar.movingTargetEnergy();
    } else {
      radarMovingTargetDetected = false;
    }
  }
}

void enableRadarUpdateTimer() {
  timer.enable(radarUpdateTimerId);
}

void disableRadarUpdateTimer() {
  timer.disable(radarUpdateTimerId);
}

void enableRadar() {
  enableRadarPower(true);
  setupLd2410();
  setupLd2450();
  enableRadarUpdateTimer();
  restartPirTimeoutTimer();
  isRadarEnabled = true;
}

void disableRadar() {
  isRadarEnabled = false;

  disableRadarUpdateTimer();
  enableRadarPower(false);
}

void restartPirTimeoutTimer() {
  timer.setTimeout(PIR_SENSOR_TIMEOUT, disableRadar);
}

void setupPirSensor() {
  pinMode(PIR_SENSOR_PIN, INPUT);
}

void enableRadarPower(bool isOn) {
  digitalWrite(RADAR_POWER_PIN, isOn ? HIGH : LOW);  
}

void setupRadarPower() {
  pinMode(RADAR_POWER_PIN, OUTPUT);
  enableRadarPower(false);
  enableRadarPower(true);
}

void setupLightMeter() {
  Wire.setPins(SDA_PIN, SCL_PIN);
  Wire.begin();
  lightMeter.begin();
}

void setupLd2410() {
  delay(2000);
  Ld2410Serial.begin(LD2410_BAUDRATE, SERIAL_8N1, LD2410_RX_PIN, LD2410_TX_PIN);
  delay(1000);
  radar.begin(Ld2410Serial);
}

void setupLd2450() {
  //if(!hlk.connect(256000, HLK_TX, HLK_RX)){ // Establishing a connection with the sensor
  //  Serial.print("ERROR: Invalid EspSoftwareSerial pin configuration");
  //  while (1) { // Don't continue with invalid configuration
   //   delay (1000);
  //  }
  //} 
}

void setupPirUpdateTimer() {
  timer.setInterval(PIR_UPDATE_INTERVAL, updatePir);
}

void setupLightMeterUpdateTimer() {
  timer.setInterval(LIGHT_METER_UPDATE_INTERVAL, updateLightMeter);
}

void setupRadarUpdateTimer() {
  radarUpdateTimerId = timer.setInterval(RADAR_UPDATE_INTERVAL, reportRadarData);
  timer.disable(radarUpdateTimerId);
}

void setup() {
  Serial.begin(9600);

  setupPirSensor();
  setupRadarPower();
  //setupLightMeter();
  setupPirUpdateTimer();
  //setupLightMeterUpdateTimer();
  setupRadarUpdateTimer();
}

void loop() {
  if (isRadarEnabled) {
    updateRadar();
  }
  timer.run();
  //Serial.print("Target X: ");
  //Serial.println(ld2450.getTargetX());
/*

  Serial.print("Target Y: ");
  Serial.println(ld2450.getTargetY());
  Serial.print("Speed: ");
  Serial.println(ld2450.getSpeed());
  Serial.print("tDisRes: ");
  Serial.println(ld2450.getDistanceResolution());
  
  Serial.println("-----------------------------------------------");
*/
}
