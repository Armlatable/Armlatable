import serial
import time
import threading

class PicoInterface:
    def __init__(self, port, baudrate=115200):
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.lock = threading.Lock()
        time.sleep(2) # Picoのリセット待ち

        self.latest_pwm = 0

    def set_motor_pwm(self, pwm):
        """PicoへモーターPWM指令を送信する (-255 から 255)"""
        cmd = f"M:{int(pwm)}\n"
        with self.lock:
            self.ser.write(cmd.encode())

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
