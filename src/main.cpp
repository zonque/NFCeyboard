#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SPI.h>
#include <MFRC522.h>

struct NfcTag {
  byte uid[10];
  byte uidSize;
  const char *topic;
  byte counter;
};

const int NfcTagTimeoutSeconds = 5;

#include "config.h"

static void mqtt_callback(char *topic, uint8_t *payload, unsigned int length) {
  // handle inbound MQTT messages
}

WiFiClient wifiClient;
PubSubClient client(MQTT_SERVER, 1883, mqtt_callback, wifiClient);

bool connect() {
  Serial.println();
  Serial.print("Connecting to WiFi ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.print("Connecting to MQTT server ");
  Serial.print(MQTT_SERVER);
  Serial.print(" ...");

  while (!client.connect("key-board")) {
    delay(500);
    Serial.print(".");
  }

  Serial.print("connected!\n");

  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.print("Firmware starting");
  SPI.begin();

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
    mfrc522[reader].PCD_Init(ssPins[reader], 0xff);

  for (NfcTag *tag = tags; tag->topic; tag++)
    tag->counter = NfcTagTimeoutSeconds;
}

void pollNfcReader(MFRC522 &reader) {
  if (reader.PICC_IsNewCardPresent() && reader.PICC_ReadCardSerial()) {
    // Serial.print(F("Card UID:"));
    // for (byte i = 0; i < reader.uid.size; i++) {
    //         if(reader.uid.uidByte[i] < 0x10)
    //                 Serial.print(F(" 0"));
    //         else
    //                 Serial.print(F(" "));
    //         Serial.print(reader.uid.uidByte[i], HEX);
    // }
    // Serial.println();

    for (NfcTag *tag = tags; tag->topic; tag++) {
      if (memcmp(tag->uid, reader.uid.uidByte, min(tag->uidSize, reader.uid.size)) == 0) {
        Serial.print("Card matches topic ");
        Serial.println(tag->topic);

        if (tag->counter == 0)
          client.publish(tag->topic, "home");

        tag->counter = NfcTagTimeoutSeconds;
      }
    }

    Serial.println();
  }
}

void loop() {
 if (!client.connected())
   connect();

  Serial.print("Polling for card ...\n");

  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++)
    pollNfcReader(mfrc522[reader]);

  // Now decrement the counter values of all tags by one.
  // If one of them drops to zero, report it as 'away'.

  for (NfcTag *tag = tags; tag->topic; tag++) {
    if (tag->counter > 0) {
      tag->counter--;

      if (tag->counter == 0)
        client.publish(tag->topic, "away");
    }
  }

  delay(1000);
}