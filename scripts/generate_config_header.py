import yaml
import os
import sys

def generate_header(config_path, output_path):
    with open(config_path, 'r') as f:
        config = yaml.safe_load(f)

    header_content = """/**
 * @file configuration.h
 * @brief Robot/Dongle Common Configuration (Auto-generated from config.yaml)
 */

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>

namespace locomo {
namespace config {

// WiFi Settings
"""

    wifi = config.get('wifi', {})
    header_content += f'constexpr const char* SSID = "{wifi.get("ssid", "LocomoDongle")}";\n'
    header_content += f'constexpr const char* PASSWORD = "{wifi.get("password", "locomorobot")}";\n'
    header_content += f'constexpr int UDP_PORT = {wifi.get("udp_port", 8888)};\n'
    header_content += f'constexpr int WIFI_CHANNEL = {wifi.get("channel", 1)};\n\n'

    ip = wifi.get('broadcast_ip', [192, 168, 4, 255])
    header_content += "// IP Settings\n"
    header_content += "constexpr uint8_t BROADCAST_IP[] = {" + f"{ip[0]}, {ip[1]}, {ip[2]}, {ip[3]}" + "};\n\n"

    serial = config.get('serial', {})
    header_content += "// Serial Settings\n"
    header_content += f'constexpr int SERIAL_BAUD_RATE = {serial.get("baud_rate", 115200)};\n\n'

    robot = config.get('robot', {})
    header_content += "// Robot Physical Parameters\n"
    header_content += f'constexpr float MAX_VELOCITY = {robot.get("max_velocity", 0.5)}f;       // [m/s]\n'
    header_content += f'constexpr float MAX_ANGULAR_RATE = {robot.get("max_angular_rate", 1.0)}f;   // [rad/s]\n'
    header_content += f'constexpr int CONNECTION_TIMEOUT_MS = {robot.get("timeout_ms", 500)};\n'

    header_content += """
} // namespace config
} // namespace locomo

#endif // CONFIGURATION_H
"""

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, 'w') as f:
        f.write(header_content)
    print(f"Generated {output_path} from {config_path}")

if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: python generate_config_header.py <config.yaml> <output_header.h>")
        sys.exit(1)

    generate_header(sys.argv[1], sys.argv[2])
