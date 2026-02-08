"""
TestController: 時間ベースで目標値を計算するコントローラー
"""
from dataclasses import dataclass

@dataclass
class ControlCommand:
    """制御コマンド"""
    dc_pwm: int         # DCモーターPWM (-255 ~ 255)
    dxl_mode: int       # Dynamixelモード (1: Velocity, 3: Position)
    dxl_targets: dict[int, int] # IDごとの目標値


class TestController:
    """
    時間経過に基づいて目標値を計算するテスト用コントローラー
    """
    def __init__(self, dxl_ids: list[int]):
        self.dxl_ids = dxl_ids
        self.period = 10.0  # 周期 (秒)

    def update(self, elapsed: float) -> ControlCommand:
        """
        経過時間から目標値を計算する

        Args:
            elapsed: 経過時間 (秒)

        Returns:
            ControlCommand: 制御コマンド
        """
        phase = elapsed % self.period

        if phase < 5.0:
            # Phase 1: PWM Sweep & Position Control
            dc_pwm = int((phase / 5.0) * 510 - 255)
            dxl_mode = 3  # Position Mode
            dxl_target = int((phase / 5.0) * 4095)  # 4095 = 360deg
        else:
            # Phase 2: Pico Stop & Velocity Control
            dc_pwm = 0
            dxl_mode = 1  # Velocity Mode
            dxl_target = 100  # 100rpm

        targets = {id: dxl_target for id in self.dxl_ids}
        return ControlCommand(dc_pwm=dc_pwm, dxl_mode=dxl_mode, dxl_targets=targets)

    def should_continue(self) -> bool:
        """継続するかどうか (常にTrue)"""
        return True
