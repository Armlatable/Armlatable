/**
 * @file pico_motor_driver.ino
 * @brief Raspberry Pi Pico モータードライバーファームウェア (直値制御)
 * @details シリアル通信で目標PWM値を受信し、TB6612を用いてDCモーターを制御する。
 */

// --- ピン定義 ---
const uint8_t PIN_AIN1 = 4;
const uint8_t PIN_AIN2 = 5;
const uint8_t PIN_PWMA = 3;
const uint8_t PIN_STBY = 2;

// --- グローバル変数 ---
int target_pwm = 0; ///< 目標PWM値 (-255 ~ 255)

// --- プロトタイプ宣言 ---
/**
 * @brief モーターのPWM出力を設定する
 * @param pwm PWM値 (-255 ~ 255)
 */
void setMotorPWM(int pwm);

void setup() {
  Serial.begin(115200);

  // モーターピン
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);
  digitalWrite(PIN_STBY, HIGH); // ドライバ有効化

}

void loop() {
  // PWM直接制御
  setMotorPWM(target_pwm);

  // シリアルコマンド解析
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();

    // コマンド形式: "M:100" (PWM値を -255 から 255 で設定)
    if (input.startsWith("M:")) {
      target_pwm = input.substring(2).toInt();
    }
  }

  // ステータス報告 (PWM値をエコー)
  // 形式: "S:PWM"
  // 頻繁な送信を防ぐため、一定間隔で送信
  static unsigned long last_report = 0;
  if (millis() - last_report > 100) {
     Serial.print("S:");
     Serial.println(target_pwm);
     last_report = millis();
  }
}

/**
 * @brief モータードライバ (TB6612) への出力を行う
 * @param pwm PWM値 (-255 ~ 255) 正の値で正転、負の値で逆転
 */
void setMotorPWM(int pwm) {
  if (pwm > 255) pwm = 255;
  if (pwm < -255) pwm = -255;

  if (pwm > 0) {
    digitalWrite(PIN_AIN1, HIGH);
    digitalWrite(PIN_AIN2, LOW);
    analogWrite(PIN_PWMA, pwm);
  } else if (pwm < 0) {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, HIGH);
    analogWrite(PIN_PWMA, -pwm);
  } else {
    digitalWrite(PIN_AIN1, LOW);
    digitalWrite(PIN_AIN2, LOW);
    analogWrite(PIN_PWMA, 0);
  }
}
