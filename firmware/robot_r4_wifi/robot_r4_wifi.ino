
/**
 * @file robot_r4_wifi.ino
 * @brief Robot側ファームウェア (UNO R4 WiFi)
 */

#include <Arduino.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include "protocol/protocol.h"
#include "configuration/configuration.h"

using namespace locomo::protocol;
using namespace locomo::config;

// Instances
WiFiUDP udp;
VelocityCommand current_cmd = {0.0f, 0.0f};

// バッファ
uint8_t rx_buffer[128];

void setup() {
  Serial.begin(SERIAL_BAUD_RATE);
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);

  // WiFi Station Mode
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true);
  }

  // Connect to AP
  Serial.print("Attempting to connect to SSID: ");
  Serial.println(SSID);
  WiFi.begin(SSID, PASSWORD);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  udp.begin(UDP_PORT);
}

void handlePacket(const Header& header, const uint8_t* payload) {
  if (header.cmd_id == (uint8_t)CmdID::SET_VELOCITY) {
    if (header.size == sizeof(VelocityCommand)) {
      VelocityCommand cmd;
      memcpy(&cmd, payload, sizeof(VelocityCommand));
      current_cmd = cmd;

      // Debug Output
      Serial.print("Cmd: vx="); Serial.print(cmd.vx);
      Serial.print(", wz="); Serial.println(cmd.wz);

      // Echo back status (Mock)
      RobotStatus status;
      status.voltage = 12.5f;
      status.current = 0.5f;
      status.yaw = 0.0f; // Dummy

      uint8_t tx_buffer[64];
      uint8_t tx_len = Packetizer::createPacket(
          CmdID::REPORT_STATUS, &status, sizeof(status), tx_buffer, sizeof(tx_buffer)
      );

      if (tx_len > 0) {
          udp.beginPacket(udp.remoteIP(), udp.remotePort());
          udp.write(tx_buffer, tx_len);
          udp.endPacket();
      }

      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
}

void loop() {
  int packet_size = udp.parsePacket();
  if (packet_size) {
    int len = udp.read(rx_buffer, sizeof(rx_buffer));
    if (len > 0) {
      if (Packetizer::validatePacket(rx_buffer, len)) {
        const Header* header = reinterpret_cast<const Header*>(rx_buffer);
        const uint8_t* payload = rx_buffer + sizeof(Header);
        handlePacket(*header, payload);
      } else {
        Serial.println("Invalid packet received");
      }
    }
  }

  // モーター制御ループ等はここに入れる
}
