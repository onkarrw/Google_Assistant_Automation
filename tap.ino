#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define tapPin 2

#define WLAN_SSID       "Xae15Bhy8" //our SSID
#define WLAN_PASS       "a1b2c3z26"        // Your password

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com" //Adafruit Server
#define AIO_SERVERPORT  1883                   

#define AIO_USERNAME  "Onkar2654"
#define AIO_KEY       "aio_nFze34emH9yFzx2UuBva3Tjxyt0T"

//WIFI CLIENT
WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Subscribe Light1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/tap"); // Feed name should be the same everywhere
Adafruit_MQTT_Publish timeOn = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/timeon");
Adafruit_MQTT_Publish waterUsed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/waterused");

unsigned long tapStartTime = 0;
bool tapOn = false;

void MQTT_connect();

void setup() {
  Serial.begin(115200);

  pinMode(tapPin, OUTPUT);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());
 
  mqtt.subscribe(&Light1);
}

void loop() {
  MQTT_connect();
  
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(20000))) {
    if (subscription == &Light1) {
      Serial.print(F("Got: "));
      Serial.println((char *)Light1.lastread);
      int Light1_State = atoi((char *)Light1.lastread);
      digitalWrite(tapPin, Light1_State);
      
      if (Light1_State == HIGH && !tapOn) {
        tapStartTime = millis();
        tapOn = true;
      } else if (Light1_State == LOW && tapOn) {
        unsigned long tapEndTime = millis();
        unsigned long tapDuration = (tapEndTime - tapStartTime)/1000;
        char durationStr[10];
        sprintf(durationStr, "%lu", tapDuration);
        timeOn.publish(durationStr);
        int duration = atoi(durationStr); // Convert durationStr to integer
        float w = duration * 0.15; // Multiply duration by 0.15 to calculate water usage
        waterUsed.publish(w);
   
tapOn = false;
      }
    }
  }
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 5 seconds...");
    mqtt.disconnect();
    delay(5000); 
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}
