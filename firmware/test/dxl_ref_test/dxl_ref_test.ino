/**
 * @file dxl_ref_test.ino
 * @brief User Suggested Test Code (Ported to Dynamixel2Arduino for R4 WiFi)
 *
 * Implements the user's logic (OP_POSITION, UNIT_DEGREE) using
 * Dynamixel2Arduino library directly to avoid compilation issues with DynamixelShield wrapper.
 *
 * Pinout: Serial1 (D0/D1), DIR=2
 */

#include <Dynamixel2Arduino.h>

// R4 WiFi specific
#define DXL_SERIAL   Serial1
#define DEBUG_SERIAL Serial
const int DXL_DIR_PIN = 2; // DYNAMIXEL Shield DIR PIN

const uint8_t DXL_ID1 = 1;
const uint8_t DXL_ID2 = 2;
const uint8_t DXL_ID3 = 3;
const float DXL_PROTOCOL_VERSION = 2.0;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

// This namespace is required to use Control table item names
using namespace ControlTableItem;

void setup() {
  DEBUG_SERIAL.begin(115200);
  delay(1000);
  DEBUG_SERIAL.println("=== REFERENCE TEST START (D2A) ===");

  dxl.begin(57600);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  // Ping
  DEBUG_SERIAL.print("Ping ID 1: "); DEBUG_SERIAL.println(dxl.ping(DXL_ID1) ? "OK" : "MISS");
  DEBUG_SERIAL.print("Ping ID 2: "); DEBUG_SERIAL.println(dxl.ping(DXL_ID2) ? "OK" : "MISS");
  DEBUG_SERIAL.print("Ping ID 3: "); DEBUG_SERIAL.println(dxl.ping(DXL_ID3) ? "OK" : "MISS");

  // Torque OFF, Mode Set, Torque ON
  dxl.torqueOff(DXL_ID1);
  dxl.torqueOff(DXL_ID2);
  dxl.torqueOff(DXL_ID3);

  dxl.setOperatingMode(DXL_ID1, OP_POSITION);
  dxl.setOperatingMode(DXL_ID2, OP_POSITION);
  dxl.setOperatingMode(DXL_ID3, OP_POSITION);

  dxl.torqueOn(DXL_ID1);
  dxl.torqueOn(DXL_ID2);
  dxl.torqueOn(DXL_ID3);
}

void loop() {
  DEBUG_SERIAL.println("Goal: 210 deg");
  dxl.setGoalPosition(DXL_ID1, 210.0, UNIT_DEGREE);
  dxl.setGoalPosition(DXL_ID2, 210.0, UNIT_DEGREE);
  dxl.setGoalPosition(DXL_ID3, 210.0, UNIT_DEGREE);
  delay(1000);

  // Print present position in degree value
  DEBUG_SERIAL.print("Pos(deg): ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_ID1, UNIT_DEGREE)); DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_ID2, UNIT_DEGREE)); DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.println(dxl.getPresentPosition(DXL_ID3, UNIT_DEGREE));
  delay(1000);

  DEBUG_SERIAL.println("Goal: 330 deg");
  dxl.setGoalPosition(DXL_ID1, 330.0, UNIT_DEGREE);
  dxl.setGoalPosition(DXL_ID2, 330.0, UNIT_DEGREE);
  dxl.setGoalPosition(DXL_ID3, 330.0, UNIT_DEGREE);
  delay(1000);

  // Print present position in degree value
  DEBUG_SERIAL.print("Pos(deg): ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_ID1, UNIT_DEGREE)); DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.print(dxl.getPresentPosition(DXL_ID2, UNIT_DEGREE)); DEBUG_SERIAL.print(", ");
  DEBUG_SERIAL.println(dxl.getPresentPosition(DXL_ID3, UNIT_DEGREE));
  delay(1000);
}
