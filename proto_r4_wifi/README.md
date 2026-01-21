# proto_r4_wifi: Arduino R4 WiFi 版 (ドングル方式)

> [!NOTE]
> **最新バージョン**
> PC に ESP32 ドングルを接続し、専用の WiFi ネットワークで通信します。PC のインターネット接続を妨げません。

## システム構成

```mermaid
graph LR
    PC["PC"] <-->|USB| Dongle["ESP32 Dongle"]
    Dongle <-->|WiFi UDP| R4["Arduino R4 WiFi"]
    R4 --> Motors["DYNAMIXEL & DC Motors"]
```

詳細: [docs/architecture.md](./docs/architecture.md)

## 必要なハードウェア

1. **PC 側**:
   - ESP32 開発ボード (M5Atom, DevKitC など) x1
   - USB ケーブル

2. **ロボット側**:
   - Arduino R4 WiFi
   - DYNAMIXEL Shield + XM430-W350
   - TB6612FNG + DC Motors
   - 12V 電源

## セットアップ手順

### 1. ESP32 ドングルの準備
PC に接続する ESP32 にブリッジファームウェアを書き込みます。
```bash
cd firmware/esp_dongle
# ボードタイプに合わせてコンパイル・書き込み (例: m5stack-atom)
arduino-cli compile --fqbn esp32:esp32:m5stack-atom esp_dongle.ino
arduino-cli upload -p <PORT> ...
```

### 2. ロボットの準備
Arduino R4 WiFi に制御ファームウェアを書き込みます。
```bash
make flash
```

### 3. PC アプリの準備
```bash
make install
```

### 4. 実行
ESP32 ドングルを PC に接続し、Arduino R4 WiFi の電源を入れた状態で:
```bash
# config.yaml のポート設定を確認してから
make test
```
