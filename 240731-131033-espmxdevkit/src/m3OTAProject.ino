#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESP8266WebServer.h>
#include <StreamString.h>
#define FOR(I,N) for(int I=0;I<N;I++)
const char*apid = "TIGO_m3a_A7";
const char*pswd = "12345678";
ESP8266WebServer server(80);
#define AT1_SLAVE 0x12
#define AT2_SLAVE 0x14

#define TF_OFF
#define ACC_OFF
#define CLR_OFF

#ifdef ACC_ON
  #include "SparkFun_LIS2DH12.h"
  SPARKFUN_LIS2DH12 accel;
  float z_acc=0.0;
#endif
#ifdef TF_ON
  #include <VL53L0X.h>
  VL53L0X sensor; //0x29
  int head=0;
#endif
#ifdef CLR_ON
  #include "veml6040.h"
  VEML6040 RGBWSensor;
  int red=0; int mred=0;
  int blue=0; int mblue=0; 
  int green=0; int mgreen=0;
#endif
//function header 
void TCA9548A(uint8_t bus);
void to_MotorA(int dir, int speed);
void to_MotorD(int dir, int speed);
void to_RGB(long color);
void to_WLED1(char val);
void signalling(int);
void scanI2CDevice();
StreamString htmlBuffer;
StreamString outputBuffer;
char ipString[16];

void handleRoot() {
  IPAddress localIp = WiFi.localIP();
  sprintf(ipString,"%d.%d.%d.%d" ,localIp[0],localIp[1],localIp[2],localIp[3]);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  //scanI2CDevice(); //only enable when needed.
  outputBuffer.replace("\n", "<br/>");
  htmlBuffer.clear();
  htmlBuffer.printf("<html><head><meta http-equiv='refresh' content='5'/></head>\
  <body>\
    <h1>esp m3 Data Output (Air Serial)</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <p>IP Adress: %s </p>\
    <p>Data: %s </p>\
  </body>\
</html>",
              hr, min % 60, sec % 60, ipString, outputBuffer.c_str());
  server.send(200, "text/html", htmlBuffer.c_str());
}

void setup() {
  htmlBuffer.reserve(200);  //approx 200 chars
  outputBuffer.reserve(200); //same
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apid, pswd);
  //IPAddress IP = WiFi.softAPIP();
  WiFi.begin("TIGO5G2.12", "abcdefab");
  // delay(1000);
  ArduinoOTA.begin();
  pinMode(2, OUTPUT);
  server.on("/", handleRoot);
  server.begin();
  Wire.setClock(400000); //set i2c fast mode.
  Wire.begin(4,14); //join i2c as master //sda4,scl14

}

void loop() {
  server.handleClient(); //use this to enable airSerial
  ArduinoOTA.handle(); //required for ota.
}
