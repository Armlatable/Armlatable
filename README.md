# Armlatable

## 構成

- **PC**: 高度な制御・コーディネーション (Python)
- **Raspberry Pi Pico**: リアルタイムDCモーター制御 (C++/Arduino)
- **PC-Pico通信**: シリアル通信による指令・ステータス交換

## 1. ハードウェア準備

1. **電源供給**
    - ブレッドボードの電源ラインに DC電源 (約 12V ~ 15V) を供給する。

2. **PC接続**
    - **Pico**: USBケーブルでPCに接続する。
    - **U2D2 (Dynamixel)**: USBケーブルでPCに接続する。

## 2. ソフトウェア準備

ターミナルで本ディレクトリ (`Armlatable`) に移動し、以下の手順でセットアップを行います。

### 2.1 依存ライブラリのインストール
Pythonの依存ライブラリをインストールします（仮想環境 `venv` が作成されます）。
```bash
make install
```

### 2.2 ポートの自動検出と設定
デバイス（U2D2, R4/Pico）をPCに接続した状態で、以下のコマンドを実行します。
```bash
make setup
```
これにより、シリアルポートが自動検出され、ルートディレクトリの `config.yaml` が更新されます。

### 2.3 ファームウェアの書き込み (必要な場合のみ)
Arduino R4 WiFi または Raspberry Pi Pico にファームウェアを書き込みます。
```bash
# R4 WiFi の場合
arduino-cli compile --fqbn arduino:renesas_uno:unor4wifi firmware/r4_motor_driver/r4_motor_driver.ino
arduino-cli upload -p <PORT> --fqbn arduino:renesas_uno:unor4wifi firmware/r4_motor_driver/r4_motor_driver.ino
```

## 3. 設定の確認

ルートディレクトリの `config.yaml` で、ハードウェア構成に合わせて設定を調整できます。

```yaml
serial:
  motor_driver_type: "r4" # "r4" または "pico"
```

## 4. 実行

### 4.1 キーボード操作モード (推奨)

```bash
make keyboard
```

#### 操作方法
| キー | 動作 |
|------|------|
| `0` | **すべてのDynamixelを選択** |
| `1`, `2`, `3` | 個別のDynamixel IDを選択 |
| `a` / `d` | 選択中のDynamixelを増減 |
| `w` / `s` | DCモーター PWM +/- 50 |
| `x` | DCモーター 停止 |
| `e` / `r` | モーターの有効化 / 無効化 (Torque/STBY) |
| `m` | Dynamixelモード切替 (Position ↔ Velocity) |
| `Space` | 全停止 |
| `q` | 終了 |

### 4.2 自動テストモード

```bash
make test
```

## 5. その他

- **トラブルシューティング**
    - **ポートが検出されない**: `make setup` で検出されない場合は、`config.yaml` を手動で編集してください。
    - **Resource busy**: 他のシリアルモニタを閉じてください。
