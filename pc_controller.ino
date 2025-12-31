#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

struct PinInfo {
  const char* label;   // "D5"
  uint8_t gpio;        // GPIO number
};

// ------------------- CONFIGURATION -------------------
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Pin Definitions (Based on our previous diagram)
const int PIN_POWER_BTN = D1;  // GPIO 5 - PowerOff
const int PIN_RESET_BTN = D2;  // GPIO 4 - Reset
const int PIN_STATUS_LED = D5; // GPIO 14 - LED Sensing (Input)

// -----------------------------------------------------
// Helper function to check if pc is in on/off state
bool isPcOn() {
  const int totalSamples = 10;   // how many times to sample
  const int threshold    = 5;    // required ON detections
  const int intervalMs   = 100;  // delay between samples (ms)

  int onCount = 0;

  for (int i = 0; i < totalSamples; i++) {
    if (digitalRead(PIN_STATUS_LED) == LOW) { // LOW = PC ON
      onCount++;
    }
    delay(intervalMs);
  }

  return onCount >= threshold;
}

const char* wifiStatusToString(wl_status_t status) {
  switch (status) {
    case WL_CONNECTED:        return "WL_CONNECTED";
    case WL_NO_SSID_AVAIL:    return "WL_NO_SSID_AVAIL";
    case WL_CONNECT_FAILED:  return "WL_CONNECT_FAILED";
    case WL_WRONG_PASSWORD:  return "WL_WRONG_PASSWORD";
    case WL_DISCONNECTED:    return "WL_DISCONNECTED";
    case WL_IDLE_STATUS:     return "WL_IDLE_STATUS";
    default:                 return "WL_UNKNOWN";
  }
}

// Helper json builder function
String jsonAppend(String json, const char* key, const String& value, bool quoteValue = true) {
  // Remove trailing '}'
  if (json.endsWith("}")) {
    json.remove(json.length() - 1);
  }

  // Add comma if not empty object
  if (json.length() > 1) {
    json += ",";
  }

  // Add key
  json += "\"";
  json += key;
  json += "\":";

  // Add value
  if (quoteValue) {
    json += "\"";
    json += value;
    json += "\"";
  } else {
    json += value;
  }

  // Close object
  json += "}";

  return json;
}

// Create http server on port 80
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);

  // 1. Initialize Pins
  // Important: Initialize outputs to LOW immediately to prevent "phantom" presses on boot
  pinMode(PIN_POWER_BTN, OUTPUT);
  digitalWrite(PIN_POWER_BTN, LOW); 

  pinMode(PIN_RESET_BTN, OUTPUT);
  digitalWrite(PIN_RESET_BTN, LOW);

  pinMode(PIN_STATUS_LED, INPUT_PULLUP);


  // 2. Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());

  // 3. Define Web Routes
  server.on("/", handleRoot);
  server.on("/action/on", handleTurnOn);      // The requested GET URL
  server.on("/action/off", handleTurnOff);
  server.on("/action/reset", handleReset);    // Optional Reset
  server.on("/action/forceoff", handleForceOff); // Hard shutdown
  server.on("/status", handleStatus);         // Check if PC is ON/OFF
  server.on("/pin", handlePinsStatus);        // Debug Check Pin Status

  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
}

String getWifiInfoJson() {
  String json = "{}";

  json = jsonAppend(json, "connected", String(WiFi.status() == WL_CONNECTED), false);
  json = jsonAppend(json, "status", wifiStatusToString(WiFi.status()));
  json = jsonAppend(json, "ssid", WiFi.SSID());
  json = jsonAppend(json, "ip", WiFi.localIP().toString());
  json = jsonAppend(json, "mac", WiFi.macAddress());
  json = jsonAppend(json, "rssi", String(WiFi.RSSI()), false);
  json = jsonAppend(json, "gateway", WiFi.gatewayIP().toString());
  json = jsonAppend(json, "subnet", WiFi.subnetMask().toString());
  json = jsonAppend(json, "dns", WiFi.dnsIP().toString());
  json = jsonAppend(json, "channel", String(WiFi.channel()), false);

  return json;
}


