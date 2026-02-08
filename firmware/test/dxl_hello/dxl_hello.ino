/**
 * @file dxl_hello.ino
 * @brief Simple Dynamixel Test for R4 WiFi (Periodic Ping)
 */

#include <Dynamixel2Arduino.h>

// R4 WiFi specific: Hardware Serial1 is on D0/D1
#define DXL_SERIAL   Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN

const float DXL_PROTOCOL_VERSION = 2.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

// This namespace is required to use Control table item names
using namespace ControlTableItem;

void setup() {
  DEBUG_SERIAL.begin(115200);
  // Wait a bit, but don't block forever if USB is late
  delay(2000);

  DEBUG_SERIAL.println("=== DXL HELLO START ===");
  DEBUG_SERIAL.println("Config: Serial1 (D0/D1), DIR=2, Baud=57600/1M");

  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
}

void loop() {
  DEBUG_SERIAL.println("--- Loop Start ---");

  // Try 57600
  DEBUG_SERIAL.println("Trying 57600 bps...");
  dxl.begin(57600);
  delay(100);
  pingAll();

  // Try 1Mbps
  DEBUG_SERIAL.println("Trying 1 Mbps...");
  dxl.begin(1000000);
  delay(100);
  pingAll();

  delay(2000);
}

void pingAll() {
  bool found = false;
  // Scan ID 1-10
  for(int id=1; id<=10; id++) {
      if(dxl.ping(id)) {
        DEBUG_SERIAL.print("  [FOUND] ID: ");
        DEBUG_SERIAL.print(id);
        DEBUG_SERIAL.print(" Model: ");
        DEBUG_SERIAL.println(dxl.getModelNumber(id));
        found = true;
      }
  }
  if (!found) {
    DEBUG_SERIAL.println("  No Dynamixel found at this baudrate.");
  }
}
