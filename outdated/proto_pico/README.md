# proto_pico: Raspberry Pi Pico 版 (旧Ver)

DCモーターとDynamixelを制御するプロトタイプ。Raspberry Pi Pico をリアルタイムモーターコントローラとして使用。

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

ターミナルで本ディレクトリ (`proto_pico`) に移動し、以下のコマンドを実行する。

### ライブラリのインストール
Pythonの依存ライブラリをインストールする（仮想環境 `venv` が作成される）。
```bash
make install
```

### ファームウェアの書き込み (初回のみ)
Raspberry Pi Pico にファームウェアを書き込む。
Arduino IDEを使用するか、CLIがセットアップされている場合は以下を実行する。
```bash
make flash
# または Arduino IDE で firmware/pico_motor_driver/pico_motor_driver.ino を書き込む
```

## 3. 設定確認

`src/config.yaml` を開き、接続されているシリアルポートが正しいか確認・修正する。

```yaml
serial:
  dxl_port: "/dev/tty.usbserial-XXXXXX" # U2D2のポート
  pico_port: "/dev/tty.usbmodemXXXXXX"  # Picoのポート
```

ポート名の確認方法 (Mac):
```bash
ls /dev/tty.usb* /dev/tty.usbmodem*
```

## 4. 実行

### 4.1 デモ実行 (自動制御)

以下のコマンドでテスト用の自動制御プログラムを開始する。

```bash
make test
```

#### 動作内容
- **0〜5秒**:
    - DCモーター: PWM制御で徐々に加速・減速 (-255 〜 255)
    - Dynamixel: 位置制御 (Position Mode) で往復運動
- **5〜10秒**:
    - DCモーター: 停止
    - Dynamixel: 速度制御 (Velocity Mode) で一定回転 (100 RPM)

### 4.2 キーボード操作

以下のコマンドでキーボード操作モードを開始する。

```bash
make keyboard
```

#### 操作方法
| キー | 動作 |
|------|------|
| `w` / `s` | DCモーター PWM +/- 50 |
| `x` | DCモーター 停止 |
| `a` / `d` | Dynamixel Position/Velocity +/- 100 |
| `m` | モード切替 (Position ↔ Velocity) |
| `Space` | 全停止 |
| `q` | 終了 |

## 5. トラブルシューティング

- **Error: Resource busy**: Arduino IDEのシリアルモニタ等を閉じること。
- **Permission denied**: USBデバイスのアクセス権限を確認すること。
