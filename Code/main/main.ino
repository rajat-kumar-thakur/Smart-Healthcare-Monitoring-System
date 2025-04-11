/**************************************************************
 *  Includes
 **************************************************************/
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// DS18B20 Libraries
#include <OneWire.h>
#include <DallasTemperature.h>

// DHT22 Library
#include <DHT.h>

// MAX30100 Library (example library)
#include <Wire.h>
#include "MAX30100_PulseOximeter.h" 
// ^ If using a different library, adjust accordingly

/**************************************************************
 *  Pin Definitions
 **************************************************************/
// Wi-Fi credentials
const char* ssid = "Billo_Bagge";
const char* password = "rajat123";

// DS18B20 on D4
#define ONE_WIRE_BUS  D4   // GPIO2 on NodeMCU
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// DHT22 on D3
#define DHTPIN D3          // GPIO0 on NodeMCU
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// MAX30100 on D1 (SCL) and D2 (SDA)
// On NodeMCU, D1=GPIO5, D2=GPIO4
PulseOximeter pox;   // if using the PulseOximeter-based library

// LED pins
#define LED_GREEN D5       // GPIO14
#define LED_RED   D6       // GPIO12

/**************************************************************
 *  Web Server on Port 80
 **************************************************************/
ESP8266WebServer server(80);

/**************************************************************
 *  Variables for Sensor Readings
 **************************************************************/
// We'll store the latest reading globally each time the page is requested
float roomTemp = 0.0;
float roomHumidity = 0.0;
float dsTemp = 0.0;  // DS18B20 reading
float heartRate = 0.0;
float spo2 = 0.0;

// We’ll define “safe” ranges for demonstration. Adjust to your needs!
bool isAllSafe = false;

// Example “safe” ranges
// Feel free to refine these as needed:
float SAFE_ROOMTEMP_MIN = 18.0;  // °C
float SAFE_ROOMTEMP_MAX = 28.0;  // °C

float SAFE_ROOMHUMID_MIN = 30.0; // %
float SAFE_ROOMHUMID_MAX = 60.0; // %

float SAFE_BODYTEMP_MIN = 35.0;  // °C
float SAFE_BODYTEMP_MAX = 38.0;  // °C

float SAFE_HR_MIN      = 50.0;   // BPM
float SAFE_HR_MAX      = 110.0;  // BPM

float SAFE_SPO2_MIN    = 90.0;   // %
float SAFE_SPO2_MAX    = 100.0;  // (usually up to 100%)

/**************************************************************
 *  Read Sensors
 **************************************************************/
void readSensors() {
  // --- DHT22 ---
  roomHumidity = dht.readHumidity();
  roomTemp = dht.readTemperature(); // in °C (room temperature)
  
  // --- DS18B20 ---
  ds18b20.requestTemperatures();
  dsTemp = ds18b20.getTempCByIndex(0); // single DS18B20 sensor reading in °C
  // (Assume this is your “body temperature” reading)

  // --- MAX30100 (Heart Rate, SpO2) ---
  // For demonstration, we’ll call .update() a few times
  // Realistically, you should handle continuous reading in loop() or with interrupts.
  // The PulseOximeter library typically calls pox.update() frequently.
  // Then pox.getHeartRate() and pox.getSpO2() are updated automatically.
  for (int i = 0; i < 30; i++) {
    pox.update();
    delay(10);
  }
  heartRate = pox.getHeartRate();
  spo2      = pox.getSpO2();
  
  // If sensor read fails or returns NaN, handle accordingly
  if (isnan(roomHumidity) || isnan(roomTemp)) {
    roomHumidity = 0.0;
    roomTemp = 0.0;
  }
  if (dsTemp == DEVICE_DISCONNECTED_C) {
    dsTemp = 0.0; // DS18B20 read error
  }
  if (heartRate < 1) {
    // Possibly no finger on sensor or reading error
    heartRate = 0.0;
  }
  if (spo2 < 1) {
    spo2 = 0.0;
  }
}

/**************************************************************
 *  Check if all values are "safe"
 **************************************************************/
bool checkAllSafe() {
  bool safe = true;
  
  // Check each parameter against min/max thresholds
  if (roomTemp < SAFE_ROOMTEMP_MIN || roomTemp > SAFE_ROOMTEMP_MAX) safe = false;
  if (roomHumidity < SAFE_ROOMHUMID_MIN || roomHumidity > SAFE_ROOMHUMID_MAX) safe = false;
  if (dsTemp < SAFE_BODYTEMP_MIN || dsTemp > SAFE_BODYTEMP_MAX) safe = false;
  if (heartRate < SAFE_HR_MIN || heartRate > SAFE_HR_MAX) safe = false;
  if (spo2 < SAFE_SPO2_MIN || spo2 > SAFE_SPO2_MAX) safe = false;
  
  return safe;
}

/**************************************************************
 *  Handle Root Request
 **************************************************************/
