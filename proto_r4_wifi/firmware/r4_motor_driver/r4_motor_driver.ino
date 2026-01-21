/**
 * @file r4_motor_driver.ino
 * @brief Arduino R4 WiFi モーター制御ファームウェア (UDP版)
 * @details ESP32 ドングル (AP) に接続し、UDP でコマンドを送受信する。CONFIG コマンドにより動的に設定可能。
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
const uint8_t MAX_DXL_COUNT = 8;
uint8_t dxl_ids[MAX_DXL_COUNT];
uint8_t dxl_count = 0; // 初期状態では0 (CONFIG受信待ち)
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
int dxl_target[MAX_DXL_COUNT] = {0};
int dxl_mode[MAX_DXL_COUNT] = {4};

// プロトタイプ
void setupWiFi();
void setupMotorPins();
void setupDynamixel();
void handleUDP();
void parseCommand(String& cmd);
void setMotorPWM(uint8_t motor, int pwm);
void sendStatus();
void reconfigureDynamixel(String& ids_str);

/**
 * @brief 初期化
 **/
void setup() {
  Serial.begin(115200);

  setupMotorPins();

  // DXL初期化 (ポート設定のみ、IDスキャンはCONFIG受信後)
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  setupWiFi();
}

/**
 * @brief メインループ
 **/
void loop() {
  handleUDP();

  // DYNAMIXEL 制御 (設定されている場合のみ)
  for (uint8_t i = 0; i < dxl_count; i++) {
    if (dxl_mode[i] == 4) {
      dxl.setGoalPosition(dxl_ids[i], dxl_target[i]);
    } else if (dxl_mode[i] == 1) {
      dxl.setGoalVelocity(dxl_ids[i], dxl_target[i]);
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
 * @brief DXL 再設定 (CONFIG受信時)
 **/
void reconfigureDynamixel(String& ids_str) {
  // 文字列 "1,2,3" を解析
  dxl_count = 0;
  int dxl_idx = 0;
  int start = 0;
  int comma = ids_str.indexOf(',', start);

  while (comma != -1 && dxl_idx < MAX_DXL_COUNT) {
    dxl_ids[dxl_idx++] = ids_str.substring(start, comma).toInt();
    start = comma + 1;
    comma = ids_str.indexOf(',', start);
  }
  if (start < ids_str.length() && dxl_idx < MAX_DXL_COUNT) {
    dxl_ids[dxl_idx++] = ids_str.substring(start).toInt();
  }
  dxl_count = dxl_idx;

  // 初期設定
  for (uint8_t i = 0; i < dxl_count; i++) {
    dxl.ping(dxl_ids[i]);
    dxl.torqueOff(dxl_ids[i]);
    dxl.setOperatingMode(dxl_ids[i], OP_EXTENDED_POSITION);
    dxl.torqueOn(dxl_ids[i]);
    // 初期モードとターゲットをリセット
    dxl_mode[i] = 4;
    dxl_target[i] = dxl.getPresentPosition(dxl_ids[i]);
  }
  Serial.print("DXL Configured: ");
  Serial.println(dxl_count);
}

/**
 * @brief UDP受信処理
 **/
void handleUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
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
 * @brief コマンドパース
 * @details CONFIG:ids=1,2,3... を追加
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

      for (uint8_t i = 0; i < dxl_count; i++) {
        if (dxl_ids[i] == id) {
          dxl_mode[i] = mode;
          dxl_target[i] = target;
          break;
        }
      }
    }
  } else if (cmd.startsWith("CONFIG:")) {
    // CONFIG:ids=1,2,3
    String params = cmd.substring(7);
    if (params.startsWith("ids=")) {
        String ids = params.substring(4);
        reconfigureDynamixel(ids);
    }
  } else if (cmd == "STOP") {
    dc_pwm[0] = 0;
    dc_pwm[1] = 0;
    for (uint8_t i = 0; i < dxl_count; i++) {
      dxl.torqueOff(dxl_ids[i]);
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

  for (uint8_t i = 0; i < dxl_count; i++) {
    int pos = dxl.getPresentPosition(dxl_ids[i]);
    status += "," + String(dxl_ids[i]) + ":" + String(pos);
  }

  // ドングルへ送信
  udp.beginPacket(DONGLE_IP, DONGLE_PORT);
  udp.print(status);
  udp.print("\n");
  udp.endPacket();
}
