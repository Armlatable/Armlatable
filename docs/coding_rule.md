
# コーディング規約

## 命名規則

| 対象 | 形式 | 例 | 備考 |
| --- | --- | --- | --- |
| ファイル名 | snake_case | `my_class.cpp`, `robot_control.h` | 全て小文字 |
| ディレクトリ名 | snake_case | `src/algorithm`, `firmware/robot` | 全て小文字 |
| クラス名 | PascalCase | `RobotController`, `PacketHandler` | |
| 関数名 | camelCase | `initRobot()`, `sendPacket()` | |
| 変数名 | snake_case | `motor_speed`, `is_connected` | |
| 定数名 | UPPER_SNAKE_CASE | `MAX_VELOCITY`, `DEFAULT_PORT` | |
| メンバ変数 | snake_case + _ | `velocity_`, `target_position_` | 末尾にアンダースコア |

## コメント規約
ドキュメントコメントには **Doxygen形式** を使用し、**日本語** で記述してください。

### ファイルヘッダ
全てのソースファイルの先頭に記述してください。
```cpp
/**
 * @file protocol.h
 * @brief 通信プロトコル定義
 */
```

### クラス・関数・変数
特に `host/include/algorithm` や `module/src/protocol` などのロジック層・共通定義には、数式の意図やデータの意味（単位など）を丁寧に記述してください。

```cpp
/**
 * @brief ロボットの目標速度を設定する
 * @param vx 前進速度 [m/s]
 * @param wz 旋回角速度 [rad/s]
 */
void setVelocity(float vx, float wz);

/**
 * @brief 現在のバッテリー電圧 [V]
 */
float battery_voltage_;
```
