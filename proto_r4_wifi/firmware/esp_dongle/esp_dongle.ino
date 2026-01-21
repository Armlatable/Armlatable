/**
 * @file esp_dongle.ino
 * @brief PC側 ESP32 ドングル用ファームウェア (Serial-UDP Bridge)
 * @details PC とは USB Serial (115200bps) で通信し、ロボットとは WiFi UDP で通信する。
 *          APモードで起動し、ロボットからの接続を待つ。
 **/

#include <WiFi.h>
#include <WiFiUdp.h>

/**
 * @name WiFi設定 (AP)
 **/
const char* SSID = "proto_r4_net";
const char* PASS = "12345678";
const int LOCAL_PORT = 8080;  ///< 受信ポート
const int REMOTE_PORT = 8080; ///< 送信先ポート

WiFiUDP udp;
IPAddress remoteIP; // ロボットのIP（パケット受信時に学習）
bool hasRemote = false;

void setup() {
  Serial.begin(115200);

  // APモード設定
  WiFi.mode(WIFI_AP);
  WiFi.softAP(SSID, PASS);

  udp.begin(LOCAL_PORT);

  // 起動メッセージ (PCへ)
  Serial.println("ESP32 Dongle Ready");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  handleSerial();
  handleUDP();
}

/**
 * @brief Serial -> UDP
 * @details PCから受信したデータをUDPでロボットへ送信
 **/
void handleSerial() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
      if (hasRemote) {
        udp.beginPacket(remoteIP, REMOTE_PORT);
        udp.print(line);
        udp.print('\n'); // 終端文字
        udp.endPacket();
      } else {
        // ロボット未接続時はブロードキャストしてみる（オプション）
        // 現状は接続待ちとする
      }
    }
  }
}

/**
 * @brief UDP -> Serial
 * @details ロボットから受信したデータをSerialでPCへ送信
 **/
void handleUDP() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    // 送信元をロボットとして登録
    remoteIP = udp.remoteIP();
    hasRemote = true;

    // データを読み出してSerialへ転送
    while (udp.available()) {
      char c = udp.read();
      Serial.write(c);
    }
    // 終端文字がない場合に備えて改行を送るか、そのまま送るか。
    // parsePacket単位で来るので、最後に改行を入れるのが安全。
    // ただしデータ内に改行が含まれている前提ならそのまま。
    // 安全のため、受信データ末尾が改行でなければ追加する処理はPC側に任せる。
  }
}
