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
from dataclasses import dataclass
from hardware.dxl_interface import DynamixelInterface
from hardware.pico_interface import PicoInterface

# --- 設定クラス ---
@dataclass
class SerialConfig:
    dxl_port: str
    pico_port: str

@dataclass
class DynamixelConfig:
    id: int
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
def get_controller(name: str):
    """
    指定された名前のコントローラーを取得する

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

# --- メイン ---
def main():
    # コントローラー選択
    controller_name = sys.argv[1] if len(sys.argv) > 1 else 'test'

    # --- 設定読み込み ---
    try:
        config = AppConfig.load('src/config.yaml')
        print(f"Loaded config: {config}")
    except Exception as e:
        print(f"Failed to load config: {e}")
        return

    # --- コントローラー初期化 ---
    try:
        controller = get_controller(controller_name)
        print(f"Controller: {controller_name}")
    except Exception as e:
        print(f"Failed to load controller: {e}")
        return

    # --- ハードウェア初期化 ---
    print("Initializing Hardware...")
    try:
        dxl = DynamixelInterface(config.serial.dxl_port, baud_rate=config.dynamixel.baud_rate)
        pico = PicoInterface(config.serial.pico_port)
    except Exception as e:
        print(f"初期化失敗: {e}")
        return

    DXL_ID = config.dynamixel.id
    dxl.enable_torque(DXL_ID, True)

    # 電流制限（トルク制限）を設定
    dxl.set_current_limit(DXL_ID, config.dynamixel.current_limit_ma)
    print(f"Current limit set to: {config.dynamixel.current_limit_ma} mA")

    # キーボードモードの場合、現在位置に同期＆位置制限を設定
    if hasattr(controller, 'set_initial_position'):
        current_pos = dxl.get_present_position(DXL_ID)
        controller.set_initial_position(current_pos if current_pos else 0)
        print(f"Synced to current position: {current_pos}")
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
            pico.update()

            # --- 目標値計算 (コントローラーに委譲) ---
            cmd = controller.update(elapsed)

            # --- 出力 ---
            # モード変更時のみ設定
            if cmd.dxl_mode != last_mode:
                dxl.set_operating_mode(DXL_ID, cmd.dxl_mode)
                last_mode = cmd.dxl_mode

            # DXL制御
            if cmd.dxl_mode == 3 or cmd.dxl_mode == 4:  # Position / Extended Position
                dxl.set_position(DXL_ID, cmd.dxl_target)
            elif cmd.dxl_mode == 1:  # Velocity
                dxl.set_velocity(DXL_ID, cmd.dxl_target)

            # Pico出力
            pico.set_motor_pwm(cmd.dc_pwm)

            # Log
            mode_str = 'ExtPos' if cmd.dxl_mode == 4 else ('Pos' if cmd.dxl_mode == 3 else 'Vel')
            print(f"\rTime: {elapsed:.2f} | DC: {cmd.dc_pwm:4d} | DXL({mode_str}): {cmd.dxl_target:5d}", end='')
            sys.stdout.flush()

            time.sleep(1.0 / config.control.loop_rate_hz)

    except KeyboardInterrupt:
        print("\n停止中...")

    finally:
        pico.set_motor_pwm(0)
        dxl.enable_torque(DXL_ID, False)
        dxl.close()
        pico.close()
        if hasattr(controller, 'cleanup'):
            controller.cleanup()
        print("\nハードウェア接続を終了しました。")


if __name__ == '__main__':
    main()
