#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Wire.h>
#include "MAX30100_PulseOximeter.h"

#define REPORTING_PERIOD_MS 5000  // Read every 5 seconds

const char* ssid = "Billo_Bagge";
const char* password = "rajat123";

PulseOximeter pox;
float BPM, SpO2;
uint32_t tsLastReport = 0;

ESP8266WebServer server(80);

void setup() {
  Serial.begin(9600);
  delay(100);
  pinMode(16, OUTPUT);

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected.");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");

  Serial.print("Initializing MAX30100 sensor...");
  if (!pox.begin()) {
    Serial.println("FAILED");
    while (1);
  } else {
    Serial.println("SUCCESS");
  }
}

void loop() {
  server.handleClient();
  pox.update();

  if (millis() - tsLastReport > REPORTING_PERIOD_MS) {
    BPM = pox.getHeartRate();
    SpO2 = pox.getSpO2();

    Serial.print("BPM: ");
    Serial.print(BPM);
    Serial.print(" | SpO2: ");
    Serial.print(SpO2);
    Serial.println("%");
    Serial.println("************************");

    tsLastReport = millis();
  }
}

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML(BPM, SpO2));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float BPM, float SpO2) {
  String html = R"rawliteral(
    <!DOCTYPE html><html><head>
    <title>ESP8266 MAX30100</title>
    <meta name='viewport' content='width=device-width, initial-scale=1'>
    <style>
      body { font-family: Arial; text-align: center; background-color: #f0f0f0; }
      .card { background: white; padding: 20px; margin: 20px auto; width: 300px; border-radius: 10px; box-shadow: 0 4px 6px rgba(0,0,0,0.1); }
      h1 { color: teal; }
      .reading { font-size: 2rem; margin: 10px 0; }
    </style>
    <script>
      setInterval(() => {
        fetch("/")
          .then(response => response.text())
          .then(data => document.documentElement.innerHTML = data);
      }, 5000);
    </script>
    </head><body>
    <div class="card">
      <h1>MAX30100 Readings</h1>
      <div class="reading">💓 Heart Rate: <strong>)rawliteral" + String((int)BPM) + R"rawliteral( BPM</strong></div>
      <div class="reading">🩸 SpO2: <strong>)rawliteral" + String((int)SpO2) + R"rawliteral(%</strong></div>
    </div>
    </body></html>
  )rawliteral";

  return html;
}
