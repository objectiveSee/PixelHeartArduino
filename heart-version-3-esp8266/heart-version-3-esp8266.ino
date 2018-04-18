#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WebSocketsServer.h>
#include <Hash.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>

#include "build.h" // should be first of user defined ones
#include "webpage.h"
#include "animations.h"

/**
 * Wifi
 */
MDNSResponder mdns;
ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

const char* ssid     = "Stream";
const char* password = "ChesapeakeBayFoundation";

/** 
 * Light modes as enum. 
 * NOTE: LIGHT_MODE_OFF is after the LIGHT_MODE_COUNT so it isn't cycled through.
 */
typedef enum {LIGHT_MODE_RED = 0, LIGHT_MODE_ORANGE, LIGHT_MODE_GREEN, LIGHT_MODE_BLUE, LIGHT_MODE_RAINBOW, LIGHT_MODE_COUNT, LIGHT_MODE_OFF} LightMode;
#define DEFAULT_LIGHT_MODE LIGHT_MODE_RAINBOW
static int lightMode = DEFAULT_LIGHT_MODE;

Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS_PER_STRIP, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

/**
 * On-board LED properties
 */
const int ledPin = 0;
bool LEDStatus;
#define BLINK_DURATION 100    // duration of blink when it's on
#define BLINK_INTERVAL 5000   // interval in ms that blinks happen at
#define BLINK_DURATION_DURING_SETUP 100    // duration of blink when it's on, during setup
#define BLINK_INTERVAL_DURING_SETUP 200   // interval in ms that blinks happen at, during setup

/**
 * Parameters edited via Wireless API
 */
int potentiometer_value; // 0 to 123 inclusive

/**
 * Wifi (ESP)
 */

