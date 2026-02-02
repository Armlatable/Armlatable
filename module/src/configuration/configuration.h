/**
 * @file configuration.h
 * @brief Robot/Dongle Common Configuration (Auto-generated from config.yaml)
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>

namespace locomo {
namespace config {

// WiFi Settings
constexpr const char* SSID = "LocomoDongle";
constexpr const char* PASSWORD = "locomorobot";
constexpr int UDP_PORT = 8888;
constexpr int WIFI_CHANNEL = 1;

// IP Settings
constexpr uint8_t BROADCAST_IP[] = {192, 168, 4, 255};

// Serial Settings
constexpr int SERIAL_BAUD_RATE = 115200;

// Robot Physical Parameters
constexpr float MAX_VELOCITY = 0.5f;       // [m/s]
constexpr float MAX_ANGULAR_RATE = 1.0f;   // [rad/s]
constexpr int CONNECTION_TIMEOUT_MS = 500;

} // namespace config
} // namespace locomo

#endif // CONFIGURATION_H
