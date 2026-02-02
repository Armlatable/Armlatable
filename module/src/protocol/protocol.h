
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <cstdint>
#include <cstring>

namespace locomo {
namespace protocol {

// マジックナンバー
constexpr uint8_t MAGIC_BYTE = 0xAA;

/**
 * @brief コマンドID定義
 */
enum class CmdID : uint8_t {
    NONE = 0x00,
    SET_VELOCITY = 0x01, ///< 速度指令
    GET_STATUS = 0x02,   ///< ステータス取得要求
    REPORT_STATUS = 0x03 ///< ステータス報告 (Robot -> Host)
};

/**
 * @brief パケットヘッダ構造体
 * @note __attribute__((packed)) は環境依存の可能性があるため、
 *       今回は単純な構造体とし、シリアライズ時にバイト列に整形する方式をとる。
 */
struct Header {
    uint8_t magic;   ///< マジックナンバー (0xAA)
    uint8_t cmd_id;  ///< コマンドID
    uint8_t size;    ///< ペイロードサイズ
};

/**
 * @brief 速度指令データ
 */
struct VelocityCommand {
    float vx; ///< 前進速度 [m/s]
    float wz; ///< 旋回角速度 [rad/s]
};

/**
 * @brief ロボットステータスデータ
 */
struct RobotStatus {
    float voltage; ///< バッテリー電圧 [V]
    float current; ///< 消費電流 [A]
    float yaw;     ///< 現在のヨー角 [rad]
};

/**
 * @brief パケット処理クラス
 */
class Packetizer {
public:
    /**
     * @brief パケットを作成する（シリアライズ）
     * @param cmd_id コマンドID
     * @param data データへのポインタ
     * @param data_size データのサイズ
     * @param buffer 出力バッファ（Header + Data + Checksumが入る）
     * @param buffer_size バッファサイズ
     * @return 生成されたパケットの合計バイト数（失敗時は0）
     */
    static uint8_t createPacket(CmdID cmd_id, const void* data, uint8_t data_size, uint8_t* buffer, uint8_t buffer_size) {
        if (buffer_size < sizeof(Header) + data_size + 1) return 0;

        Header* header = reinterpret_cast<Header*>(buffer);
        header->magic = MAGIC_BYTE;
        header->cmd_id = static_cast<uint8_t>(cmd_id);
        header->size = data_size;

        if (data_size > 0 && data != nullptr) {
            std::memcpy(buffer + sizeof(Header), data, data_size);
        }

        uint8_t checksum = 0;
        for (int i = 0; i < sizeof(Header) + data_size; ++i) {
            checksum += buffer[i];
        }
        buffer[sizeof(Header) + data_size] = checksum;

        return sizeof(Header) + data_size + 1;
    }

    /**
     * @brief 受信データの検証を行う
     * @param buffer 受信した各種データ（Header + Payload + Checksum）
     * @param length データ長
     * @return 正しいパケットであればtrue
     */
    static bool validatePacket(const uint8_t* buffer, uint8_t length) {
        if (length < sizeof(Header) + 1) return false; // ヘッダ+チェックサム分は最低必要

        const Header* header = reinterpret_cast<const Header*>(buffer);
        if (header->magic != MAGIC_BYTE) return false;
        if (header->size != length - sizeof(Header) - 1) return false;

        uint8_t checksum = 0;
        for (int i = 0; i < length - 1; ++i) {
            checksum += buffer[i];
        }

        return checksum == buffer[length - 1];
    }
};

} // namespace protocol
} // namespace locomo

#endif // PROTOCOL_H