void setupWifi() {
  Serial.print("WIFI Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, password);

  while(WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("Connected");
}

// Commands sent through Web Socket
const char LIGHTSON[] = "lightson";
const char LIGHTSOFF[] = "lightsoff";
const char ANIMATIONNEXT[] = "animationnext";
const char ANIMATIONPREVIOUS[] = "animationprevious";

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
  Serial.printf("webSocketEvent(%d, %d, ...)\r\n", num, type);
  switch(type) {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\r\n", num);
      break;
    case WStype_CONNECTED:
      {
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connection from %d.%d.%d.%d url: %s\r\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        // Send the current LED status
        if (LEDStatus) {
          webSocket.sendTXT(num, LIGHTSON, strlen(LIGHTSON));
        }
        else {
          webSocket.sendTXT(num, LIGHTSOFF, strlen(LIGHTSOFF));
        }
      }
      break;
    case WStype_TEXT:
      static int saved_mode = DEFAULT_LIGHT_MODE;
      Serial.printf("[%u] get Text: %s\r\n", num, payload);

      if (strcmp(LIGHTSOFF, (const char *)payload) == 0) {
        saved_mode = lightMode;
        lightMode = LIGHT_MODE_OFF;
        writeLED(false);
      }
      else if (strcmp(LIGHTSON, (const char *)payload) == 0) {
        lightMode = saved_mode;
        writeLED(true);
      } else if (strcmp(ANIMATIONPREVIOUS, (const char *)payload) == 0) {                
        lightMode = ((lightMode-1+LIGHT_MODE_COUNT) % LIGHT_MODE_COUNT);  // must add `LIGHT_MODE_COUNT` because % returns a negative otherwise
        Serial.print("Light Mode Previous "); Serial.println(lightMode);
      }
      else if (strcmp(ANIMATIONNEXT, (const char *)payload) == 0) {
        lightMode = ((lightMode+1) % LIGHT_MODE_COUNT);
        Serial.print("Light Mode Next "); Serial.println(lightMode);
      } else {
        Serial.println("Unknown command");
      }
      // send data to all connected clients
      webSocket.broadcastTXT(payload, length);
      break;
    case WStype_BIN:
      Serial.printf("[%u] get binary length: %u\r\n", num, length);
      hexdump(payload, length);

      // echo data back to browser
      webSocket.sendBIN(num, payload, length);
      break;
    default:
      Serial.printf("Invalid WStype [%d]\r\n", type);
      break;
  }
}

void handleRoot()
{
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setupServers() {
  unsigned long started = millis();
  Serial.println("Setting up servers");
  server.on("/", handleRoot);
  Serial.println("[Setup Done] root");
  server.onNotFound(handleNotFound);
  Serial.println("[Setup Done] not found");
  server.begin();
  Serial.println("[Setup Done] Server Begin");
  webSocket.begin();
  Serial.println("[Setup Done] Websocker Begin");
  webSocket.onEvent(webSocketEvent);
  Serial.println("[Setup Done] Websocket Event");

  Serial.print("[Setup Done] Entire setup took ");
  unsigned long duration = millis() - started;
  Serial.print(duration); Serial.println("ms");
}

void setupDNS () {
  unsigned long start = millis();
  if (mdns.begin("infinityMirror", WiFi.localIP())) {
    Serial.println("MDNS responder started");
    mdns.addService("http", "tcp", 80);
    mdns.addService("ws", "tcp", 81);
  }
  else {
    Serial.println("MDNS.begin failed");
  }
  Serial.print("Connect to http://infinityMirror.local or http://");
  Serial.println(WiFi.localIP());
  unsigned long duration = millis() - start;
  Serial.print("MDNS setup took "); Serial.print(duration); Serial.println("ms");
}

String wifiStatusAsString(int wifi_status) {

    if (wifi_status == WL_CONNECTED)
      return "connected";
    else if (wifi_status == WL_NO_SSID_AVAIL)
      return "no SSID available";
    else if (wifi_status == WL_CONNECT_FAILED )
      return "connection failed";
    else if (wifi_status == WL_IDLE_STATUS)
      return "idle";
    else if (wifi_status == WL_DISCONNECTED)
      return "disconnected";
    else
      return "unknown";
}

static bool wifi_connected= false;
void loopWifi() {
  static uint8_t last_status = 0xAF;  // initalize to a value not defined in `wl_tcp_state`
  uint8_t current_status = WiFiMulti.run();

  if ( last_status == current_status ) {
    return;
  }
  last_status = current_status;
  wifi_connected = (current_status == WL_CONNECTED);
  Serial.print("Wifi status changed. State (");
  Serial.print(current_status); Serial.print(") = ");
  Serial.println(wifiStatusAsString(current_status));
  if ( wifi_connected ) {
    Serial.println("Setting up servers.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP()); 
    setupServers();     // TODO: Call everytime or just once?
    setupDNS();         // TODO: Call everytime or just once? Testing on re-connects to Wifi needed.
  }
}

static void writeLED(bool LEDon)
{
  LEDStatus = LEDon;
#ifdef USE_ONBOARD_LED
  // Note inverted logic for Adafruit HUZZAH board
  if (LEDon) {
    digitalWrite(ledPin, 0);
  }
  else {
    digitalWrite(ledPin, 1);
  }
#endif
}


/**
 * Arduino Interface
 */

void setup() {
  
  Serial.setTimeout(500);
  Serial.begin(115200);

  strip.setBrightness(10);  // TODO: Debugging only
  strip.begin();
  allColor(0);
  strip.show();

#ifdef USE_ONBOARD_LED
  pinMode(ledPin, OUTPUT);
#endif
  writeLED(false);

  for(uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] BOOT WAIT %d...\r\n", t);
    Serial.flush();
    delay(1000);
  }
  setupWifi();
}


void loopLightAnimations() {

//  uint16_t i, j;
//
//  j = (millis() / 10) % 256;
//
//  for(i=0; i<strip.numPixels(); i++) {
//    strip.setPixelColor(i, Wheel((i+j) & 255));
//  }
//  strip.show();
//  return;

  switch (lightMode) {
  
    case LIGHT_MODE_RED:
      lightAnimationRed();
      break;
  
    case LIGHT_MODE_ORANGE:
      lightAnimationOrange();
      break;
  
    case LIGHT_MODE_GREEN:
      lightAnimationGreen();
      break;
  
    case LIGHT_MODE_BLUE:
      lightAnimationBlue();
      break;

    case LIGHT_MODE_RAINBOW:
      lightAnimationRainbow();
      break;
  
    case LIGHT_MODE_OFF:
      lightAnimationOff();
      break;
    
    break;
    default:
      Serial.print("Unknown light mode "); Serial.println(lightMode);
      allColor(0x000000);
    break;
  }
}

void logLightMode(int mode) {
  Serial.print("Light mode is "); Serial.println(mode);
}

void loop() {
  loopWifi();
  webSocket.loop();
  server.handleClient();
  loopLightAnimations();
//  logLightMode(lightMode);
  delay(10);
}

