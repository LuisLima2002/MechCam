#include <WiFi.h>
const char* ssid     = "ServerSimulation";
const char* password = NULL;

// Set web server port number to 80
WiFiServer server(80);
void setup() {
  Serial.begin(115200);
  // Connect to Wi-Fi network with SSID and password
  Serial.println("\n[*] Creating AP");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  Serial.print("[+] AP Created with IP Gateway ");
  Serial.println(WiFi.softAPIP());
  server.begin();
}

void loop(){
	
}