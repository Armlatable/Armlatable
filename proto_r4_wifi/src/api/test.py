"""
@file test.py
@brief 時間ベースのテストコントローラー
"""
from dataclasses import dataclass


@dataclass
class ControlCommand:
    """制御コマンド"""
    dc_pwm: int
    dxl_mode: int
    dxl_target: int


class TestController:
    """時間経過に基づく自動テストコントローラー"""

    def __init__(self):
        self.period = 10.0

    def update(self, elapsed: float) -> ControlCommand:
        """
        目標値計算

        Args:
            elapsed: 経過時間 (秒)

        Returns:
            ControlCommand
        """
        phase = elapsed % self.period

        if phase < 5.0:
            dc_pwm = int((phase / 5.0) * 510 - 255)
            dxl_mode = 3
            dxl_target = int((phase / 5.0) * 4095)
        else:
            dc_pwm = 0
            dxl_mode = 1
            dxl_target = 100

        return ControlCommand(dc_pwm=dc_pwm, dxl_mode=dxl_mode, dxl_target=dxl_target)

    def should_continue(self) -> bool:
        return True
