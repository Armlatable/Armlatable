/**
 * @file direct_drive_test.ino
 * @brief Continuous Motor Drive Test (Restored)
 *
 * Automatically cycles DC motors and Dynamixels.
 * Scans Baudrates 57600 and 1000000.
 *
 * Pinout (User Specified):
 * DC A: 9, 10, 11
 * DC B: 5, 4, 3
 * STBY: 6
 * DXL: Serial1, DIR=2
 */

#include <DynamixelShield.h>

// DC Motor Pins
const uint8_t PIN_AIN1 = 9;
const uint8_t PIN_AIN2 = 10;
const uint8_t PIN_PWMA = 11;
const uint8_t PIN_BIN1 = 5;
const uint8_t PIN_BIN2 = 4;
const uint8_t PIN_PWMB = 3;
const uint8_t PIN_STBY = 6;

// Dynamixel
// R4 WiFi: Serial1 is D0/D1
DynamixelShield dxl(Serial1, 2);

const float DXL_PROTOCOL_VERSION = 2.0;
const uint8_t DXL_IDS[] = {1, 2, 3};

using namespace ControlTableItem;

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== DIRECT DRIVE TEST (RESTORED) ===");

  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);
  pinMode(PIN_BIN2, OUTPUT);
  pinMode(PIN_PWMB, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  digitalWrite(PIN_STBY, HIGH); // Enable Driver

  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  dxl.begin(57600);
}

void loop() {
  static bool use_high_baud = false;
  long baud = use_high_baud ? 1000000 : 57600;

  Serial.print("--- Baud: "); Serial.print(baud); Serial.println(" ---");
  dxl.begin(baud);
  delay(100);

  // Check Connection
  bool any_found = false;
  Serial.print("DXL Status: ");
  for(uint8_t id : DXL_IDS) {
      if(dxl.ping(id)) {
          Serial.print("[ID:"); Serial.print(id); Serial.print(" OK] ");
          any_found = true;
          dxl.torqueOff(id);
          dxl.setOperatingMode(id, OP_EXTENDED_POSITION);
          dxl.torqueOn(id);
      } else {
          Serial.print("[ID:"); Serial.print(id); Serial.print(" MISS] ");
      }
  }
  Serial.println();

  if(any_found) {
    Serial.println("  -> DXL: Moving +500");
    moveDXL(2048 + 500);
    delay(500);
    Serial.println("  -> DXL: Center");
    moveDXL(2048);
    delay(500);
  }

  // Toggle Baud for next loop
  use_high_baud = !use_high_baud;

  // DC Motor Test
  Serial.println("  -> DC: Forward");
  driveDC(0, 100);
  driveDC(1, 100);
  delay(500);

  Serial.println("  -> DC: Stop");
  driveDC(0, 0);
  driveDC(1, 0);
  delay(500);

  Serial.println("  -> DC: Reverse");
  driveDC(0, -100);
  driveDC(1, -100);
  delay(500);

  Serial.println("  -> DC: Stop");
  driveDC(0, 0);
  driveDC(1, 0);
  delay(500);
}

void driveDC(int motor_id, int pwm) {
  uint8_t in1, in2, pwmPin;
  if(motor_id == 0) {
    in1 = PIN_AIN1; in2 = PIN_AIN2; pwmPin = PIN_PWMA;
  } else {
    in1 = PIN_BIN1; in2 = PIN_BIN2; pwmPin = PIN_PWMB;
  }

  if(pwm > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW); analogWrite(pwmPin, pwm);
  } else if(pwm < 0) {
    digitalWrite(in1, LOW); digitalWrite(in2, HIGH); analogWrite(pwmPin, -pwm);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW); analogWrite(pwmPin, 0);
  }
}

void moveDXL(int pos) {
  for(uint8_t id : DXL_IDS) {
    dxl.setGoalPosition(id, pos);
  }
}