// --- Handler Functions ---
void handleRoot() {
  String json = "{}";
  json = jsonAppend(json, "error", "false", false);
  json = jsonAppend(json, "connection", getWifiInfoJson(), false);

  server.send(200, "application/json", json);
}

// Trigger the Power Button (Short Press)
void handleTurnOn() {
  if (isPcOn()) {
    String json = jsonAppend("{}", "error", "false", false);
    json = jsonAppend(json, "message", "PC was already ON.");
    return server.send(200, "application/json", json);
  }
  
  // "Press" the button
  digitalWrite(PIN_POWER_BTN, HIGH);
  delay(800); // Hold for 800ms
  // "Release" the button
  digitalWrite(PIN_POWER_BTN, LOW);

  String json = jsonAppend("{}", "error", "false", false);
  json = jsonAppend(json, "message", "Button press triggered.");

  server.send(200, "application/json", json);
}

// Trigger the Power Button (Short Press)
void handleTurnOff() {
  if (!isPcOn()) {
    String json = jsonAppend("{}", "error", "false", false);
    json = jsonAppend(json, "message", "PC was already OFF.");
    return server.send(200, "application/json", json);
  }
  
  // "Press" the button
  digitalWrite(PIN_POWER_BTN, HIGH);
  delay(800); // Hold for 800ms
  // "Release" the button
  digitalWrite(PIN_POWER_BTN, LOW);

  String json = jsonAppend("{}", "error", "false", false);
  json = jsonAppend(json, "message", "Button press triggered.");

  server.send(200, "application/json", json);
}

// Trigger the Reset Button
void handleReset() {
  Serial.println("Received Reset Request");
  digitalWrite(PIN_RESET_BTN, HIGH);
  delay(800);
  digitalWrite(PIN_RESET_BTN, LOW);
  server.send(200, "application/json", "OK: Reset Button Pulsed");
}

// Force Off (Press and Hold for 6 seconds)
void handleForceOff() {
  if (!isPcOn()) {
    String json = jsonAppend("{}", "error", "false", false);
    json = jsonAppend(json, "message", "PC was already OFF.");
    return server.send(200, "application/json", json);
  }

  digitalWrite(PIN_POWER_BTN, HIGH);
  delay(7000); // Hold for 7s
  digitalWrite(PIN_POWER_BTN, LOW);

  String error = "false";
  String message = "force stop complete";
  bool isOn = isPcOn();
  if (isOn) {
    error = "true";
    message = "Something went wrong";
  }
  String json = jsonAppend("{}", "error", error, false);
  json = jsonAppend(json, "message", message);
  json = jsonAppend("{}", "pc_status", isOn ? "ON" : "OFF");

  server.send(200, "application/json", json);
}

// Check PC Status (Read Voltage Divider)
void handleStatus() {
  // ---- Build debug JSON ----
  String json = jsonAppend("{}", "is_on", isPcOn() ? "true" : "false", false);
  json = jsonAppend("{}", "pc_status", isPcOn() ? "ON" : "OFF");
  server.send(200, "application/json", json);
}

PinInfo pinsToRead[] = {
  { "D1", D1 },
  { "D2", D2 },
  { "D3", D3 },
  { "D4", D4 },
  { "D5", D5 },
  { "D6", D6 }
};

String readPinsState(const PinInfo pins[], size_t count) {
  String json = "{}";

  for (size_t i = 0; i < count; i++) {
    int state = digitalRead(pins[i].gpio);
    json = jsonAppend(json, pins[i].label, (state == LOW) ? "\"LOW\"" : "\"HIGH\"");
  }

  return json;
}

void handlePinsStatus() {
  const size_t pinCount = sizeof(pinsToRead) / sizeof(pinsToRead[0]);
  String json = "{}";
  json = jsonAppend(json, "pins", readPinsState(pinsToRead, pinCount), false);

  server.send(200, "application/json", json);
}


