
/**
 * @file wifi_comm_test.ino
 * @brief 通信確認用テストコード (LED Control based on Packet)
 */

#include <Arduino.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "protocol/protocol.h"
#include "configuration/configuration.h"

using namespace locomo::protocol;
using namespace locomo::config;

WiFiUDP udp;
uint8_t rx_buffer[256];

// Blink Control
unsigned long last_blink_time = 0;
int blink_interval = 1000; // ms (Default: Slow)
int led_state = LOW;

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(1000); // reduced delay
  Serial.println("--- WiFi Communication Test (Blink) ---");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  Serial.print("Connecting to SSID: ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  int retry_count = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    retry_count++;
    if (retry_count > 20) {
        Serial.println("\nConnection Failed! Check Dongle.");
        break;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nConnected to WiFi");
      udp.begin(UDP_PORT);
      Serial.print("Listening on UDP port ");
      Serial.println(UDP_PORT);
  }

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
     // Reconnect logic omitted for simple test stability
     blink_interval = 2000;
  }

  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(rx_buffer, sizeof(rx_buffer));
    if (len > 0) {
        if (Packetizer::validatePacket(rx_buffer, len)) {
            Header* header = reinterpret_cast<Header*>(rx_buffer);

            if (header->cmd_id == (uint8_t)CmdID::SET_VELOCITY &&
                header->size == sizeof(VelocityCommand)) {

                VelocityCommand* cmd = reinterpret_cast<VelocityCommand*>(rx_buffer + sizeof(Header));

                Serial.print("Received Velocity: vx=");
                Serial.print(cmd->vx);
                Serial.print(", wz=");
                Serial.println(cmd->wz);

                // Change Blink Interval
                if (abs(cmd->vx) > 0.01) {
                    blink_interval = 100; // Fast blink
                } else if (abs(cmd->wz) > 0.01) {
                    blink_interval = 300; // Medium blink
                } else {
                    blink_interval = 1000; // Slow blink
                }
            }
        }
    }
  }

  if (millis() - last_blink_time >= blink_interval) {
    last_blink_time = millis();
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state);
  }
}