void handleRoot() {
  // Read fresh sensor values
  readSensors();
  
  // Determine if all in safe range
  isAllSafe = checkAllSafe();
  
  // Control LEDs
  if (isAllSafe) {
    digitalWrite(LED_GREEN, HIGH);  // Turn green LED ON
    digitalWrite(LED_RED, LOW);     // Turn red LED OFF
  } else {
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
  }

  // Build the dynamic HTML with real sensor data
  // We’ll directly embed the user-provided HTML, 
  // inserting sensor readings in the placeholders.
  String page = R"=====(
<!DOCTYPE html>
<html>
<head>
    <title>Patient Health Monitoring</title>
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.7.2/css/all.min.css">
    <style>
        body {
            margin: 0;
            padding: 0;
            background-color: #fff;
            font-family: sans-serif;
            color: #333;
            font-size: 12px;
            box-sizing: border-box;
        }
        #page {
            margin: 10px auto;
            background-color: #fff;
            max-width: 600px;
        }
        .header {
            padding: 10px;
            text-align: center;
        }
        .header h1 {
            margin: 0;
            font-size: 28px;
            color: #008080;
            font-family: Garamond, 'sans-serif';
        }
        h2 {
            font-size: 16px;
            margin-top: 10px;
            margin-bottom: 10px;
            border-bottom: 1px solid #eee;
            text-align: left;
        }
        .box-full {
            padding: 10px;
            border: 1px solid #ddd;
            border-radius: 8px;
            box-shadow: 1px 4px 6px rgba(0, 0, 0, 0.2);
            background: #fff;
            width: auto;
        }
        .sensor {
            margin: 8px 0;
            font-size: 1.5rem;
        }
        .sensor-labels {
            font-size: 0.9rem;
            vertical-align: middle;
        }
        .units {
            font-size: 1rem;
        }
        hr {
            height: 1px;
            background-color: #eee;
            border: none;
            margin: 5px 0;
        }
        .developed-by {
            margin-top: 10px;
            padding: 5px;
            text-align: center;
            background-color: #f8f9fa;
            border-top: 1px solid #ddd;
            border-radius: 0 0 8px 8px;
        }
        .developed-by h3 {
            margin: 5px 0;
            font-size: 14px;
            color: #444;
        }
        .developed-by p {
            margin: 2px 0;
            font-size: 12px;
        }
    </style>
</head>
<body>
    <div id="page">
        <!-- Header Section -->
        <div class="header">
            <h1>WSN Based Smart Healthcare Monitoring System</h1>
        </div>

        <!-- Sensor Data Section -->
        <div class="box-full" align="left">
            <h2>Sensors Readings</h2>

            <!-- Temperature (Room) -->
            <p class="sensor">
                <i class="fas fa-thermometer-half" style="color:#0275d8"></i>
                <span class="sensor-labels"> Room Temperature </span>
                )=====";
  page += String(roomTemp, 1);
  page += R"=====(
                <sup class="units">°C</sup>
            </p>
            <hr>

            <!-- Humidity -->
            <p class="sensor">
                <i class="fas fa-tint" style="color:#5bc0de"></i>
                <span class="sensor-labels"> Room Humidity </span>
                )=====";
  page += String(roomHumidity, 1);
  page += R"=====(
                <sup class="units">%</sup>
            </p>
            <hr>

            <!-- Heart Rate -->
            <p class="sensor">
                <i class="fas fa-heartbeat" style="color:#cc3300"></i>
                <span class="sensor-labels"> Heart Rate </span>
                )=====";
  page += String(heartRate, 0);
  page += R"=====(
                <sup class="units">BPM</sup>
            </p>
            <hr>

            <!-- SpO2 -->
            <p class="sensor">
                <i class="fas fa-burn" style="color:#f7347a"></i>
                <span class="sensor-labels"> SpO2 </span>
                )=====";
  page += String(spo2, 0);
  page += R"=====(
                <sup class="units">%</sup>
            </p>
            <hr>

            <!-- Body Temperature (DS18B20) -->
            <p class="sensor">
                <i class="fas fa-thermometer-full" style="color:#d9534f"></i>
                <span class="sensor-labels"> Body Temperature </span>
                )=====";
  page += String(dsTemp, 1);
  page += R"=====(
                <sup class="units">°C</sup>
            </p>
        </div>

        <!-- Developed by Section -->
        <div class="developed-by">
            <h3>Developed by</h3>
            <p><strong>Isha Jangir</strong> (202211031)</p>
            <p><strong>Rajat Kumar Thakur</strong> (202211070)</p>
        </div>
    </div>
</body>
</html>
)=====";

  // Send the dynamic page
  server.send(200, "text/html", page);
}

/**************************************************************
 *  Setup
 **************************************************************/
void setup() {
  Serial.begin(115200);

  // Initialize pins for LEDs
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  digitalWrite(LED_GREEN, LOW);
  digitalWrite(LED_RED, LOW);

  // Initialize sensors
  dht.begin();           // DHT22
  ds18b20.begin();       // DS18B20

  // MAX30100
  Wire.begin(D2, D1);    // SDA, SCL
  if (!pox.begin()) {
    Serial.println("MAX30100 init failed. Check wiring or try again.");
  } else {
    Serial.println("MAX30100 ready!");
  }

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to WiFi (");
  Serial.print(ssid);
  Serial.print(")");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Setup server routes
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started.");
}

/**************************************************************
 *  Main Loop
 **************************************************************/
void loop() {
  // In many MAX30100 libraries, you call pox.update() regularly
  pox.update();

  // ESP8266 Web Server handle
  server.handleClient();
}
