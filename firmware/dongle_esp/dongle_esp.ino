
/**
 * @file dongle_esp.ino
 * @brief Dongle側ファームウェア (ESP32) - 通信ブリッジ
 */

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include "configuration/configuration.h"

using namespace locomo::config;

WiFiUDP udp;

// Broadcast IP
IPAddress broadcast_ip(BROADCAST_IP[0], BROADCAST_IP[1], BROADCAST_IP[2], BROADCAST_IP[3]);

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);

  // WiFi Access Point Mode
  WiFi.softAP(SSID, PASSWORD, WIFI_CHANNEL);

  udp.begin(UDP_PORT);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // 1. Host (Serial) -> Robot (UDP)
  if (Serial.available()) {
    size_t len = Serial.available();
    uint8_t buffer[64];
    if (len > sizeof(buffer)) len = sizeof(buffer);

    Serial.readBytes(buffer, len);

    // Broadcast to Robot
    udp.beginPacket(broadcast_ip, UDP_PORT);
    udp.write(buffer, len);
    udp.endPacket();

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
  }

  // 2. Robot (UDP) -> Host (Serial)
  int packet_size = udp.parsePacket();
  if (packet_size) {
    uint8_t buffer[64];
    int len = udp.read(buffer, sizeof(buffer));
    if (len > 0) {
      Serial.write(buffer, len);
    }
  }
}
