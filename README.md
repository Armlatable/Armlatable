# Armlatable

アームテーブル制御システムのプロトタイプ集。

## プロトタイプ一覧

| ディレクトリ | マイコン | 通信 | ステータス | 説明 |
|-------------|---------|------|-----------|------|
| [proto_r4_wifi](./proto_r4_wifi/) | Arduino R4 WiFi | **WiFi (Dongle)** | **🟢 最新** | ESP32ドングル制御 |
| [proto_pico](./proto_pico/) | Raspberry Pi Pico | USB Serial | 旧Ver | 初期プロトタイプ |

> [!TIP]
> **新規開発は [proto_r4_wifi](./proto_r4_wifi/) をベースにしてください。**

## システム構成 (proto_r4_wifi)

```mermaid
graph LR
    PC["PC"] <-->|USB| Dongle["ESP32 Dongle"]
    Dongle <-->|WiFi UDP| R4["Arduino R4 WiFi"]
    R4 --> Motors["DYNAMIXEL & DC Motors"]
```

詳細は [proto_r4_wifi/docs/architecture.md](./proto_r4_wifi/docs/architecture.md) を参照。

## クイックスタート

```bash
# 最新版 (R4 WiFi) を使う場合
cd proto_r4_wifi
make help

# 旧版 (Pico) を使う場合
cd proto_pico
make help
```

## 共通コンポーネント

| コンポーネント | proto_r4_wifi | proto_pico |
|---------------|---------------|------------|
| PC 制御 | Python | Python |
| DYNAMIXEL | Shield (RS-485) | U2D2 (USB) |
| DC モーター | TB6612FNG | TB6612FNG |
| 通信 | **WiFi (Dedicated)** | USB Serial |
