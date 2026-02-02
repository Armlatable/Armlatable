
# システムアーキテクチャ

## 概要
本システムは、PC（Host）からESP32ドングルを経由して、Arduino Uno R4 WiFi搭載のロボットを制御するシステムです。

## データフロー
```mermaid
graph LR
    User[User (Keyboard/GUI)] -->|Input| Host[Host PC]
    Host -->|Serial (USB)| Dongle[ESP32 Dongle]
    Dongle -->|ESP-NOW / WiFi| Robot[Robot (R4 WiFi)]
    Robot -->|Sensors| Dongle
    Dongle -->|Serial| Host
    Host -->|Render| GUI[ImGui Viewer]
```

## ディレクトリ構成と責務

| ディレクトリ | 責務 | 言語/ツール |
| --- | --- | --- |
| `host/` | ユーザーインターフェース、高レベル制御アルゴリズム、データの可視化を担当します。 | C++17, CMake, ImGui |
| `firmware/dongle_esp/` | PCとロボット間の通信ブリッジとして機能します。シリアル通信と無線通信の変換を行います。 | Arduino (C++) |
| `firmware/robot_r4_wifi/` | ロボットのハードウェア制御（モーター、センサー）を担当します。Hostからの指令に従い、センサー情報を返します。 | Arduino (C++) |
| `module/` | HostとFirmwareで共有されるコード（通信プロトコル定義など）を管理します。 | C++ |
| `docs/` | プロジェクトのドキュメントを格納します。 | Markdown |

## 通信プロトコル
Host-Dongle間およびDongle-Robot間は、共通のパケット構造を使用して通信を行います。詳細は `module/src/protocol/protocol.h` を参照してください。