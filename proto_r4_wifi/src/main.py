"""
@file main.py
@brief proto_r4_wifi メインコントロールプログラム (ドングル方式)

使用方法:
    python main.py [controller]

controller:
    test     - 自動テスト (デフォルト)
    keyboard - キーボード操作
"""
import yaml
import time
import sys
import os
from dataclasses import dataclass
from typing import List
from hardware.r4_interface import R4Interface


@dataclass
class SerialConfig:
    """シリアル接続設定"""
    port: str
    baud_rate: int


@dataclass
class DynamixelConfig:
    """DYNAMIXEL設定"""
    ids: List[int]
    baud_rate: int
    current_limit_ma: int = 500


@dataclass
class DCMotorConfig:
    """DCモーター設定"""
    count: int = 2


@dataclass
class ControlConfig:
    """制御パラメータ"""
    loop_rate_hz: int = 50
    position_min: int = -20000
    position_max: int = 20000


@dataclass
class AppConfig:
    """アプリケーション設定"""
    serial: SerialConfig
    dynamixel: DynamixelConfig
    dc_motor: DCMotorConfig
    control: ControlConfig

    @staticmethod
    def load(path: str) -> 'AppConfig':
        """設定ファイルを読み込む"""
        if not os.path.exists(path):
             raise FileNotFoundError(f"Config file not found: {path}")

        print(f"Loading config from: {path}")
        with open(path, 'r') as f:
            data = yaml.safe_load(f)

        return AppConfig(
            serial=SerialConfig(**data['serial']),
            dynamixel=DynamixelConfig(**data['dynamixel']),
            dc_motor=DCMotorConfig(**data['dc_motor']),
            control=ControlConfig(**data['control'])
        )


def get_controller(name: str):
    """
    コントローラーを取得する

    Args:
        name: コントローラー名 ('test' or 'keyboard')

    Returns:
        Controller instance
    """
    if name == 'test':
        from api.test import TestController
        return TestController()
    elif name == 'keyboard':
        from api.keyboard import KeyboardController
        return KeyboardController()
    else:
        raise ValueError(f"Unknown controller: {name}")


def main():
    """メインエントリーポイント"""
    controller_name = sys.argv[1] if len(sys.argv) > 1 else 'test'

    # 設定読み込み
    try:
        # 優先: 親ディレクトリ (proto_r4_wifi/config.yaml)
        # 次点: 同じディレクトリ (proto_r4_wifi/src/config.yaml) ※後方互換

        # main.py の一つ上の階層
        config_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'config.yaml'))

        if not os.path.exists(config_path):
             # 見つからなければ従来の場所を探す
             config_path = os.path.join(os.path.dirname(__file__), 'config.yaml')

        config = AppConfig.load(config_path)
        print(f"proto_r4_wifi (Dongle Mode)")
        print(f"Port: {config.serial.port} @ {config.serial.baud_rate}")
        print(f"DXL: {config.dynamixel.ids}")
    except Exception as e:
        print(f"Config error: {e}")
        return

    # コントローラー初期化
    try:
        controller = get_controller(controller_name)
        print(f"Controller: {controller_name}")
    except Exception as e:
        print(f"Controller error: {e}")
        return

    # ハードウェア接続
    print("Connecting to Dongle...")
    try:
        r4 = R4Interface(config.serial.port, config.serial.baud_rate)
        # 設定送信 (DYNAMIXEL ID等)
        r4.send_config(config.dynamixel.ids)
        print("Config sent to firmware.")

    except Exception as e:
        print(f"Connection error: {e}")
        return

    if hasattr(controller, 'set_position_limits'):
        controller.set_position_limits(config.control.position_min, config.control.position_max)

    if hasattr(controller, 'set_dxl_ids'):
        controller.set_dxl_ids(config.dynamixel.ids)

    print("Running... (Ctrl+C to stop)")

    try:
        start_time = time.time()

        while controller.should_continue():
            elapsed = time.time() - start_time
            r4.update()
            cmd = controller.update(elapsed)

            # DC モーター
            if hasattr(cmd, 'dc_pwm'):
                if isinstance(cmd.dc_pwm, list) and len(cmd.dc_pwm) >= 2:
                    r4.set_dc_motor(cmd.dc_pwm[0], cmd.dc_pwm[1])
                else:
                    r4.set_dc_motor(cmd.dc_pwm, 0)

            # DYNAMIXEL
            if hasattr(cmd, 'dxl_targets'):
                for dxl_id, target in cmd.dxl_targets.items():
                    mode = cmd.dxl_modes.get(dxl_id, 4) if hasattr(cmd, 'dxl_modes') else 4
                    r4.set_dynamixel(dxl_id, mode, target)
            elif hasattr(cmd, 'dxl_target'):
                mode = cmd.dxl_mode if hasattr(cmd, 'dxl_mode') else 4
                r4.set_dynamixel(config.dynamixel.ids[0], mode, cmd.dxl_target)

            # ログ出力
            status = r4.get_status()
            print(f"\rT:{elapsed:.1f} DC:{status['dc_pwm']} DXL:{status['dxl_positions']}", end='')
            sys.stdout.flush()

            time.sleep(1.0 / config.control.loop_rate_hz)

    except KeyboardInterrupt:
        print("\nStopping...")

    finally:
        r4.stop_all()
        r4.close()
        if hasattr(controller, 'cleanup'):
            controller.cleanup()
        print("Done")


if __name__ == '__main__':
    main()
