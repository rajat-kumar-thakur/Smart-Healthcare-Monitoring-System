#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Replace with your Wi-Fi credentials
const char* ssid = "Billo_Bagge";
const char* password = "rajat123";

// Create a web server object on port 80
ESP8266WebServer server(80);

// Function to handle root URL "/"
void handleRoot() {
  server.send(200, "text/html", "<h1>Hello from NodeMCU ESP8266!</h1>");
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Print local IP address
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Define routes
  server.on("/", handleRoot);

  // Start the server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();
}
