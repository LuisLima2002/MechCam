#include <WiFi.h>
const char* ssid     = "MechCam";
const char* password = NULL;

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Decode HTTP GET value
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

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
WiFiClient client = server.available();   // Listen for incoming clients

if (client) {                             // If a new client connects,
  currentTime = millis();
  previousTime = currentTime;
  String currentLine = "";                // make a String to hold incoming data from the client
  while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
    currentTime = millis();
    if (client.available()) {             // if there's bytes to read from the client,
      char c = client.read();             // read a byte, then
      header += c;
      if (c == '\n') {                    // if the byte is a newline character
        if (currentLine.length() == 0) {
          // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
          // and a content-type so the client knows what's coming, then a blank line:
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:text/html");
          client.println("Connection: close");
          client.println();
          // Display the HTML web page
          client.println("<!DOCTYPE html><html>");
          client.println("<head>");
          client.println("<link rel=\"icon\" href=\"data:,\">");
          // CSS to style the on/off buttons 
          // Feel free to change the background-color and font-size attributes to fit your preferences
          client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
          client.println(".slider { width: 300px; }</style>");
          // Web Page
          client.println("</head><body><h1>Barra e bola</h1>");
          client.println("<p>Position: <span id=\"servoPos\"></span></p>");          
          client.println("<input type=\"range\" min=\"50\" max=\"135\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\""+valueString+"\"/>");
          client.println("<h1 id=\"distanceT\"/></h1>");
          client.println("</body></html>");     
          // The HTTP response ends with another blank line
          client.println();
          if(valueString=="1"){
            valueString = "1";
          }else{
            valueString = "0";
          }
          // Break out of the while loop
          break;
        } else { // if you got a newline, then clear currentLine
          currentLine = "";
        }
      } else if (c != '\r') {  // if you got anything else but a carriage return character,
        currentLine += c;      // add it to the end of the currentLine
      }
    }
  }
  // Clear the header variable
  header = "";
  // Close the connection
  client.stop();
}

	
}