#include <Stepper.h>
#include "camera_pins.h"
#include <WiFi.h>
#include "esp_camera.h"
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include <iostream>
#include <string>
using namespace std;
#define EEPROM_SIZE 64
      
String ssid;
String password;
const int stepsPerRevolution = 1024; 
const char* nameWifi     = "MechCam";
const char* passwordWifi = NULL;
int64_t timeToRead=0;

int angle = 0;
// stepsPerRevolution = 180 graus
// ULN2003 Motor Driver Pins
#define IN1 12
#define IN2 13
#define IN3 15
#define IN4 14
#define angleAdress 0
#define GPIO_SENSOR_GAS 2
const float adjustAngle = float(stepsPerRevolution)/180.0;
Stepper myStepper(stepsPerRevolution, IN1, IN3, IN2, IN4);
float R0;
int n=10;
WebSocketsClient webSocket;
void writeAngleIntoEEPROM(int number)
{ 
  EEPROM.write(angleAdress, number);
  EEPROM.commit();
}

int readAngleFromEEPROM()
{
  return EEPROM.read(angleAdress);
}

void writeSSIDIntoEEPROM(String ssid)
{ 
  int adress = angleAdress + sizeof(angle);
  EEPROM.writeString(adress, ssid);
  EEPROM.commit();
}

String readSSIDFromEEPROM()
{
  int adress = angleAdress + sizeof(angle);
  return EEPROM.readString(adress);
}

void writePasswordIntoEEPROM(String password)
{ 
  int adress = angleAdress + sizeof(angle) +sizeof(ssid)+10;//safe space;
  EEPROM.writeString(adress, password);
  EEPROM.commit();
}

String readPasswordFromEEPROM()
{
  int adress = angleAdress + sizeof(angle) +sizeof(ssid)+10;//safe space;
  return EEPROM.readString(adress);
}
framesize_t convert(const char *str)
{
    if(strcmp(str,"FRAMESIZE_96X96")==0) return FRAMESIZE_96X96;
    else if(strcmp(str,"FRAMESIZE_240X240")==0) return FRAMESIZE_240X240;
    else if(strcmp(str,"FRAMESIZE_QQVGA")==0) return FRAMESIZE_QQVGA;
    else if(strcmp(str,"FRAMESIZE_UXGA")==0) return FRAMESIZE_UXGA;
}

void moveTo(int value){
  value = constrain(value,-175,175);
  Serial.printf("move to %d and now is %d with adjustAngle equal to %f\n",value,angle,adjustAngle);
  int step = (value-angle)*(adjustAngle);
  myStepper.step(step);
  angle=value;
  writeAngleIntoEEPROM(angle);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
  const uint8_t size = JSON_OBJECT_SIZE(5);
  StaticJsonDocument<size> json;
  deserializeJson(json, payload);
  const char *action = json["action"];

  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[WSc] Disconnected!\n");
      break;
    case WStype_CONNECTED: {
      Serial.printf("[WSc] Connected to url: %s\n", payload);
    }
      break;
    case WStype_TEXT:
      Serial.printf("[WSc] get text: %s\n", payload);
      if(strcmp(action,"resolution")==0){
        Serial.printf("Changing resolution\n");
        const char *value = json["value"];
        framesize_t resolution = convert(value);
        sensor_t * s = esp_camera_sensor_get();
        delay(1000);
        s->set_framesize(s,resolution);
        delay(1000);
      }
      else if(strcmp(action,"angle")==0){
        Serial.printf("Changing angle\n");
        const int value = json["value"];
        moveTo(value);
      }
      break;

}
}

void setupCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 8;
    config.fb_count = 1;
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); 
    s->set_brightness(s, 1); 
    s->set_saturation(s, -2); 
  }
  s->set_framesize(s,FRAMESIZE_240X240);
}

