"""
KeyboardController: キーボード入力で目標値を決定するコントローラー
"""
import sys
import tty
import termios
import select
from dataclasses import dataclass

@dataclass
class ControlCommand:
    """制御コマンド"""
    dc_pwm: int         # DCモーターPWM (-255 ~ 255)
    dxl_mode: int       # Dynamixelモード (1: Velocity, 4: Extended Position)
    dxl_targets: dict[int, int] # IDごとの目標値
    enable: bool | None = None # 有効/無効切り替え (Torque, STBY)


class KeyboardController:
    """
    キーボード入力に基づいて目標値を計算するコントローラー

    操作方法:
        w/s: DCモーター PWM +/- 50
        x:   DCモーター 停止
        a/d: Dynamixel +/- 500 (Position) / +/- 20 (Velocity)
        m:   モード切替 (Position ↔ Velocity)
        Space: 全停止
        q:   終了
    """
    def __init__(self, dxl_ids: list[int]):
        self.dxl_ids = dxl_ids
        self.selected_id = dxl_ids[0] if dxl_ids else 1
        self.dc_pwm = 0
        self.dxl_mode = 4  # Extended Position Control (multi-turn)
        self.dxl_target_pos = {id: 0 for id in dxl_ids}
        self.dxl_target_vel = {id: 0 for id in dxl_ids}
        self._running = True
        self._initialized = False
        self.pos_min = -20000  # Default limits
        self.pos_max = 20000

        # Terminal settings for non-blocking input
        self._old_settings = termios.tcgetattr(sys.stdin)
        tty.setcbreak(sys.stdin.fileno())

        self._print_help()

    def _print_help(self):
        print("\n=== Keyboard Control ===")
        print(" [0]       Select ALL Dynamixels")
        print(" [1, 2, 3] Select individual Dynamixel ID")
        print(" [w/s] DC Motor +/- 50 PWM")
        print(" [x]   DC Motor Stop")
        print(" [a/d] Selected DXL +/- 500 (Pos) / +/- 20 (Vel)")
        print(" [m]   Toggle Mode (Pos/Vel) for ALL")
        print(" [e/r] Enable / Disable Motors")
        print(" [Space] Stop All")
        print(" [q]   Quit")
        print("========================\n")

    def set_initial_position(self, pos_dict: dict[int, int]):
        """初期位置を設定（現在のDynamixel位置に同期）"""
        for dxl_id, pos in pos_dict.items():
            if dxl_id in self.dxl_target_pos:
                self.dxl_target_pos[dxl_id] = pos
        self._initialized = True

    def set_position_limits(self, pos_min: int, pos_max: int):
        """位置制限を設定"""
        self.pos_min = pos_min
        self.pos_max = pos_max

    def _get_key(self) -> str | None:
        """Non-blocking key read"""
        if select.select([sys.stdin], [], [], 0)[0]:
            return sys.stdin.read(1)
        return None

    def update(self, elapsed: float) -> ControlCommand:
        """
        キー入力を処理して目標値を更新する

        Args:
            elapsed: 経過時間 (未使用だがインターフェース統一のため)

        Returns:
            ControlCommand: 制御コマンド
        """
        key = self._get_key()

        if key:
            if key == 'q':
                self._running = False
            elif key == '0':
                self.selected_id = 0
                print("\nSelected: ALL Dynamixels")
            elif key in [str(i) for i in self.dxl_ids]:
                self.selected_id = int(key)
                print(f"\nSelected Dynamixel ID: {self.selected_id}")
            elif key == 'w':
                self.dc_pwm = min(255, self.dc_pwm + 50)
            elif key == 's':
                self.dc_pwm = max(-255, self.dc_pwm - 50)
            elif key == 'x':
                self.dc_pwm = 0
            elif key == 'a':
                ids_to_move = self.dxl_ids if self.selected_id == 0 else [self.selected_id]
                for dxl_id in ids_to_move:
                    if self.dxl_mode == 4:
                        self.dxl_target_pos[dxl_id] += 500
                    else:
                        self.dxl_target_vel[dxl_id] += 20
            elif key == 'd':
                ids_to_move = self.dxl_ids if self.selected_id == 0 else [self.selected_id]
                for dxl_id in ids_to_move:
                    if self.dxl_mode == 4:
                        self.dxl_target_pos[dxl_id] -= 500
                    else:
                        self.dxl_target_vel[dxl_id] -= 20
            elif key == 'm':
                if self.dxl_mode == 4:
                    self.dxl_mode = 1
                    for dxl_id in self.dxl_ids:
                        self.dxl_target_vel[dxl_id] = 0
                    print("\nMode: Velocity")
                else:
                    self.dxl_mode = 4
                    print("\nMode: Position (Extended)")
            elif key == 'e':
                return ControlCommand(dc_pwm=self.dc_pwm, dxl_mode=self.dxl_mode, dxl_targets=self.dxl_target_pos.copy() if self.dxl_mode == 4 else self.dxl_target_vel.copy(), enable=True)
            elif key == 'r':
                return ControlCommand(dc_pwm=self.dc_pwm, dxl_mode=self.dxl_mode, dxl_targets=self.dxl_target_pos.copy() if self.dxl_mode == 4 else self.dxl_target_vel.copy(), enable=False)
            elif key == ' ':
                self.dc_pwm = 0
                for dxl_id in self.dxl_ids:
                    self.dxl_target_vel[dxl_id] = 0

        targets = self.dxl_target_pos if self.dxl_mode == 4 else self.dxl_target_vel
        return ControlCommand(dc_pwm=self.dc_pwm, dxl_mode=self.dxl_mode, dxl_targets=targets.copy())

    def should_continue(self) -> bool:
        """継続するかどうか"""
        return self._running

    def cleanup(self):
        """ターミナル設定を復元"""
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, self._old_settings)

    def __del__(self):
        try:
            self.cleanup()
        except:
            pass
