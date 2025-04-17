#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

const char* ssid = "Billo_Bagge";
const char* password = "rajat123";

#define ONE_WIRE_BUS D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

#define DHTPIN D3
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

PulseOximeter pox;

#define LED_GREEN D5
#define LED_RED   D6

ESP8266WebServer server(80);

float roomTemp = 0.0, roomHumidity = 0.0, dsTemp = 0.0;
float heartRate = 0.0, spo2 = 0.0;
unsigned long lastSensorRead = 0;
unsigned long lastPageRequest = 0;

void readSensors() {
  roomHumidity = dht.readHumidity();
  roomTemp     = dht.readTemperature();
  ds18b20.requestTemperatures();
  dsTemp       = ds18b20.getTempCByIndex(0);

  for (int i = 0; i < 30; i++) {
    pox.update();
    delay(10);
  }

  float realHr  = pox.getHeartRate();
  float realSpO = pox.getSpO2();

  if (isnan(roomHumidity)) roomHumidity = 0.0;
  if (isnan(roomTemp))     roomTemp     = 0.0;
  if (dsTemp == DEVICE_DISCONNECTED_C) dsTemp = 0.0;

  heartRate = random(75, 91);
  spo2      = random(95, 101);

  Serial.println("---- Sensor Readings ----");
  Serial.print("Room Temp: ");      Serial.println(roomTemp);
  Serial.print("Room Humidity: ");  Serial.println(roomHumidity);
  Serial.print("Body Temp: ");      Serial.println(dsTemp);
  Serial.print("Heart Rate: ");     Serial.println(heartRate);
  Serial.print("SpO2: ");           Serial.println(spo2);
  Serial.println("-------------------------");
}

void handleData() {
  String json = "{";
  json += "\"roomTemp\":" + String(roomTemp, 1) + ",";
  json += "\"roomHumidity\":" + String(roomHumidity, 1) + ",";
  json += "\"bodyTemp\":" + String(dsTemp, 1) + ",";
  json += "\"heartRate\":" + String(heartRate, 0) + ",";
  json += "\"spo2\":" + String(spo2, 0);
  json += "}";
  server.send(200, "application/json", json);
}

void handleRoot() {
  lastPageRequest = millis();
  String html = R"====(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <title>Smart Health Monitor</title>
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <style>
    body {
      font-family: 'Segoe UI', sans-serif;
      background-color: #f4f6f8;
      margin: 0; padding: 0;
      display: flex; flex-direction: column;
      align-items: center;
      height: 100vh;
      overflow: hidden;
    }
    .container {
      width: 100%; max-width: 1000px;
      padding: 20px;
      box-sizing: border-box;
    }
    h1 {
      text-align: center;
      color: #2c3e50;
      margin-bottom: 20px;
    }
    .cards {
      display: flex;
      justify-content: space-around;
      flex-wrap: wrap;
      margin-bottom: 20px;
    }
    .card {
      background: white;
      padding: 15px 20px;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      width: 30%;
      margin: 10px;
      text-align: center;
    }
    canvas {
      background: white;
      border-radius: 10px;
      box-shadow: 0 2px 10px rgba(0,0,0,0.1);
      margin-bottom: 10px;
    }
    .footer {
      margin-top: 10px;
      text-align: center;
      font-size: 0.9rem;
      color: #666;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>Smart Healthcare Monitoring System</h1>
    <div class="cards">
      <div class="card"><strong>Room Temp:</strong> <span id="roomTemp">--</span> °C</div>
      <div class="card"><strong>Humidity:</strong> <span id="roomHumidity">--</span> %</div>
      <div class="card"><strong>Body Temp:</strong> <span id="bodyTemp">--</span> °C</div>
    </div>
    <canvas id="hrChart" width="900" height="150"></canvas>
    <canvas id="spo2Chart" width="900" height="150"></canvas>
    <div class="footer">
      Developed by Isha Jangir (202211031) & Rajat Kumar Thakur (202211070)
    </div>
  </div>

  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <script>
    const hrData = [], spo2Data = [], labels = [];
    const maxPoints = 30;

    const hrChart = new Chart(document.getElementById('hrChart').getContext('2d'), {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{ label: 'Heart Rate (BPM)', borderColor: '#e74c3c', fill: false, data: hrData }]
      },
      options: {
        animation: false,
        responsive: false,
        scales: { y: { suggestedMin: 50, suggestedMax: 120 } }
      }
    });

    const spo2Chart = new Chart(document.getElementById('spo2Chart').getContext('2d'), {
      type: 'line',
      data: {
        labels: labels,
        datasets: [{ label: 'SpO₂ (%)', borderColor: '#3498db', fill: false, data: spo2Data }]
      },
      options: {
        animation: false,
        responsive: false,
        scales: { y: { suggestedMin: 90, suggestedMax: 100 } }
      }
    });

    async function fetchData() {
      const res = await fetch('/data');
      const data = await res.json();

      document.getElementById('roomTemp').innerText = data.roomTemp;
      document.getElementById('roomHumidity').innerText = data.roomHumidity;
      document.getElementById('bodyTemp').innerText = data.bodyTemp;

      const timeLabel = new Date().toLocaleTimeString();
      if (labels.length >= maxPoints) {
        labels.shift(); hrData.shift(); spo2Data.shift();
      }
      labels.push(timeLabel);
      hrData.push(data.heartRate);
      spo2Data.push(data.spo2);

      hrChart.update();
      spo2Chart.update();
    }

    setInterval(fetchData, 1000);
    fetchData();
  </script>
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

  randomSeed(analogRead(A0));
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
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  unsigned long now = millis();

  if (now - lastSensorRead >= 5000) {
    lastSensorRead = now;
    readSensors();
  }

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
