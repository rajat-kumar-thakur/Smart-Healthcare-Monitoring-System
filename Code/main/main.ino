#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

// Wi-Fi Credentials
const char* ssid = "Billo_Bagge";
const char* password = "rajat123";

// DS18B20
#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// DHT22
#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MAX30100
PulseOximeter pox;

// LEDs
#define LED_GREEN D5
#define LED_RED   D6

ESP8266WebServer server(80);

float roomTemp = 0.0, roomHumidity = 0.0, dsTemp = 0.0, heartRate = 0.0, spo2 = 0.0;
unsigned long lastSensorRead = 0;
unsigned long lastPageRequest = 0;

void readSensors() {
  roomHumidity = dht.readHumidity();
  roomTemp = dht.readTemperature();
  ds18b20.requestTemperatures();
  dsTemp = ds18b20.getTempCByIndex(0);

  for (int i = 0; i < 30; i++) {
    pox.update();
    delay(10);
  }
  heartRate = pox.getHeartRate();
  spo2 = pox.getSpO2();

  if (isnan(roomHumidity)) roomHumidity = 0.0;
  if (isnan(roomTemp)) roomTemp = 0.0;
  if (dsTemp == DEVICE_DISCONNECTED_C) dsTemp = 0.0;
  if (heartRate < 1) heartRate = 0.0;
  if (spo2 < 1) spo2 = 0.0;

  Serial.println("---- Sensor Readings ----");
  Serial.print("Room Temp: "); Serial.println(roomTemp);
  Serial.print("Room Humidity: "); Serial.println(roomHumidity);
  Serial.print("Body Temp: "); Serial.println(dsTemp);
  Serial.print("Heart Rate: "); Serial.println(heartRate);
  Serial.print("SpO2: "); Serial.println(spo2);
  Serial.println("-------------------------");
}

void handleRoot() {
  lastPageRequest = millis();

  String html = R"====(
<!DOCTYPE html>
<html>
<head>
  <title>Smart Healthcare Monitoring System</title>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <meta http-equiv="refresh" content="1">
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css">
  <style>
    body { font-family: sans-serif; background-color: #fff; color: #333; margin: 0; padding: 0; font-size: 12px; }
    #page { max-width: 600px; margin: 10px auto; padding: 10px; }
    .header h1 { color: #008080; font-family: Garamond, 'sans-serif'; }
    .sensor { margin: 8px 0; font-size: 1.5rem; }
    .sensor-labels { font-size: 0.9rem; }
    .units { font-size: 1rem; }
    hr { height: 1px; background-color: #eee; border: none; margin: 5px 0; }
  </style>
</head>
<body>
  <div id="page">
    <div class="header"><h1>Smart Healthcare Monitoring System</h1></div>
    <div class="box-full" align="left">
      <h2>Sensors Readings</h2>

      <p class="sensor"><i class="fas fa-thermometer-half" style="color:#0275d8"></i>
        <span class="sensor-labels"> Room Temperature </span>
        )====";
  html += String(roomTemp, 1);
  html += R"====( <sup class="units">°C</sup></p><hr>

      <p class="sensor"><i class="fas fa-tint" style="color:#5bc0de"></i>
        <span class="sensor-labels"> Room Humidity </span>
        )====";
  html += String(roomHumidity, 1);
  html += R"====( <sup class="units">%</sup></p><hr>

      <p class="sensor"><i class="fas fa-heartbeat" style="color:#cc3300"></i>
        <span class="sensor-labels"> Heart Rate </span>
        )====";
  html += String(heartRate, 0);
  html += R"====( <sup class="units">BPM</sup></p><hr>

      <p class="sensor"><i class="fas fa-burn" style="color:#f7347a"></i>
        <span class="sensor-labels"> SpO2 </span>
        )====";
  html += String(spo2, 0);
  html += R"====( <sup class="units">%</sup></p><hr>

      <p class="sensor"><i class="fas fa-thermometer-full" style="color:#d9534f"></i>
        <span class="sensor-labels"> Body Temperature </span>
        )====";
  html += String(dsTemp, 1);
  html += R"====( <sup class="units">°C</sup></p>
    </div>
    <div class="developed-by">
      <h3>Developed by</h3>
      <p><strong>Isha Jangir</strong> (202211031)</p>
      <p><strong>Rajat Kumar Thakur</strong> (202211070)</p>
    </div>
  </div>
</body>
</html>
)====";

  server.send(200, "text/html", html);
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, HIGH);

  dht.begin();
  ds18b20.begin();

  Wire.begin(D2, D1);
  if (!pox.begin()) {
    Serial.println("MAX30100 failed.");
  } else {
    Serial.println("MAX30100 ready.");
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= 5000) {
    lastSensorRead = now;
    readSensors();
  }

  // LED Control: webpage active in last 10 sec = green
  if (now - lastPageRequest <= 10000) {
    digitalWrite(LED_GREEN, HIGH);
    digitalWrite(LED_RED, LOW);
  } else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  }

  pox.update();
  server.handleClient();
}
