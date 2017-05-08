#include <WebSockets.h>
#include <WebSocketsServer.h>
#include <WebSocketsClient.h>

/*
  Simple example for receiving

  https://github.com/sui77/rc-switch/
*/

#include <RCSwitch.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

#define min(x, y) (x < y ? x : y)

#define FIRMWARE_NAME "my_firmware"
#define FIRMWARE_VERSION "1"

#define wifi_ssid "flatlusen"
#define wifi_password "jaggedtulip158"

#define mqtt_name "postlund"
#define mqtt_server "192.168.1.104"
#define mqtt_user "joel-mqtt"
#define mqtt_password "aflt0123"

#define mailBox_topic "sensor/mailBox"

#define send_topic "switch/light"

#define firmware_topic "ota/sensor/demo_current_firmware/state"
#define firmware_version_topic "ota/sensor/demo_current_version/state"

const int receive_pin = D2;
const int transmit_pin = D5;

WiFiClient espClient;
PubSubClient client(espClient);

WebSocketsClient webSocket;

RCSwitch mySwitch = RCSwitch();

void debugWsPrint(const char* format, ...) {
  
  char buffer[256];
  va_list args;
  va_start (args, format);
  vsnprintf (buffer,256,format, args);
  va_end (args);

  webSocket.sendTXT(buffer);
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t lenght) {


    switch(type) {
        case WStype_DISCONNECTED:
            //Serial.printf("[WSc] Disconnected!");
            break;
        case WStype_CONNECTED:
            {
                Serial.printf("[WSc] Connected to url: %s",  payload);
        
          // send message to server when Connected
        webSocket.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[WSc] get text: %s", payload);

      // send message to server
      // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            Serial.printf("[WSc] get binary lenght: %u", lenght);
            hexdump(payload, lenght);

            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

void setup() {
  Serial.begin(115200);
  mySwitch.enableReceive(receive_pin);
  Serial.print("Setup complete 2");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  webSocket.begin("192.168.1.106", 8000);
  //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
  webSocket.onEvent(webSocketEvent);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  ArduinoOTA.setPort(8266);

  ArduinoOTA.onStart([]() {
    Serial.println("Start updating");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    debugWsPrint("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect(mqtt_name, mqtt_user, mqtt_password)) {
      debugWsPrint("connected");
    } else {
      debugWsPrint("failed, rc=%d ", client.state());
      
      debugWsPrint(" try again in 5 seconds ");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  client.subscribe(send_topic);
}

void callback(char* topic, byte* payload, unsigned int length) {
  char buf[32];
  int end = min(31, length);
  memcpy(buf, payload, end);
  buf[end] = '\0';


  unsigned long value = strtoul(buf, NULL, 10);
  debugWsPrint("%s:%lu", topic, value);

  mySwitch.disableReceive();
  mySwitch.enableTransmit(transmit_pin);

  mySwitch.setPulseLength(250);
  mySwitch.setRepeatTransmit(10);

  mySwitch.send(value, 32);

  mySwitch.disableTransmit();
  mySwitch.enableReceive(receive_pin);
}

int retryAttempts = 0;
void loop() {
  
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  webSocket.loop();
  
  ArduinoOTA.handle();
  if (mySwitch.available()) {

    int value = mySwitch.getReceivedValue();

    if (value == 0) {
      debugWsPrint("Unknown encoding");
    } else {
      client.publish(mailBox_topic, "ON");
      client.publish(firmware_version_topic, FIRMWARE_VERSION);
      client.publish(firmware_topic, FIRMWARE_NAME);
      debugWsPrint("Received %lu", mySwitch.getReceivedValue());
      debugWsPrint("Received length %u", mySwitch.getReceivedBitlength());
      
      debugWsPrint("Received protocol %d", mySwitch.getReceivedProtocol());
    }

    mySwitch.resetAvailable();
  }
}
