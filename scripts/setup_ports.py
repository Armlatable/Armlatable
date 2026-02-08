"""
@file setup_ports.py
@brief Armlatable用シリアルポート自動検出スクリプト
@details Dynamixel (U2D2) と モータードライバー (Pico/R4) のポートを検出し、config.yaml を更新します。
macOS および Linux/WSL をサポートしています。
"""

import serial.tools.list_ports
import yaml
import os
import sys

def detect_ports():
    """
    共通のパターンとハードウェアID（VID/PID）に基づいてシリアルポートを検出する。
    Returns: (dxl_port, motor_port)
    """
    ports = serial.tools.list_ports.comports()
    dxl_port = None
    motor_port = None

    # 一般的なパターンと VID/PID
    # U2D2 / FTDI: usbserial (Mac), ttyUSB (Linux/WSL), VID:0403
    # Pico / R4: usbmodem (Mac), ttyACM (Linux/WSL), VID:2e8a (Pico), VID:2341 (R4)

    for p in ports:
        desc = p.description.lower()
        hwid = p.hwid.lower()
        device = p.device

        # Dynamixel (U2D2 / FTDI) の検出
        if any(pat in device.lower() for pat in ["usbserial", "ttyusb"]) or "vid:0403" in hwid:
            if not dxl_port:
                dxl_port = device

        # モータードライバー (Pico / R4 WiFi) の検出
        if any(pat in device.lower() for pat in ["usbmodem", "ttyacm"]) or any(vid in hwid for vid in ["vid:2e8a", "vid:2341"]):
            if not motor_port:
                motor_port = device

    return dxl_port, motor_port

def main():
    config_path = "config.yaml"
    if not os.path.exists(config_path):
        # ファイルが存在しない場合のデフォルト構造
        config = {
            "serial": {
                "dxl_port": "",
                "pico_port": "",
                "motor_driver_type": "r4"
            },
            "dynamixel": {
                "ids": [1, 2, 3],
                "baud_rate": 57600,
                "model": "XM430-W210"
            },
            "control": {
                "loop_rate_hz": 50,
                "position_min": -20000,
                "position_max": 20000
            }
        }
    else:
        with open(config_path, "r") as f:
            config = yaml.safe_load(f)

    dxl, motor = detect_ports()

    updated = False
    if dxl:
        print(f"Dynamixelポートを検出: {dxl}")
        config["serial"]["dxl_port"] = dxl
        updated = True
    else:
        print("警告: Dynamixelポート (U2D2) を検出できませんでした。")

    if motor:
        print(f"モータードライバーポートを検出: {motor}")
        config["serial"]["pico_port"] = motor
        updated = True
    else:
        print("警告: モータードライバーポート (Pico/R4) を検出できませんでした。")

    if updated:
        with open(config_path, "w") as f:
            yaml.dump(config, f, default_flow_style=False, sort_keys=False)
        print(f"{config_path} を更新しました。")
    else:
        print("ポートが検出されませんでした。config.yaml は変更されていません。")

if __name__ == "__main__":
    main()
