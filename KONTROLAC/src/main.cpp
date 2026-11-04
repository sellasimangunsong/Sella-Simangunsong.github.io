#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "DHTesp.h"

// Library IR
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <ir_Gree.h>
#include <ir_Mitsubishi.h>

// ---------------- Pin Setup ----------------
#define DHT_PIN D1
#define IR_GREE_PIN D2
#define IR_MITSU_PIN D3

// ---------------- WiFi AP Setup ----------------
const char *apSSID = "ESP8266_AC_Server";
const char *apPassword = "12345678";

// ---------------- Objects ----------------
DHTesp dht;
ESP8266WebServer server(80);

IRGreeAC ac_gree(IR_GREE_PIN);
IRMitsubishiAC ac_mitsu(IR_MITSU_PIN);

// ---------------- Global Vars ----------------
float suhu = 0;
float kelembapan = 0;

// ---------------- Functions ----------------
void readDHT() {
  TempAndHumidity data = dht.getTempAndHumidity();
  suhu = data.temperature;
  kelembapan = data.humidity;

  Serial.printf("Suhu: %.1f °C | Kelembapan: %.1f %%\n", suhu, kelembapan);
}

String htmlPage() {
  String html = "<!DOCTYPE html><html><head><title>ESP8266 AC Control</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
  html += "<style>body{font-family:Arial;text-align:center;}button{padding:10px;margin:5px;}</style></head><body>";
  html += "<h2>ESP8266 AC Control + Monitoring</h2>";
  html += "<p><b>Suhu:</b> " + String(suhu) + " &deg;C</p>";
  html += "<p><b>Kelembapan:</b> " + String(kelembapan) + " %</p>";
  html += "<h3>Kontrol AC</h3>";
  html += "<button onclick=\"location.href='/api/gree/on'\">AC Gree ON</button>";
  html += "<button onclick=\"location.href='/api/gree/off'\">AC Gree OFF</button><br>";
  html += "<button onclick=\"location.href='/api/mitsu/on'\">AC Mitsubishi ON</button>";
  html += "<button onclick=\"location.href='/api/mitsu/off'\">AC Mitsubishi OFF</button>";
  html += "</body></html>";
  return html;
}

// ---------------- API Handlers ----------------
void apiGreeOn() {
  ac_gree.on();
  ac_gree.setTemp(24);
  ac_gree.setFan(1);
  ac_gree.send();
  server.send(200, "application/json", "{\"status\":\"AC Gree ON\"}");
}

void apiGreeOff() {
  ac_gree.off();
  ac_gree.send();
  server.send(200, "application/json", "{\"status\":\"AC Gree OFF\"}");
}

void apiMitsuOn() {
  ac_mitsu.on();
  ac_mitsu.setTemp(24);
  ac_mitsu.setFan(1);
  ac_mitsu.send();
  server.send(200, "application/json", "{\"status\":\"AC Mitsubishi ON\"}");
}

void apiMitsuOff() {
  ac_mitsu.off();
  ac_mitsu.send();
  server.send(200, "application/json", "{\"status\":\"AC Mitsubishi OFF\"}");
}

void apiSensor() {
  readDHT();
  String json = "{";
  json += "\"suhu\":" + String(suhu, 1) + ",";
  json += "\"kelembapan\":" + String(kelembapan, 1);
  json += "}";
  server.send(200, "application/json", json);
}

// ---------------- Web Handlers ----------------
void handleRoot() {
  readDHT();
  server.send(200, "text/html", htmlPage());
}

// ---------------- Setup & Loop ----------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\nESP8266 mulai...");

  dht.setup(DHT_PIN, DHTesp::DHT22);

  // WiFi Access Point
  WiFi.softAP(apSSID, apPassword);
  Serial.printf("ESP8266 jadi Access Point: %s\n", apSSID);
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  // Web server routes
  server.on("/", handleRoot);

  // API routes
  server.on("/api/gree/on", apiGreeOn);
  server.on("/api/gree/off", apiGreeOff);
  server.on("/api/mitsu/on", apiMitsuOn);
  server.on("/api/mitsu/off", apiMitsuOff);
  server.on("/api/sensor", apiSensor);

  server.begin();
  Serial.println("Web server aktif.");
}

void loop() {
  server.handleClient();
}
