"""
@file keyboard.py
@brief キーボード入力コントローラー
"""
import sys
import tty
import termios
import select
from dataclasses import dataclass


@dataclass
class ControlCommand:
    """制御コマンド"""
    dc_pwm: int
    dxl_mode: int
    dxl_target: int


class KeyboardController:
    """
    キーボード入力で目標値を決定するコントローラー

    操作:
        w/s: DC PWM +/- 50
        x:   DC停止
        a/d: DXL +/- 500(Pos) / +/- 20(Vel)
        m:   モード切替
        Space: 全停止
        q:   終了
    """

    def __init__(self):
        self.dc_pwm = 0
        self.dxl_mode = 4
        self.dxl_target_pos = 0
        self.dxl_target_vel = 0
        self._running = True
        self.pos_min = -20000
        self.pos_max = 20000
        self._old_settings = termios.tcgetattr(sys.stdin)
        tty.setcbreak(sys.stdin.fileno())
        self._print_help()

    def _print_help(self):
        print("\n[w/s] DC +/-50 | [x] DC stop")
        print("[a/d] DXL +/-  | [m] mode")
        print("[Space] stop   | [q] quit\n")

    def set_initial_position(self, pos: int):
        """初期位置設定"""
        self.dxl_target_pos = pos

    def set_position_limits(self, pos_min: int, pos_max: int):
        """位置制限設定"""
        self.pos_min = pos_min
        self.pos_max = pos_max

    def _get_key(self) -> str | None:
        if select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.read(1)
        return None

    def update(self, elapsed: float) -> ControlCommand:
        """更新"""
        key = self._get_key()

        if key:
            if key == 'q':
                self._running = False
            elif key == 'w':
                self.dc_pwm = min(255, self.dc_pwm + 50)
            elif key == 's':
                self.dc_pwm = max(-255, self.dc_pwm - 50)
            elif key == 'x':
                self.dc_pwm = 0
            elif key == 'a':
                if self.dxl_mode == 4:
                    self.dxl_target_pos += 500
                else:
                    self.dxl_target_vel += 20
            elif key == 'd':
                if self.dxl_mode == 4:
                    self.dxl_target_pos -= 500
                else:
                    self.dxl_target_vel -= 20
            elif key == 'm':
                if self.dxl_mode == 4:
                    self.dxl_mode = 1
                    self.dxl_target_vel = 0
                    print("\nMode: Velocity")
                else:
                    self.dxl_mode = 4
                    print("\nMode: Position")
            elif key == ' ':
                self.dc_pwm = 0
                self.dxl_target_vel = 0

        target = self.dxl_target_pos if self.dxl_mode == 4 else self.dxl_target_vel
        return ControlCommand(dc_pwm=self.dc_pwm, dxl_mode=self.dxl_mode, dxl_target=target)

    def should_continue(self) -> bool:
        return self._running

    def cleanup(self):
        """ターミナル復元"""
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, self._old_settings)

    def __del__(self):
        try:
            self.cleanup()
        except:
            pass
