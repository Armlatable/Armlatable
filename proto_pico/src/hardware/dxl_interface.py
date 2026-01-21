import os
from dynamixel_sdk import * # Dynamixel SDKライブラリを使用

class DynamixelInterface:
    def __init__(self, device_name, baud_rate=57600, protocol_version=2.0):
        self.device_name = device_name
        self.baud_rate = baud_rate
        self.protocol_version = protocol_version

        # Dynamixel コントロールテーブルアドレス (XM540-W150-R / X-Series 推奨値)
        # 使用するモデルに応じて変更
        self.ADDR_TORQUE_ENABLE          = 64
        self.ADDR_GOAL_VELOCITY          = 104
        self.ADDR_GOAL_POSITION          = 116
        self.ADDR_PRESENT_VELOCITY       = 128
        self.ADDR_PRESENT_POSITION       = 132
        self.ADDR_OPERATING_MODE         = 11
        self.ADDR_GOAL_CURRENT           = 102  # 電流制限 (トルク制限)

        self.portHandler = PortHandler(device_name)
        self.packetHandler = PacketHandler(protocol_version)

        if self.portHandler.openPort():
            print(f"Succeeded to open the port {device_name}")
        else:
            print(f"Failed to open the port {device_name}")
            return

        if self.portHandler.setBaudRate(baud_rate):
            print(f"Succeeded to change the baudrate {baud_rate}")
        else:
            print(f"Failed to change the baudrate {baud_rate}")
            return

    def enable_torque(self, dxl_id, enable=True):
        self.packetHandler.write1ByteTxRx(self.portHandler, dxl_id, self.ADDR_TORQUE_ENABLE, 1 if enable else 0)

    def set_operating_mode(self, dxl_id, mode):
        # 設定時はトルクを無効化する必要がある
        self.enable_torque(dxl_id, False)
        self.packetHandler.write1ByteTxRx(self.portHandler, dxl_id, self.ADDR_OPERATING_MODE, mode)
        self.enable_torque(dxl_id, True)

    def set_velocity(self, dxl_id, velocity):
        # この用途では速度制御モードを推奨
        # 速度の単位はモデルに依存
        self.packetHandler.write4ByteTxRx(self.portHandler, dxl_id, self.ADDR_GOAL_VELOCITY, int(velocity))

    def set_position(self, dxl_id, position):
         self.packetHandler.write4ByteTxRx(self.portHandler, dxl_id, self.ADDR_GOAL_POSITION, int(position))

    def set_current_limit(self, dxl_id, current_ma):
        """
        電流制限を設定する（トルク制限として機能）

        Args:
            dxl_id: Dynamixel ID
            current_ma: 電流制限値 (mA). XM430の場合、最大は約2300mA
        """
        self.packetHandler.write2ByteTxRx(self.portHandler, dxl_id, self.ADDR_GOAL_CURRENT, int(current_ma))

    def get_present_position(self, dxl_id):
        dxl_present_position, dxl_comm_result, dxl_error = self.packetHandler.read4ByteTxRx(self.portHandler, dxl_id, self.ADDR_PRESENT_POSITION)
        # SDKバージョンやPython環境によっては2の補数処理が必要
        # read4ByteTxRx は通常unsignedを返すため:
        if dxl_present_position > 0x7fffffff:
            dxl_present_position -= 4294967296
        return dxl_present_position

    def close(self):
        self.portHandler.closePort()
