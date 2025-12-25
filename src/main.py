import yaml
import time
from dataclasses import dataclass
from hardware.dxl_interface import DynamixelInterface
from hardware.pico_interface import PicoInterface

@dataclass
class SerialConfig:
    dxl_port: str
    pico_port: str

@dataclass
class DynamixelConfig:
    id: int
    baud_rate: int
    model: str

@dataclass
class ControlConfig:
    loop_rate_hz: int

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

# @brief 経過時間から目標値を計算する
# @param elapsed 経過時間
# @return (target_dc_pwm, dxl_mode, dxl_target)
# @note テスト用のためパラメータもハードコード
def calculate_targets(elapsed: float):
    period = 10.0 # 周期
    phase = elapsed % period

    if phase < 5.0:
        # Phase 1: PWM Sweep & Position Control
        target_dc_pwm = (phase / 5.0) * 510 - 255
        dxl_mode = 3 # Position Mode
        dxl_target = int((phase / 5.0) * 4095) # 4095 = 360deg
    else:
        # Phase 2: Pico Stop & Velocity Control
        target_dc_pwm = 0
        dxl_mode = 1 # Velocity Mode
        dxl_target = 100 # 100rpm

    return int(target_dc_pwm), dxl_mode, dxl_target

def main():
    # --- 設定読み込み ---
    try:
        config = AppConfig.load('src/config.yaml')
        print(f"Loaded config: {config}")
    except Exception as e:
        print(f"Failed to load config: {e}")
        return

    # --- ハードウェア初期化 ---
    print("Initializing Hardware...")
    try:
        dxl = DynamixelInterface(config.serial.dxl_port, baud_rate=config.dynamixel.baud_rate)
        pico = PicoInterface(config.serial.pico_port)
    except Exception as e:
        print(f"初期化失敗: {e}")
        return

    # Dynamixel設定
    DXL_ID = config.dynamixel.id
    dxl.enable_torque(DXL_ID, True)

    print("制御ループ開始...")
    try:
        start_time = time.time()
        while True:
            current_time = time.time()
            elapsed = current_time - start_time

            # --- ステータス読み取り ---
            pico.update()
            dc_current_pwm = pico.get_status()

            # --- 目標値計算 ---
            target_dc_pwm, dxl_mode, dxl_target = calculate_targets(elapsed)

            # --- 出力 ---
            # DXL制御
            dxl.set_operating_mode(DXL_ID, dxl_mode)
            if dxl_mode == 3: # Position
                dxl.set_position(DXL_ID, dxl_target)
            elif dxl_mode == 1: # Velocity
                dxl.set_velocity(DXL_ID, dxl_target)
            # pico出力
            pico.set_motor_pwm(target_dc_pwm)

            # Log
            print(f"Time: {elapsed:.2f} | DC: PWM={target_dc_pwm:.0f} | DXL Mode={'Pos' if dxl_mode == 3 else 'Vel'}")

            time.sleep(1.0 / config.control.loop_rate_hz)
    except KeyboardInterrupt:
        print("\n停止中...")
        pico.set_motor_pwm(0)
        dxl.enable_torque(DXL_ID, False)

    finally:
        dxl.close()
        pico.close()
        print("ハードウェア接続を終了しました。")
if __name__ == '__main__':
    main()