void createServer(){
WiFiServer server(80);
String header;
unsigned long currentTime = millis();
unsigned long previousTime = 0; 
const long timeoutTime = 2000;
WiFi.disconnect();
WiFi.mode(WIFI_AP);
WiFi.softAP(nameWifi,passwordWifi);
Serial.println("Starting Server");
Serial.print("[+] AP Created with IP Gateway ");
Serial.println(WiFi.softAPIP());
server.begin();
while(true){
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
      if(header.indexOf("GET /?credentials=")>=0 && c  == '\n') {
          Serial.print("Header recebido: ");
          Serial.println(header);
          int pos1 = header.indexOf('=');
          int pos2 = header.indexOf(']');
          int pos3 = header.indexOf('&');
          ssid = header.substring(pos1+1, pos2);
          ssid.replace("}"," ");
          password = header.substring(pos2+1, pos3);
          Serial.println(ssid);
          Serial.println(password);
          writeSSIDIntoEEPROM(ssid);
          writePasswordIntoEEPROM(password);
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:application/json");
          client.println();
          client.printf("{\"status\":\"ok mm\"}");
          client.stop();
          WiFi.mode(WIFI_STA);
          server.close();
          return;
        }else if (c != '\r') {  // if you got anything else but a carriage return character,
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
}

void calculateR0(){
    delay(20000);
    float sensorValue = 0;
    for (int i=0; i<50;i++){
        sensorValue += analogRead(GPIO_SENSOR_GAS);
        delay(100);
     }

    sensorValue = sensorValue/100;

    float sensorVolt = sensorValue/4096*5.0;
    Serial.print("SensorVolt: ");
    Serial.println(sensorVolt);
    float RS_air = (5.0 - sensorVolt)/sensorVolt;
    R0 = RS_air/9.83;
    Serial.print("RO: ");
    Serial.println(R0);
}

void setup() {
  Serial.begin(115200);
  pinMode(GPIO_SENSOR_GAS, INPUT);
  calculateR0();

  EEPROM.begin(EEPROM_SIZE);
  ssid = readSSIDFromEEPROM();
  password = readPasswordFromEEPROM();
  angle = readAngleFromEEPROM();
  myStepper.setSpeed(10);
  Serial.println(ssid);
  Serial.println(password);
  const char * s = ssid.c_str();
  const char * p = password.c_str();
  WiFi.begin(s, p);
  int j =0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(j);
    j++;
    if(j>50){
      createServer();
      const char * s = ssid.c_str();
      const char * p = password.c_str();
      WiFi.begin(s, p);
      j=0;
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");


  webSocket.begin("192.168.4.2", 3000, "/jpgstream_server");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  webSocket.enableHeartbeat(15000, 3000, 2);
  setupCamera();

}


void sendImage(){
int64_t fr_start = esp_timer_get_time();
  webSocket.loop();
  camera_fb_t * fb = NULL;

  // Take Picture with Camera
  fb = esp_camera_fb_get();  
  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }     
  webSocket.sendBIN(fb->buf,fb->len);
  esp_camera_fb_return(fb); 
  int64_t fr_end = esp_timer_get_time();
  // Serial.printf("Image sent. %ums. FPS: %u\n", (uint32_t)((fr_end - fr_start)/1000),(uint32_t)(1000000/((fr_end - fr_start))));
}

int getRatio(){
    float sensor_volt = 0;
    float sensorValue = analogRead(GPIO_SENSOR_GAS);
    float RS_gas = 0; //Get value of RS in a GAS

    sensor_volt= sensorValue/4096*5.0;
    RS_gas = (5.0-sensor_volt)/sensor_volt;
 
    int ratio = (RS_gas*100)/R0; 
    return ratio;
}

void loop() {
  if(timeToRead<esp_timer_get_time()){
    WiFi.disconnect(true);
    n = getRatio();
    Serial.println(n);
    uint8_t * buf = (uint8_t *) &n;
    size_t buf_len = sizeof(n);
    const char * s = ssid.c_str();
    const char * p = password.c_str();
    WiFi.begin(s, p);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
    }
    timeToRead = esp_timer_get_time() + 30000000;
  }
  sendImage();
  webSocket.loop();
  uint8_t * buf = (uint8_t *) &n;
  size_t buf_len = sizeof(n);
  webSocket.sendBIN(buf,buf_len);
}
