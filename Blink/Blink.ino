#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// Pin definitions
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

// Threshold for the photoresistor
#define TH 500

// WiFi credentials
const char* ssid = "SSID";  // Replace with your WiFi SSID
const char* password = "PASSWORD";  // Replace with your WiFi password

// Static IP configuration
IPAddress local_IP(192, 168, 0, 184);  // Replace with your desired static IP
IPAddress gateway(192, 168, 0, 1);     // Replace with your network gateway (router IP)
IPAddress subnet(255, 255, 255, 0);    // Subnet mask
IPAddress primaryDNS(8, 8, 8, 8);      // Optional: Primary DNS server
IPAddress secondaryDNS(8, 8, 4, 4);    // Optional: Secondary DNS server

// Create a web server on port 80
ESP8266WebServer server(80);

// Variables to hold the mode and LED state
bool isAutoMode = true;  // Start in auto mode
bool isLEDOnManual = false;  // Store manual LED state

void handleRoot() {
  int analogValue = analogRead(A0);  // Read the analog value from pin A0
  String message = "<html><body>";
  message += "<h1>ESP8266 Control Panel</h1>";
  message += "<p>Current Analog Value: ";
  message += String(analogValue);
  message += "</p>";
  
  message += "<p>Current Mode: ";
  message += isAutoMode ? "Auto" : "Manual";
  message += "</p>";

  message += "<p><a href=\"/on\"><button>Turn LED ON (Manual)</button></a></p>";
  message += "<p><a href=\"/off\"><button>Turn LED OFF (Manual)</button></a></p>";
  message += "<p><a href=\"/auto\"><button>Switch to Auto Mode</button></a></p>";
  message += "<p><a href=\"/manual\"><button>Switch to Manual Mode</button></a></p>";
  
  message += "</body></html>";
  server.send(200, "text/html", message);  // Send HTML response
}

void handleLEDOn() {
  if (!isAutoMode) {
    digitalWrite(LED_BUILTIN, LOW);  // Turn LED ON (LOW is ON for built-in LED)
    isLEDOnManual = true;  // Set manual LED state to ON
  }
  server.sendHeader("Location", "/");  // Redirect to root page
  server.send(303);  // HTTP status 303 See Other
}

void handleLEDOff() {
  if (!isAutoMode) {
    digitalWrite(LED_BUILTIN, HIGH);  // Turn LED OFF (HIGH is OFF for built-in LED)
    isLEDOnManual = false;  // Set manual LED state to OFF
  }
  server.sendHeader("Location", "/");  // Redirect to root page
  server.send(303);  // HTTP status 303 See Other
}

void handleAutoMode() {
  isAutoMode = true;  // Set system to auto mode
  server.sendHeader("Location", "/");  // Redirect to root page
  server.send(303);  // HTTP status 303 See Other
}

void handleManualMode() {
  isAutoMode = false;  // Set system to manual mode
  server.sendHeader("Location", "/");  // Redirect to root page
  server.send(303);  // HTTP status 303 See Other
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);  // Set LED as output
  digitalWrite(LED_BUILTIN, HIGH);  // Turn LED off by default

  Serial.begin(115200);  // Start serial communication

  // Connect to WiFi with static IP
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Failed to configure Static IP");
  }

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Static IP address: ");
  Serial.println(WiFi.localIP());  // Display the assigned static IP address

  // Define web server routes
  server.on("/", handleRoot);  // Handle root page
  server.on("/on", handleLEDOn);  // Handle LED ON request
  server.on("/off", handleLEDOff);  // Handle LED OFF request
  server.on("/auto", handleAutoMode);  // Switch to Auto mode
  server.on("/manual", handleManualMode);  // Switch to Manual mode

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Listen for incoming web requests

  if (isAutoMode) {
    // Auto mode: control the LED based on the analog value from the photoresistor
    int analogValue = analogRead(A0);
    Serial.println(analogValue);
    if (analogValue < TH) {
      digitalWrite(LED_BUILTIN, LOW);  // LED ON
    } else {
      digitalWrite(LED_BUILTIN, HIGH);  // LED OFF
    }
  } else {
    // Manual mode: LED state is controlled via web requests, no automatic behavior
    Serial.println(isLEDOnManual ? "Manual LED is ON" : "Manual LED is OFF");
  }

  delay(500);  // Wait 500 ms before next action
}
