"""
@file r4_interface.py
@brief PC側 ESP32 ドングルとのシリアル通信インターフェース
"""
import serial
import time
import threading


class R4Interface:
    """ESP32 ドングルとのシリアル通信クラス"""

    def __init__(self, port: str, baudrate: int = 115200):
        """
        Args:
            port: ドングルのシリアルポート (e.g. /dev/ttyUSB0)
            baudrate: 通信ボーレート (115200)
        """
        self.ser = serial.Serial(port, baudrate, timeout=0.1)
        self.lock = threading.Lock()
        time.sleep(2) # リセット待ち

        self.dc_pwm = [0, 0]
        self.dxl_positions = {}

    def set_dc_motor(self, pwm1: int, pwm2: int):
        """
        DCモーターPWM設定
        """
        cmd = f"DC:{int(pwm1)},{int(pwm2)}\n"
        self._send(cmd)
        self.dc_pwm = [pwm1, pwm2]

    def set_dynamixel(self, dxl_id: int, mode: int, target: int):
        """
        DYNAMIXEL目標値設定
        """
        cmd = f"DXL:{dxl_id},{mode},{target}\n"
        self._send(cmd)

    def stop_all(self):
        """全停止"""
        self._send("STOP\n")
        self.dc_pwm = [0, 0]

    def _send(self, cmd: str):
        """コマンド送信"""
        with self.lock:
            try:
                self.ser.write(cmd.encode())
            except serial.SerialException as e:
                print(f"Serial write error: {e}")

    def update(self):
        """ステータス受信"""
        if self.ser.in_waiting:
            try:
                line = self.ser.readline().decode().strip()
                self._parse_status(line)
            except Exception:
                pass

    def _parse_status(self, line: str):
        """ステータスパース"""
        if line.startswith("S:"):
            parts = line[2:].split(',')
            if len(parts) >= 2:
                try:
                    self.dc_pwm[0] = int(parts[0])
                    self.dc_pwm[1] = int(parts[1])
                    for part in parts[2:]:
                        if ':' in part:
                            dxl_id, pos = part.split(':')
                            self.dxl_positions[int(dxl_id)] = int(pos)
                except ValueError:
                    pass

    def get_status(self):
        """ステータス取得"""
        return {"dc_pwm": self.dc_pwm, "dxl_positions": self.dxl_positions}

    def close(self):
        """切断"""
        self.ser.close()
