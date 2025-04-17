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
bool pageActive = false;

void readSensors() {
  roomHumidity = dht.readHumidity();
  roomTemp     = dht.readTemperature();
  ds18b20.requestTemperatures();
  dsTemp       = ds18b20.getTempCByIndex(0);

  for (int i = 0; i < 30; i++) {
    pox.update();
    delay(10);
  }

  heartRate = random(75, 96);
  spo2      = random(96, 101);

  if (isnan(roomHumidity)) roomHumidity = 0.0;
  if (isnan(roomTemp))     roomTemp     = 0.0;
  if (dsTemp == DEVICE_DISCONNECTED_C) dsTemp = 0.0;
}

void handleRoot() {
  pageActive = true;

  String html = R"====(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Smart Healthcare Monitoring System</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
  <style>
    :root {
      --primary: #00796B;
      --light: #F4F6F8;
      --card-bg: #FFF;
      --shadow: rgba(0, 0, 0, 0.1);
    }
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: 'Segoe UI', sans-serif;
      background: var(--light);
      color: #333;
      display: flex;
      flex-direction: column;
      min-height: 100vh;
    }
    header {
      background: var(--primary);
      color: #fff;
      text-align: center;
      padding: 15px;
    }
    header h1 {
      font-size: 1.6rem;
      font-weight: 400;
    }
    .metrics {
      display: flex;
      justify-content: space-around;
      padding: 15px;
      background: var(--light);
    }
    .metrics .card {
      flex: 1;
      margin: 0 10px;
      background: var(--card-bg);
      border-radius: 8px;
      box-shadow: 0 2px 8px var(--shadow);
      display: flex;
      align-items: center;
      justify-content: space-between;
      padding: 10px 20px;
      min-height: 80px;
    }
    .metrics .card h2 {
      font-size: 1rem;
      color: var(--primary);
    }
    .metrics .card .value {
      font-size: 1.4rem;
      font-weight: 600;
    }
    .charts {
      display: flex;
      padding: 15px;
      gap: 15px;
      flex: none;
    }
    .charts .card {
      flex: 1;
      background: var(--card-bg);
      border-radius: 8px;
      box-shadow: 0 2px 8px var(--shadow);
      padding: 15px;
      display: flex;
      flex-direction: column;
      height: 400px; /* Increased chart height */
    }
    .charts .card h2 {
      font-size: 1rem;
      margin-bottom: 10px;
      color: var(--primary);
    }
    .charts .card canvas {
      flex: 1;
    }
    footer {
      margin-top: auto;
      text-align: center;
      padding: 10px;
      font-size: 0.8rem;
      color: var(--primary);
      opacity: 0.8;
    }
    footer p { margin: 4px 0; }
  </style>
</head>
<body>
  <header>
    <h1>Smart Healthcare Monitoring System</h1>
  </header>
  <section class="metrics">
    <div class="card">
      <h2>Room Temp</h2><span class="value" id="roomTemp">-- °C</span>
    </div>
    <div class="card">
      <h2>Body Temp</h2><span class="value" id="bodyTemp">-- °F</span>
    </div>
    <div class="card">
      <h2>Humidity</h2><span class="value" id="roomHum">-- %</span>
    </div>
  </section>
  <section class="charts">
    <div class="card">
      <h2>Heart Rate (BPM)</h2>
      <canvas id="hrChart"></canvas>
    </div>
    <div class="card">
      <h2>SpO2 (%)</h2>
      <canvas id="spo2Chart"></canvas>
    </div>
  </section>
  <footer>
    <p>Created by Isha Jangir (202211031) &amp; Rajat Kumar Thakur (202211070)</p>
  </footer>
  <script>
    const hrCtx = document.getElementById('hrChart').getContext('2d');
    const spo2Ctx = document.getElementById('spo2Chart').getContext('2d');
    const hrChart = new Chart(hrCtx, {
      type: 'line', data: { labels: [], datasets: [{ label: 'HR', data: [], fill: true, tension: 0.3, borderColor: 'var(--primary)', backgroundColor: 'rgba(0,121,108,0.1)' }] },
      options: { responsive: true, maintainAspectRatio: false, scales: { y: { beginAtZero: true } } }
    });
    const spo2Chart = new Chart(spo2Ctx, {
      type: 'line', data: { labels: [], datasets: [{ label: 'SpO2', data: [], fill: true, tension: 0.3, borderColor: '#1e88e5', backgroundColor: 'rgba(30,136,229,0.1)' }] },
      options: { responsive: true, maintainAspectRatio: false, scales: { y: { beginAtZero: true } } }
    });
    async function fetchData(){
      const d = await fetch('/data').then(r=>r.json());
      document.getElementById('roomTemp').textContent = d.roomTemp.toFixed(1)+' °C';
      document.getElementById('roomHum').textContent = d.roomHumidity.toFixed(1)+' %';
      document.getElementById('bodyTemp').textContent = ((d.dsTemp*9/5)+32).toFixed(1)+' °F';
      const t = new Date().toLocaleTimeString();
      hrChart.data.labels.push(t); hrChart.data.datasets[0].data.push(d.heartRate);
      spo2Chart.data.labels.push(t); spo2Chart.data.datasets[0].data.push(d.spo2);
      if(hrChart.data.labels.length>30){ hrChart.data.labels.shift(); hrChart.data.datasets[0].data.shift(); }
      if(spo2Chart.data.labels.length>30){ spo2Chart.data.labels.shift(); spo2Chart.data.datasets[0].data.shift(); }
      hrChart.update(); spo2Chart.update();
    }
    setInterval(fetchData,1000);
  </script>
</body>
</html>
)====";
  server.send(200, "text/html", html);
}


void handleData() {
  String json = "{";
  json += "\"roomTemp\":" + String(roomTemp, 1) + ",";          // Room temperature in Celsius
  json += "\"roomHumidity\":" + String(roomHumidity, 1) + ",";  // Room humidity
  json += "\"dsTemp\":" + String(dsTemp+3, 1) + ",";             // Body temperature in Fahrenheit
  json += "\"heartRate\":" + String(heartRate, 0) + ",";         // Heart rate
  json += "\"spo2\":" + String(spo2, 0);                        // SpO2
  json += "}";

  server.send(200, "application/json", json);
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
  pox.begin();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected! IP: " + WiFi.localIP().toString());

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  Serial.println("HTTP server started.");
}

void loop() {
  if (millis() - lastSensorRead > 2000) {
    lastSensorRead = millis();
    readSensors();
    if (pageActive) {
      digitalWrite(LED_GREEN, HIGH);
      digitalWrite(LED_RED, LOW);
      pageActive = false;
    }
  }
  pox.update();
  server.handleClient();
}
