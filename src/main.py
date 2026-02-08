"""
Armlatable メインコントロールプログラム

使用方法:
    python src/main.py [controller]

    controller:
        test     - 自動テスト (デフォルト)
        keyboard - キーボード操作
"""
import yaml
import time
import sys
import os
from dataclasses import dataclass
from hardware.dxl_interface import DynamixelInterface
from hardware.dc_motor_interface import DCMotorInterface

# --- 設定クラス ---
@dataclass
class SerialConfig:
    dxl_port: str
    pico_port: str
    motor_driver_type: str = "pico" # "pico" or "r4"

@dataclass
class DynamixelConfig:
    ids: list[int]
    baud_rate: int
    model: str
    current_limit_ma: int = 500  # トルク制限 (mA)

@dataclass
class ControlConfig:
    loop_rate_hz: int
    position_min: int = -20000
    position_max: int = 20000

@dataclass
class AppConfig:
    serial: SerialConfig
    dynamixel: DynamixelConfig
    control: ControlConfig

    @staticmethod
    def load(path: str) -> 'AppConfig':
        with open(path, 'r') as f:
            data = yaml.safe_load(f)
        return AppConfig(
            serial=SerialConfig(**data['serial']),
            dynamixel=DynamixelConfig(**data['dynamixel']),
            control=ControlConfig(**data['control'])
        )

# --- コントローラー読み込み ---
def get_controller(name: str, dxl_ids: list[int]):
    """
    指定された名前のコントローラーを取得する

    Args:
        name: コントローラー名 ('test' or 'keyboard')
        dxl_ids: Dynamixel IDのリスト

    Returns:
        Controller instance
    """
    if name == 'test':
        from api.test import TestController
        return TestController(dxl_ids)
    elif name == 'keyboard':
        from api.keyboard import KeyboardController
        return KeyboardController(dxl_ids)
    else:
        raise ValueError(f"Unknown controller: {name}")

# --- メイン ---
def main():
    # コントローラー選択
    controller_name = sys.argv[1] if len(sys.argv) > 1 else 'test'

    # --- 設定読み込み ---
    config_paths = ['config.yaml', 'src/config.yaml']
    config = None
    for path in config_paths:
        try:
            if os.path.exists(path):
                config = AppConfig.load(path)
                print(f"Loaded config from: {path}")
                break
        except Exception as e:
            print(f"Warning: Failed to load config from {path}: {e}")

    if config is None:
        print("Error: config.yaml not found in root or src/")
        return

    # --- コントローラー初期化 ---
    DXL_IDS = config.dynamixel.ids
    try:
        controller = get_controller(controller_name, DXL_IDS)
        print(f"Controller: {controller_name}")
    except Exception as e:
        print(f"Failed to load controller: {e}")
        return

    print(f"Initializing Hardware (Motor Driver: {config.serial.motor_driver_type})...")
    try:
        dxl = DynamixelInterface(config.serial.dxl_port, baud_rate=config.dynamixel.baud_rate)
        # 内部的には同じプロトコルを使用しているため、DCMotorInterfaceを共通で使用
        dc_motor = DCMotorInterface(config.serial.pico_port)
    except Exception as e:
        print(f"初期化失敗: {e}")
        return

    DXL_IDS = config.dynamixel.ids
    for dxl_id in DXL_IDS:
        dxl.enable_torque(dxl_id, True)
        dxl.set_current_limit(dxl_id, config.dynamixel.current_limit_ma)
    print(f"Torque enabled and current limit set to {config.dynamixel.current_limit_ma} mA for IDs: {DXL_IDS}")

    # キーボードモードの場合、現在位置に同期＆位置制限を設定
    if hasattr(controller, 'set_initial_position'):
        # すべてのIDの現在位置を取得して同期
        initial_positions = {}
        for dxl_id in DXL_IDS:
            current_pos = dxl.get_present_position(dxl_id)
            initial_positions[dxl_id] = current_pos if current_pos is not None else 0

        controller.set_initial_position(initial_positions)
        print(f"Synced to current positions: {initial_positions}")
    if hasattr(controller, 'set_position_limits'):
        controller.set_position_limits(config.control.position_min, config.control.position_max)
        print(f"Position limits: [{config.control.position_min}, {config.control.position_max}]")

    print("制御ループ開始...")
    try:
        start_time = time.time()
        last_mode = None

        while controller.should_continue():
            elapsed = time.time() - start_time

            # --- ステータス読み取り ---
            dc_motor.update()

            # --- 目標値計算 (コントローラーに委譲) ---
            cmd = controller.update(elapsed)

            # DXL制御
            for dxl_id, dxl_target in cmd.dxl_targets.items():
                # モード変更時のみ設定
                if cmd.dxl_mode != last_mode:
                    dxl.set_operating_mode(dxl_id, cmd.dxl_mode)

                if cmd.dxl_mode == 3 or cmd.dxl_mode == 4:  # Position / Extended Position
                    dxl.set_position(dxl_id, dxl_target)
                elif cmd.dxl_mode == 1:  # Velocity
                    dxl.set_velocity(dxl_id, dxl_target)

            if cmd.dxl_mode != last_mode:
                last_mode = cmd.dxl_mode

            # Enable/Disable 制御
            if cmd.enable is not None:
                print(f"\nSetting Motors: {'ENABLED' if cmd.enable else 'DISABLED'}")
                for dxl_id in DXL_IDS:
                    dxl.enable_torque(dxl_id, cmd.enable)
                dc_motor.set_enabled(cmd.enable)

            # DCモーター出力
            dc_motor.set_motor_pwm(cmd.dc_pwm)

            # Log
            mode_str = 'ExtPos' if cmd.dxl_mode == 4 else ('Pos' if cmd.dxl_mode == 3 else 'Vel')
            target_str = ", ".join([f"{id}:{val}" for id, val in cmd.dxl_targets.items()])
            driver_label = config.serial.motor_driver_type.upper()
            print(f"\rTime: {elapsed:.2f} | {driver_label}: {cmd.dc_pwm:4d} | DXLs({mode_str}): {target_str}", end='')
            sys.stdout.flush()

            time.sleep(1.0 / config.control.loop_rate_hz)

    except KeyboardInterrupt:
        print("\n停止中...")

    finally:
        dc_motor.set_motor_pwm(0)
        for dxl_id in DXL_IDS:
            dxl.enable_torque(dxl_id, False)
        dxl.close()
        dc_motor.close()
        if hasattr(controller, 'cleanup'):
            controller.cleanup()
        print("\nハードウェア接続を終了しました。")


if __name__ == '__main__':
    main()
