import serial
import time
import threading

class DCMotorInterface:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        # WSL/Linuxでの安定性のためにDTR/RTSを制御
        self.ser.dtr = True
        self.ser.rts = True

        self.lock = threading.Lock()
        time.sleep(2) # Arduino/Picoのリセット待ち

        # 初期バッファをクリア
        self.ser.reset_input_buffer()
        self.ser.reset_output_buffer()

        self.latest_pwm = 0

    def set_motor_pwm(self, pwm):
        """モーターPWM指令を送信する (-255 から 255)"""
        cmd = f"M:{int(pwm)}\n"
        with self.lock:
            self.ser.write(cmd.encode())
            self.ser.flush() # WSLでのバッファリング問題を回避するために即座に送信

    def set_enabled(self, enabled: bool):
        """モーターの有効/無効 (STBY) を切り替える"""
        val = 1 if enabled else 0
        cmd = f"E:{val}\n"
        with self.lock:
            self.ser.write(cmd.encode())
            self.ser.flush()

    def update(self):
        """Picoからステータスを読み取る"""
        if self.ser.in_waiting:
            try:
                line = self.ser.readline().decode().strip()
                if line.startswith("S:"):
                    # 形式: S:PWM
                    parts = line[2:].split(',')
                    if len(parts) >= 1:
                        self.latest_pwm = int(parts[0])
            except Exception as e:
                print(f"Error reading from Pico: {e}")

    def get_status(self):
        return self.latest_pwm

    def close(self):
        self.ser.close()
