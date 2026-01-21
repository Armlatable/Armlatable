/**
 * @file r4_motor_driver.ino
 * @brief Arduino R4 WiFi モーター制御ファームウェア (UDP版)
 * @details ESP32 ドングル (AP) に接続し、UDP でコマンドを送受信する。
 *
 * @par システム構成
 *   - PC <--USB--> ESP32 Dongle (AP)
 *   - ESP32 Dongle <--WiFi UDP--> Arduino R4 WiFi (STA)
 **/

#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <DynamixelShield.h>

/**
 * @name WiFi設定 (STA)
 * @note ドングルの設定に合わせる
 **/
const char* WIFI_SSID = "proto_r4_net";
const char* WIFI_PASSWORD = "12345678";
const uint16_t LOCAL_PORT = 8080;
const char* DONGLE_IP = "192.168.4.1"; // ESP32 APのデフォルトゲートウェイ
const uint16_t DONGLE_PORT = 8080;

WiFiUDP udp;

/**
 * @name DYNAMIXEL設定
 **/
DynamixelShield dxl;
const uint8_t DXL_IDS[] = {1, 2, 3};
const uint8_t DXL_COUNT = 3;
const float DXL_PROTOCOL_VERSION = 2.0;

/**
 * @name TB6612FNG ピン定義
 **/
const uint8_t PIN_AIN1 = 4;
const uint8_t PIN_AIN2 = 5;
const uint8_t PIN_PWMA = 3;
const uint8_t PIN_BIN1 = 7;
const uint8_t PIN_BIN2 = 8;
const uint8_t PIN_PWMB = 6;
const uint8_t PIN_STBY = 2;

/**
 * @name グローバル変数
 **/
int dc_pwm[2] = {0, 0};
int dxl_target[DXL_COUNT] = {0};
int dxl_mode[DXL_COUNT] = {4};

// プロトタイプ
void setupWiFi();
void setupMotorPins();
void setupDynamixel();
void handleUDP();
void parseCommand(String& cmd);
void setMotorPWM(uint8_t motor, int pwm);
void sendStatus();

/**
 * @brief 初期化
 **/
void setup() {
  Serial.begin(115200);
  // PCとのシリアル通信はデバッグ用

  setupMotorPins();
  setupDynamixel();
  setupWiFi();
}

/**
 * @brief メインループ
 **/
void loop() {
  handleUDP();

  // DYNAMIXEL 制御
  for (uint8_t i = 0; i < DXL_COUNT; i++) {
    if (dxl_mode[i] == 4) {
      dxl.setGoalPosition(DXL_IDS[i], dxl_target[i]);
    } else if (dxl_mode[i] == 1) {
      dxl.setGoalVelocity(DXL_IDS[i], dxl_target[i]);
    }
  }

  // DC モーター制御
  setMotorPWM(0, dc_pwm[0]);
  setMotorPWM(1, dc_pwm[1]);

  // ステータス送信
  static unsigned long last_status = 0;
  if (millis() - last_status > 100) {
    sendStatus();
    last_status = millis();
  }
}

/**
 * @brief WiFi接続 (STA)
 **/
void setupWiFi() {
  Serial.print("Connecting to AP: ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nConnected");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  udp.begin(LOCAL_PORT);
}

/**
 * @brief Motor Pin Init
 **/
void setupMotorPins() {
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);
  pinMode(PIN_BIN2, OUTPUT);
  pinMode(PIN_PWMB, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH);
}

/**
 * @brief DXL Init
 **/
void setupDynamixel() {
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);
  for (uint8_t i = 0; i < DXL_COUNT; i++) {
    dxl.ping(DXL_IDS[i]);
    dxl.torqueOff(DXL_IDS[i]);
    dxl.setOperatingMode(DXL_IDS[i], OP_EXTENDED_POSITION);
    dxl.torqueOn(DXL_IDS[i]);
  }
}

/**
 * @brief UDP受信処理
 **/
void handleUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // パケット全体を読み込む
    char packetBuffer[255];
    int len = udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    String line = String(packetBuffer);
    line.trim();
    if (line.length() > 0) {
      parseCommand(line);
    }
  }
}

/**
 * @brief コマンドパース (DC:.., DXL:..)
 **/
void parseCommand(String& cmd) {
  if (cmd.startsWith("DC:")) {
    String params = cmd.substring(3);
    int comma = params.indexOf(',');
    if (comma > 0) {
      dc_pwm[0] = params.substring(0, comma).toInt();
      dc_pwm[1] = params.substring(comma + 1).toInt();
    }
  } else if (cmd.startsWith("DXL:")) {
    String params = cmd.substring(4);
    int c1 = params.indexOf(',');
    int c2 = params.indexOf(',', c1 + 1);
    if (c1 > 0 && c2 > c1) {
      int id = params.substring(0, c1).toInt();
      int mode = params.substring(c1 + 1, c2).toInt();
      int target = params.substring(c2 + 1).toInt();

      for (uint8_t i = 0; i < DXL_COUNT; i++) {
        if (DXL_IDS[i] == id) {
          dxl_mode[i] = mode;
          dxl_target[i] = target;
          break;
        }
      }
    }
  } else if (cmd == "STOP") {
    dc_pwm[0] = 0;
    dc_pwm[1] = 0;
    for (uint8_t i = 0; i < DXL_COUNT; i++) {
      dxl.torqueOff(DXL_IDS[i]);
    }
  }
}

/**
 * @brief DC Motor PWM
 **/
void setMotorPWM(uint8_t motor, int pwm) {
  if (pwm > 255) pwm = 255;
  if (pwm < -255) pwm = -255;

  uint8_t in1, in2, pwmPin;
  if (motor == 0) {
    in1 = PIN_AIN1;
    in2 = PIN_AIN2;
    pwmPin = PIN_PWMA;
  } else {
    in1 = PIN_BIN1;
    in2 = PIN_BIN2;
    pwmPin = PIN_PWMB;
  }

  if (pwm > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, pwm);
  } else if (pwm < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    analogWrite(pwmPin, -pwm);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    analogWrite(pwmPin, 0);
  }
}

/**
 * @brief ステータス送信 (S:...) -> UDPでドングルへ
 **/
void sendStatus() {
  String status = "S:";
  status += String(dc_pwm[0]) + "," + String(dc_pwm[1]);

  for (uint8_t i = 0; i < DXL_COUNT; i++) {
    int pos = dxl.getPresentPosition(DXL_IDS[i]);
    status += "," + String(DXL_IDS[i]) + ":" + String(pos);
  }

  // ドングルへユニキャスト送信
  udp.beginPacket(DONGLE_IP, DONGLE_PORT);
  udp.print(status);
  udp.print("\n");
  udp.endPacket();
}
