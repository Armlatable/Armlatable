/**
 * @file r4_motor_driver.ino
 * @brief DC Motor Driver for Arduino R4 WiFi (replacing Pico)
 *
 * Receives serial commands: "M:PWM\n" (PWM: -255 to 255)
 * Drives two DC motors with the same PWM value.
 *
 * Pinout (User Specified):
 * DC A: 9, 10, 11
 * DC B: 5, 4, 3
 * STBY: 6
 */

// DC Motor Pins
const uint8_t PIN_AIN1 = 9;
const uint8_t PIN_AIN2 = 10;
const uint8_t PIN_PWMA = 11;
const uint8_t PIN_BIN1 = 5;
const uint8_t PIN_BIN2 = 4;
const uint8_t PIN_PWMB = 3;
const uint8_t PIN_STBY = 6;

int current_pwm = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_PWMA, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);
  pinMode(PIN_BIN2, OUTPUT);
  pinMode(PIN_PWMB, OUTPUT);
  pinMode(PIN_STBY, OUTPUT);

  digitalWrite(PIN_STBY, HIGH); // Enable Driver

  Serial.println("R4 Motor Driver Started");
}

void loop() {
  // Serial Command Parsing
  if (Serial.available() > 0) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("M:")) {
      int pwm = line.substring(2).toInt();
      driveMotors(pwm);
      current_pwm = pwm;
    } else if (line.startsWith("E:")) {
      int enable = line.substring(2).toInt();
      digitalWrite(PIN_STBY, enable ? HIGH : LOW);
    }
  }

  // Send Status (Match pico_interface expectation)
  static uint32_t last_status_time = 0;
  if (millis() - last_status_time > 100) {
    Serial.print("S:");
    Serial.println(current_pwm);
    last_status_time = millis();
  }
}

void driveMotors(int pwm) {
  // Motor A
  driveMotor(PIN_AIN1, PIN_AIN2, PIN_PWMA, pwm);
  // Motor B
  driveMotor(PIN_BIN1, PIN_BIN2, PIN_PWMB, pwm);
}

void driveMotor(uint8_t in1, uint8_t in2, uint8_t pwmPin, int pwm) {
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
